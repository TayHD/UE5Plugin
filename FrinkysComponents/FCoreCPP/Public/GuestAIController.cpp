// GuestAIController.cpp
#include "GuestAIController.h"
#include "GuestPawn.h"
#include "CheckInDesk.h"
#include "RoomActor.h"
#include "Navigation/PathFollowingComponent.h"
#include "Kismet/GameplayStatics.h"

AGuestAIController::AGuestAIController()
{
    bSetControlRotationFromPawnOrientation = true;
    
    // Create path following component
    if (!GetPathFollowingComponent())
    {
        SetPathFollowingComponent(CreateDefaultSubobject<UPathFollowingComponent>(TEXT("PathFollowingComponent")));
    }
}

void AGuestAIController::BeginPlay()
{
    Super::BeginPlay();
    
    // Bind movement completion
    if (GetPathFollowingComponent())
    {
        GetPathFollowingComponent()->OnRequestFinished.AddUObject(this, &AGuestAIController::OnMoveCompleted);
    }
}

void AGuestAIController::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);
    
    ControlledGuest = Cast<AGuestPawn>(InPawn);
    
    if (ControlledGuest)
    {
        UE_LOG(LogTemp, Log, TEXT("AI Controller possessed guest: %s"), 
               *ControlledGuest->GetGuestData().GuestName);
        
        // Start by moving to check-in desk
        MoveToCheckInDesk();
    }
}

void AGuestAIController::MoveToCheckInDesk()
{
    ACheckInDesk* Desk = FindCheckInDesk();
    
    if (!Desk)
    {
        UE_LOG(LogTemp, Error, TEXT("GuestAI: No check-in desk found in level!"));
        return;
    }
    
    // Move to the desk's guest wait point
    FVector Destination = Desk->GuestWaitPoint->GetComponentLocation();
    
    EPathFollowingRequestResult::Type Result = MoveToLocation(Destination, AcceptanceRadius);
    
    if (Result == EPathFollowingRequestResult::Failed)
    {
        UE_LOG(LogTemp, Error, TEXT("GuestAI: Failed to start pathfinding to desk"));
    }
    else
    {
        UE_LOG(LogTemp, Log, TEXT("GuestAI: %s moving to check-in desk"), 
               *ControlledGuest->GetGuestData().GuestName);
    }
}

void AGuestAIController::MoveToRoom(ARoomActor* Room)
{
    if (!Room)
    {
        UE_LOG(LogTemp, Error, TEXT("GuestAI: Cannot move to null room"));
        return;
    }
    
    // Move to center of room
    FVector Destination = Room->GetActorLocation();
    
    EPathFollowingRequestResult::Type Result = MoveToLocation(Destination, AcceptanceRadius);
    
    if (Result == EPathFollowingRequestResult::Failed)
    {
        UE_LOG(LogTemp, Error, TEXT("GuestAI: Failed to start pathfinding to room"));
    }
    else
    {
        UE_LOG(LogTemp, Log, TEXT("GuestAI: %s moving to Room %d"), 
               *ControlledGuest->GetGuestData().GuestName, Room->RoomNumber);
    }
}

void AGuestAIController::MoveToExit()
{
    // For now, just move away from the hotel (we'll add proper exit points later)
    FVector CurrentLocation = GetPawn()->GetActorLocation();
    FVector ExitDirection = FVector(1000.0f, 0.0f, 0.0f); // Move 1000 units in X direction
    FVector Destination = CurrentLocation + ExitDirection;
    
    EPathFollowingRequestResult::Type Result = MoveToLocation(Destination, AcceptanceRadius);
    
    if (Result != EPathFollowingRequestResult::Failed)
    {
        UE_LOG(LogTemp, Log, TEXT("GuestAI: %s leaving hotel"), 
               *ControlledGuest->GetGuestData().GuestName);
    }
}

void AGuestAIController::StopGuestMovement()
{
    StopMovement(); // Call base class function
    
    if (ControlledGuest)
    {
        UE_LOG(LogTemp, Log, TEXT("GuestAI: %s stopped moving"), 
               *ControlledGuest->GetGuestData().GuestName);
    }
}

void AGuestAIController::OnMoveCompleted(FAIRequestID RequestID, const FPathFollowingResult& Result)
{
    if (!ControlledGuest)
    {
        return;
    }
    
    switch (Result.Code)
    {
        case EPathFollowingResult::Success:
        {
            UE_LOG(LogTemp, Log, TEXT("GuestAI: %s reached destination"), 
                   *ControlledGuest->GetGuestData().GuestName);
            
            // Handle arrival based on current state
            EGuestBehaviorState CurrentState = ControlledGuest->CurrentBehaviorState;
            
            if (CurrentState == EGuestBehaviorState::Approaching)
            {
                // Reached desk, add to queue
                ACheckInDesk* Desk = FindCheckInDesk();
                if (Desk)
                {
                    Desk->AddGuestToQueue(ControlledGuest);
                }
            }
            else if (CurrentState == EGuestBehaviorState::GoingToRoom)
            {
                // Reached room, enter it
                ControlledGuest->Server_SetBehaviorState(EGuestBehaviorState::InRoom);
            }
            else if (CurrentState == EGuestBehaviorState::Leaving)
            {
                // Reached exit, destroy self
                ControlledGuest->Destroy();
            }
            
            break;
        }
        case EPathFollowingResult::Blocked:
        case EPathFollowingResult::Aborted:
        {
            UE_LOG(LogTemp, Warning, TEXT("GuestAI: %s pathfinding blocked/aborted"), 
                   *ControlledGuest->GetGuestData().GuestName);
            break;
        }
        case EPathFollowingResult::Invalid:
        {
            UE_LOG(LogTemp, Error, TEXT("GuestAI: %s pathfinding failed - invalid path"), 
                   *ControlledGuest->GetGuestData().GuestName);
            break;
        }
        default:
            break;
    }
}

ACheckInDesk* AGuestAIController::FindCheckInDesk()
{
    TArray<AActor*> FoundDesks;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ACheckInDesk::StaticClass(), FoundDesks);
    
    if (FoundDesks.Num() > 0)
    {
        return Cast<ACheckInDesk>(FoundDesks[0]);
    }
    
    return nullptr;
}