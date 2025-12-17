// DoorActor.cpp
#include "DoorActor.h"
#include "Net/UnrealNetwork.h"

ADoorActor::ADoorActor()
{
    PrimaryActorTick.bCanEverTick = true;
    bReplicates = true;
    
    // Set interaction type to instant (door toggles immediately)
    InteractionType = EInteractionType::Instant;
    InteractionName = "Open Door";
    
    // Create door frame (static)
    DoorFrame = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DoorFrame"));
    DoorFrame->SetupAttachment(InteractableRoot);
    DoorFrame->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    DoorFrame->SetCollisionResponseToAllChannels(ECR_Block);
    
    // Create hinge pivot (movable)
    HingePivot = CreateDefaultSubobject<USceneComponent>(TEXT("HingePivot"));
    HingePivot->SetupAttachment(InteractableRoot);
    HingePivot->SetMobility(EComponentMobility::Movable);
    
    // Create door mesh (rotates with pivot)
    DoorMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DoorMesh"));
    DoorMesh->SetupAttachment(HingePivot);
    DoorMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    DoorMesh->SetCollisionResponseToAllChannels(ECR_Block);
    
    // Create doorway blocker (moves with door)
    DoorwayBlocker = CreateDefaultSubobject<UBoxComponent>(TEXT("DoorwayBlocker"));
    DoorwayBlocker->SetupAttachment(HingePivot);
    DoorwayBlocker->SetBoxExtent(FVector(50.0f, 5.0f, 100.0f));
    DoorwayBlocker->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    DoorwayBlocker->SetCollisionResponseToAllChannels(ECR_Block);
    
    // Initialize state
    CurrentState = EDoorState::Closed;
    bIsLocked = false;
    CurrentRotation = 0.0f;
    TargetRotation = 0.0f;
}

void ADoorActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    
    DOREPLIFETIME(ADoorActor, CurrentState);
    DOREPLIFETIME(ADoorActor, bIsLocked);
    DOREPLIFETIME(ADoorActor, CurrentRotation);
    DOREPLIFETIME(ADoorActor, TargetRotation);
}

void ADoorActor::BeginPlay()
{
    Super::BeginPlay();
    
    // Apply initial lock state
    if (HasAuthority())
    {
        SetLocked(bStartsLocked);
    }
}

void ADoorActor::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    // Animate door rotation (runs on all machines)
    AnimateDoor(DeltaTime);
}

void ADoorActor::OpenDoor()
{
    // Regular function - should only be called on server
    if (!HasAuthority())
    {
        UE_LOG(LogTemp, Error, TEXT("DoorActor: OpenDoor called on client! This should never happen."));
        return;
    }
    
    if (bIsLocked)
    {
        UE_LOG(LogTemp, Warning, TEXT("Door: Cannot open - door is locked"));
        return;
    }
    
    if (CurrentState == EDoorState::Open || CurrentState == EDoorState::Opening)
    {
        return; // Already open/opening
    }
    
    CurrentState = EDoorState::Opening;
    TargetRotation = OpenAngle;
    
    // Update interaction prompt
    if (PlayerInteractionPoint)
    {
        PlayerInteractionPoint->InteractionName = "Close Door";
    }
    
    UE_LOG(LogTemp, Log, TEXT("Door: Opening"));
}

void ADoorActor::CloseDoor()
{
    // Regular function - should only be called on server
    if (!HasAuthority())
    {
        UE_LOG(LogTemp, Error, TEXT("DoorActor: CloseDoor called on client! This should never happen."));
        return;
    }
    
    if (CurrentState == EDoorState::Closed || CurrentState == EDoorState::Closing)
    {
        return; // Already closed/closing
    }
    
    CurrentState = EDoorState::Closing;
    TargetRotation = 0.0f;
    
    // Update interaction prompt
    if (PlayerInteractionPoint)
    {
        PlayerInteractionPoint->InteractionName = "Open Door";
    }
    
    UE_LOG(LogTemp, Log, TEXT("Door: Closing"));
}

void ADoorActor::SetLocked(bool bLocked)
{
    // Regular function - should only be called on server
    if (!HasAuthority())
    {
        UE_LOG(LogTemp, Error, TEXT("DoorActor: SetLocked called on client! This should never happen."));
        return;
    }
    
    bIsLocked = bLocked;
    
    // Update interaction point
    if (PlayerInteractionPoint)
    {
        PlayerInteractionPoint->isEnabled = !bIsLocked;
        PlayerInteractionPoint->InteractionName = bIsLocked ? "Locked" : (IsOpen() ? "Close Door" : "Open Door");
    }
    
    // Disable blocker if open and locked (guest is inside)
    if (DoorwayBlocker && bIsLocked && IsOpen())
    {
        DoorwayBlocker->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    }
    
    UE_LOG(LogTemp, Log, TEXT("Door lock set: %s"), bIsLocked ? TEXT("LOCKED") : TEXT("UNLOCKED"));
}

void ADoorActor::OnInteractionStarted_Implementation(AActor* Interactor)
{
    // Called when player/AI interacts with door
    // Toggle door open/close based on current state
    
    if (IsClosed())
    {
        OpenDoor();
    }
    else if (IsOpen())
    {
        CloseDoor();
    }
}

bool ADoorActor::CanInteract(AActor* Interactor) const
{
    // Call base implementation first
    if (!Super::CanInteract(Interactor))
    {
        return false;
    }
    
    // Door-specific: Can't interact if locked
    if (bIsLocked)
    {
        return false;
    }
    
    return true;
}

void ADoorActor::OnRep_DoorState()
{
    // Update visual state on clients when replicated
    UE_LOG(LogTemp, Log, TEXT("Door state replicated: %d"), (int32)CurrentState);
}

void ADoorActor::AnimateDoor(float DeltaTime)
{
    // Smoothly rotate door to target (runs on all machines)
    if (FMath::IsNearlyEqual(CurrentRotation, TargetRotation, 0.1f))
    {
        // Reached target
        CurrentRotation = TargetRotation;
        
        // Update state (only on server)
        if (HasAuthority())
        {
            if (CurrentState == EDoorState::Opening)
            {
                CurrentState = EDoorState::Open;
            }
            else if (CurrentState == EDoorState::Closing)
            {
                CurrentState = EDoorState::Closed;
            }
        }
    }
    else
    {
        // Interpolate rotation (happens on all machines)
        CurrentRotation = FMath::FInterpTo(CurrentRotation, TargetRotation, DeltaTime, OpenSpeed);
    }
    
    // Apply rotation to hinge pivot (visual update on all machines)
    if (HingePivot)
    {
        HingePivot->SetRelativeRotation(FRotator(0.0f, CurrentRotation, 0.0f));
    }
}