// GuestPawn.cpp
#include "GuestPawn.h"
#include "RoomActor.h"
#include "GuestAIController.h"
#include "Net/UnrealNetwork.h"
#include "TimerManager.h"

AGuestPawn::AGuestPawn()
{
    PrimaryActorTick.bCanEverTick = false;
    bReplicates = true;
    
    // Create capsule component
    CapsuleComponent = CreateDefaultSubobject<UCapsuleComponent>(TEXT("CapsuleComponent"));
    CapsuleComponent->InitCapsuleSize(42.0f, 96.0f);
    CapsuleComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    CapsuleComponent->SetCollisionResponseToAllChannels(ECR_Block);
    RootComponent = CapsuleComponent;
    
    // Create skeletal mesh
    GuestMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("GuestMesh"));
    GuestMesh->SetupAttachment(CapsuleComponent);
    GuestMesh->SetRelativeLocation(FVector(0.0f, 0.0f, -96.0f)); // Offset to feet
    
    // Create movement component
    MovementComponent = CreateDefaultSubobject<UFloatingPawnMovement>(TEXT("MovementComponent"));
    MovementComponent->MaxSpeed = WalkSpeed;
    
    // Initialize state
    CurrentBehaviorState = EGuestBehaviorState::Approaching;
    AssignedRoom = nullptr;
    bHasPaid = false;
    bHasCheckedIn = false;
    TimeInHotel = 0.0f;
    
    // AI will control this pawn
    AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
    AIControllerClass = AGuestAIController::StaticClass();
}

void AGuestPawn::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    
    DOREPLIFETIME(AGuestPawn, GuestInfo);
    DOREPLIFETIME(AGuestPawn, CurrentBehaviorState);
    DOREPLIFETIME(AGuestPawn, AssignedRoom);
    DOREPLIFETIME(AGuestPawn, bHasPaid);
    DOREPLIFETIME(AGuestPawn, bHasCheckedIn);
    DOREPLIFETIME(AGuestPawn, TimeInHotel);
}

void AGuestPawn::BeginPlay()
{
    Super::BeginPlay();
    
    // Validate guest data
    GuestInfo.ValidateData();
    
    UE_LOG(LogTemp, Log, TEXT("Guest spawned: %s (Type: %d)"), 
           *GuestInfo.GuestName, (int32)GuestInfo.GuestType);
}

void AGuestPawn::InitializeGuest(FGuestData Data)
{
    GuestInfo = Data;
    GuestInfo.ValidateData();
    
    UE_LOG(LogTemp, Log, TEXT("Guest initialized: %s"), *GuestInfo.GuestName);
}

void AGuestPawn::Server_SetBehaviorState_Implementation(EGuestBehaviorState NewState)
{
    if (!HasAuthority())
    {
        return;
    }
    
    EGuestBehaviorState OldState = CurrentBehaviorState;
    CurrentBehaviorState = NewState;
    
    // Trigger replication callback
    OnRep_BehaviorState();
    
    // Get AI controller
    AGuestAIController* AIController = Cast<AGuestAIController>(GetController());
    
    // Handle state-specific behavior
    switch (NewState)
    {
        case EGuestBehaviorState::GoingToRoom:
            if (AIController && AssignedRoom)
            {
                AIController->MoveToRoom(AssignedRoom);
            }
            break;
            
        case EGuestBehaviorState::InRoom:
            if (AIController)
            {
                AIController->StopMovement();
            }
            if (AssignedRoom)
            {
                AssignedRoom->Server_AssignGuest(this);
            }
            StartCheckOutTimer();
            break;
            
        case EGuestBehaviorState::Leaving:
            if (AIController)
            {
                AIController->MoveToExit();
            }
            break;
            
        default:
            break;
    }
    
    UE_LOG(LogTemp, Log, TEXT("Guest %s: State changed %d -> %d"), 
           *GuestInfo.GuestName, (int32)OldState, (int32)NewState);
}

void AGuestPawn::Server_AssignRoom_Implementation(ARoomActor* Room)
{
    if (!HasAuthority())
    {
        return;
    }
    
    if (!Room)
    {
        UE_LOG(LogTemp, Warning, TEXT("Guest %s: Tried to assign null room"), *GuestInfo.GuestName);
        return;
    }
    
    AssignedRoom = Room;
    
    UE_LOG(LogTemp, Log, TEXT("Guest %s: Assigned to Room %d"), 
           *GuestInfo.GuestName, Room->RoomNumber);
}

void AGuestPawn::Server_MarkCheckedIn_Implementation()
{
    if (!HasAuthority())
    {
        return;
    }
    
    bHasCheckedIn = true;
    bHasPaid = true;
    
    UE_LOG(LogTemp, Log, TEXT("Guest %s: Checked in (Paid: $%.2f)"), 
           *GuestInfo.GuestName, GuestInfo.RoomRate);
}

void AGuestPawn::Server_CheckOut_Implementation()
{
    if (!HasAuthority())
    {
        return;
    }
    
    // Notify room that guest is leaving
    if (AssignedRoom)
    {
        AssignedRoom->Server_CheckOutGuest();
    }
    
    // Change state to leaving
    CurrentBehaviorState = EGuestBehaviorState::Leaving;
    OnRep_BehaviorState();
    
    UE_LOG(LogTemp, Log, TEXT("Guest %s: Checking out"), *GuestInfo.GuestName);
    
    // AI will handle walking to exit and destroying
    // (The destroy happens in AI controller when reaching exit)
}

void AGuestPawn::OnRep_BehaviorState()
{
    // Broadcast to Blueprint for visual feedback
    OnBehaviorStateChanged.Broadcast(this, CurrentBehaviorState);
    
    // Update movement speed based on state
    if (MovementComponent)
    {
        switch (CurrentBehaviorState)
        {
            case EGuestBehaviorState::Panicked:
                MovementComponent->MaxSpeed = WalkSpeed * 2.0f; // Run when panicked
                break;
            case EGuestBehaviorState::InRoom:
                MovementComponent->MaxSpeed = 0.0f; // Don't move in room
                break;
            default:
                MovementComponent->MaxSpeed = WalkSpeed;
                break;
        }
    }
}

void AGuestPawn::StartCheckOutTimer()
{
    if (!HasAuthority())
    {
        return;
    }
    
    // Guest will check out after MaxStayDuration
    GetWorldTimerManager().SetTimer(CheckOutTimer, this, &AGuestPawn::Server_CheckOut, MaxStayDuration, false);
    
    UE_LOG(LogTemp, Log, TEXT("Guest %s: Will check out in %.1f seconds"), 
           *GuestInfo.GuestName, MaxStayDuration);
}