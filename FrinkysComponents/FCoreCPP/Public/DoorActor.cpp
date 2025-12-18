// DoorActor.cpp
#include "DoorActor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "NavModifierComponent.h"
#include "NavAreas/NavArea_Obstacle.h"
#include "Net/UnrealNetwork.h"
#include "TimerManager.h"

ADoorActor::ADoorActor()
{
    PrimaryActorTick.bCanEverTick = true;
    bReplicates = true;
    
    // Parent class already creates InteractableRoot and PlayerInteractionPoint
    // We just need to create door-specific components
    
    // Create door frame
    DoorFrame = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DoorFrame"));
    DoorFrame->SetupAttachment(InteractableRoot);  // <-- Use parent's root
    DoorFrame->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    DoorFrame->SetCollisionResponseToAllChannels(ECR_Block);
    
    // Create door mesh
    DoorMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DoorMesh"));
    DoorMesh->SetupAttachment(DoorFrame);
    DoorMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    
    // Create Nav Modifier Component
    NavModifier = CreateDefaultSubobject<UNavModifierComponent>(TEXT("NavModifier"));
    NavModifier->SetAreaClass(UNavArea_Obstacle::StaticClass());
    
    // Configure interaction settings from parent
    InteractionName = "Open Door";
    InteractionType = EInteractionType::Instant;
    bPlayerCanInteract = true;
    bAICanInteract = true;
    
    // Initialize door state
    CurrentState = EDoorState::Closed;
    bIsLocked = false;
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
    
    // Store initial rotations
    ClosedRotation = DoorMesh->GetRelativeRotation();
    OpenRotation = ClosedRotation + FRotator(0.0f, OpenAngle, 0.0f);
}

void ADoorActor::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    // Update door rotation based on state
    UpdateDoorRotation(DeltaTime);
}

void ADoorActor::StartInteraction_Implementation(APawn* InteractingPawn)
{
    // Only execute on server
    if (!HasAuthority())
    {
        return;
    }
    
    // Can't interact if locked
    if (bIsLocked)
    {
        UE_LOG(LogTemp, Warning, TEXT("Door: Cannot open - door is locked"));
        return;
    }
    
    // Toggle door
    if (CurrentState == EDoorState::Closed)
    {
        Open();
    }
    else if (CurrentState == EDoorState::Open)
    {
        Close();
    }
}

bool ADoorActor::CanInteract_Implementation(APawn* InteractingPawn) const
{
    // Can only interact if closed or open (not while moving)
    return !bIsLocked && (CurrentState == EDoorState::Closed || CurrentState == EDoorState::Open);
}

void ADoorActor::Open()
{
    if (!HasAuthority())
    {
        return;
    }
    
    if (CurrentState == EDoorState::Closed && !bIsLocked)
    {
        CurrentState = EDoorState::Opening;
        OnRep_DoorState(); // Call locally on server
        
        UE_LOG(LogTemp, Log, TEXT("Door: Opening"));
        
        // Make door walkable when opening
        if (NavModifier)
        {
            NavModifier->SetAreaClass(nullptr); // Walkable area
            UE_LOG(LogTemp, Log, TEXT("Door: NavModifier set to walkable"));
        }
    }
}

void ADoorActor::Close()
{
    if (!HasAuthority())
    {
        return;
    }
    
    if (CurrentState == EDoorState::Open)
    {
        CurrentState = EDoorState::Closing;
        OnRep_DoorState(); // Call locally on server
        
        UE_LOG(LogTemp, Log, TEXT("Door: Closing"));
        
        // Make door obstacle when closing
        if (NavModifier)
        {
            NavModifier->SetAreaClass(UNavArea_Obstacle::StaticClass());
            UE_LOG(LogTemp, Log, TEXT("Door: NavModifier set to obstacle"));
        }
    }
}

void ADoorActor::SetLocked(bool bLocked)
{
    if (!HasAuthority())
    {
        UE_LOG(LogTemp, Error, TEXT("Door: SetLocked called on client!"));
        return;
    }
    
    bIsLocked = bLocked;
    OnRep_Locked(); // Call locally on server
    
    UE_LOG(LogTemp, Log, TEXT("Door lock set: %s"), bIsLocked ? TEXT("LOCKED") : TEXT("UNLOCKED"));
}

void ADoorActor::OnRep_DoorState()
{
    // Visual/audio feedback based on state
    switch (CurrentState)
    {
        case EDoorState::Opening:
            // Play opening sound
            ScheduleAutoClose();
            break;
            
        case EDoorState::Closing:
            // Play closing sound
            GetWorld()->GetTimerManager().ClearTimer(AutoCloseTimer);
            break;
            
        default:
            break;
    }
}

void ADoorActor::OnRep_Locked()
{
    // Visual feedback for locked state
}

void ADoorActor::UpdateDoorRotation(float DeltaTime)
{
    FRotator CurrentRotation = DoorMesh->GetRelativeRotation();
    FRotator TargetRotation = ClosedRotation;
    
    switch (CurrentState)
    {
        case EDoorState::Opening:
            TargetRotation = OpenRotation;
            CurrentRotation = FMath::RInterpTo(CurrentRotation, TargetRotation, DeltaTime, OpenSpeed);
            DoorMesh->SetRelativeRotation(CurrentRotation);
            
            // Check if fully open
            if (CurrentRotation.Equals(OpenRotation, 1.0f))
            {
                if (HasAuthority())
                {
                    CurrentState = EDoorState::Open;
                    OnRep_DoorState();
                }
            }
            break;
            
        case EDoorState::Closing:
            TargetRotation = ClosedRotation;
            CurrentRotation = FMath::RInterpTo(CurrentRotation, TargetRotation, DeltaTime, OpenSpeed);
            DoorMesh->SetRelativeRotation(CurrentRotation);
            
            // Check if fully closed
            if (CurrentRotation.Equals(ClosedRotation, 1.0f))
            {
                if (HasAuthority())
                {
                    CurrentState = EDoorState::Closed;
                    OnRep_DoorState();
                }
            }
            break;
            
        default:
            break;
    }
}

void ADoorActor::ScheduleAutoClose()
{
    if (!HasAuthority())
    {
        return;
    }
    
    // Clear existing timer
    GetWorld()->GetTimerManager().ClearTimer(AutoCloseTimer);
    
    // Schedule auto-close
    GetWorld()->GetTimerManager().SetTimer(
        AutoCloseTimer,
        this,
        &ADoorActor::Close,
        AutoCloseDelay,
        false
    );
}