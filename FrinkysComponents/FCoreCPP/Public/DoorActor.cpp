// DoorActor.cpp
#include "DoorActor.h"
#include "HotelPlayerController.h"
#include "Components/BoxComponent.h"
#include "Net/UnrealNetwork.h"

ADoorActor::ADoorActor()
{
    PrimaryActorTick.bCanEverTick = true;
    bReplicates = true;
    
    // Create root
    DoorRoot = CreateDefaultSubobject<USceneComponent>(TEXT("DoorRoot"));
    RootComponent = DoorRoot;
    
    // Create door frame (static)
    DoorFrame = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DoorFrame"));
    DoorFrame->SetupAttachment(DoorRoot);
    DoorFrame->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    DoorFrame->SetCollisionResponseToAllChannels(ECR_Block);
    
    // Create hinge pivot (movable)
    HingePivot = CreateDefaultSubobject<USceneComponent>(TEXT("HingePivot"));
    HingePivot->SetupAttachment(DoorRoot);
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
    
    // Create interaction point
    InteractionPoint = CreateDefaultSubobject<UFC_InteractionPoint>(TEXT("InteractionPoint"));
    InteractionPoint->SetupAttachment(DoorRoot);
    InteractionPoint->InteractionName = "Open Door";
    InteractionPoint->InteractionTime = 0.0f; // Instant
    
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
}

void ADoorActor::BeginPlay()
{
    Super::BeginPlay();
    
    // Bind interaction
    if (InteractionPoint)
    {
        InteractionPoint->OnInteract.AddDynamic(this, &ADoorActor::OnDoorInteracted);
    }
    
    // Apply initial lock state
    if (HasAuthority())
    {
        SetLocked(bStartsLocked);
    }
}

void ADoorActor::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    // Animate door rotation
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
    if (InteractionPoint)
    {
        InteractionPoint->InteractionName = "Close Door";
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
    if (InteractionPoint)
    {
        InteractionPoint->InteractionName = "Open Door";
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
    if (InteractionPoint)
    {
        InteractionPoint->isEnabled = !bIsLocked;
        InteractionPoint->InteractionName = bIsLocked ? "Locked" : (IsOpen() ? "Close Door" : "Open Door");
    }
    
    // Disable blocker if open and locked (guest is inside)
    if (DoorwayBlocker && bIsLocked && IsOpen())
    {
        DoorwayBlocker->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    }
    
    UE_LOG(LogTemp, Log, TEXT("Door lock set: %s"), bIsLocked ? TEXT("LOCKED") : TEXT("UNLOCKED"));
}

void ADoorActor::OnRep_DoorState()
{
    // Update visual state on clients when replicated
    UE_LOG(LogTemp, Log, TEXT("Door state replicated: %d"), (int32)CurrentState);
}

void ADoorActor::OnDoorInteracted(APawn* Interactor)
{
    // Player pressed E - call server RPC on their PlayerController
    AHotelPlayerController* PC = Cast<AHotelPlayerController>(Interactor->GetController());
    if (!PC)
    {
        UE_LOG(LogTemp, Warning, TEXT("Door: Interactor has no HotelPlayerController"));
        return;
    }
    
    // Determine if player wants to open or close
    bool bWantsOpen = IsClosed();
    
    // Call server RPC on player's controller
    PC->Server_RequestDoorInteraction(this, bWantsOpen);
}

void ADoorActor::AnimateDoor(float DeltaTime)
{
    // Smoothly rotate door to target
    if (FMath::IsNearlyEqual(CurrentRotation, TargetRotation, 0.1f))
    {
        // Reached target
        CurrentRotation = TargetRotation;
        
        // Update state
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
        // Interpolate rotation
        CurrentRotation = FMath::FInterpTo(CurrentRotation, TargetRotation, DeltaTime, OpenSpeed);
    }
    
    // Apply rotation to hinge pivot
    if (HingePivot)
    {
        HingePivot->SetRelativeRotation(FRotator(0.0f, CurrentRotation, 0.0f));
    }
}