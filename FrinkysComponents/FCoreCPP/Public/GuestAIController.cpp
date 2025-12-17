// GuestAIController.cpp
#include "GuestAIController.h"
#include "GuestPawn.h"
#include "CheckInDesk.h"
#include "RoomActor.h"
#include "InteractableActor.h"
#include "Navigation/PathFollowingComponent.h"
#include "Kismet/GameplayStatics.h"

AGuestAIController::AGuestAIController()
{
    bSetControlRotationFromPawnOrientation = true;
    
    // Initialize
    ControlledGuest = nullptr;
    TargetInteractable = nullptr;
}

void AGuestAIController::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);
    
    // Store reference to guest
    ControlledGuest = Cast<AGuestPawn>(InPawn);
    
    if (!ControlledGuest)
    {
        UE_LOG(LogTemp, Error, TEXT("GuestAI: Possessed pawn is not a GuestPawn"));
        return;
    }
    
    UE_LOG(LogTemp, Log, TEXT("AI Controller possessed guest: %s"), 
           *ControlledGuest->GetGuestData().GuestName);
    
    // Bind movement completion (use GetPathFollowingComponent instead of PathFollowingComponent)
    UPathFollowingComponent* PathFollowing = GetPathFollowingComponent();
    if (PathFollowing)
    {
        PathFollowing->OnRequestFinished.AddUObject(this, &AGuestAIController::OnMoveCompleted);
    }
    
    // Start moving to check-in desk
    MoveToCheckInDesk();
}

void AGuestAIController::MoveToCheckInDesk()
{
    if (!ControlledGuest)
    {
        return;
    }
    
    // Find check-in desk
    ACheckInDesk* Desk = FindCheckInDesk();
    if (!Desk)
    {
        UE_LOG(LogTemp, Error, TEXT("GuestAI: No check-in desk found in level!"));
        return;
    }
    
    // Get destination from desk's guest wait point
    FVector Destination = Desk->GuestWaitPoint->GetComponentLocation();
    
    // Move to destination
    MoveToLocation(Destination, AcceptanceRadius);
    
    UE_LOG(LogTemp, Log, TEXT("GuestAI: %s moving to check-in desk"), 
           *ControlledGuest->GetGuestData().GuestName);
}

void AGuestAIController::MoveToRoom(ARoomActor* Room)
{
    if (!Room)
    {
        UE_LOG(LogTemp, Warning, TEXT("GuestAI: MoveToRoom - Room is null"));
        return;
    }
    
    if (!ControlledGuest)
    {
        return;
    }
    
    // Get room center as destination
    FVector Destination = Room->GetActorLocation();
    
    // Move to room
    MoveToLocation(Destination, AcceptanceRadius);
    
    UE_LOG(LogTemp, Log, TEXT("GuestAI: %s moving to Room %d"), 
           *ControlledGuest->GetGuestData().GuestName, Room->RoomNumber);
}

void AGuestAIController::MoveToExit()
{
    if (!ControlledGuest)
    {
        return;
    }
    
    // Simple exit: just move away from hotel (1000 units in X direction)
    FVector CurrentLocation = ControlledGuest->GetActorLocation();
    FVector ExitDirection = FVector(1000.0f, 0.0f, 0.0f);
    FVector ExitLocation = CurrentLocation + ExitDirection;
    
    // Move to exit
    MoveToLocation(ExitLocation, AcceptanceRadius);
    
    UE_LOG(LogTemp, Log, TEXT("GuestAI: %s moving to exit"), 
           *ControlledGuest->GetGuestData().GuestName);
}

void AGuestAIController::StopGuestMovement()
{
    StopMovement();
    
    if (ControlledGuest)
    {
        UE_LOG(LogTemp, Log, TEXT("GuestAI: %s stopped moving"), 
               *ControlledGuest->GetGuestData().GuestName);
    }
}

void AGuestAIController::MoveToAndInteract(AInteractableActor* Interactable)
{
    if (!Interactable)
    {
        UE_LOG(LogTemp, Warning, TEXT("GuestAI: Tried to interact with null interactable"));
        return;
    }
    
    TargetInteractable = Interactable;
    
    // Move to the interactable
    MoveToActor(Interactable, AcceptanceRadius);
    
    if (ControlledGuest)
    {
        UE_LOG(LogTemp, Log, TEXT("GuestAI: %s moving to interact with %s"), 
               *ControlledGuest->GetGuestData().GuestName, *Interactable->GetName());
    }
}

void AGuestAIController::OnMoveCompleted(FAIRequestID RequestID, const FPathFollowingResult& Result)
{
    if (!ControlledGuest)
    {
        return;
    }
    
    // Check if movement succeeded
    switch (Result.Code)
    {
        case EPathFollowingResult::Success:
        {
            UE_LOG(LogTemp, Log, TEXT("GuestAI: %s reached destination"), 
                   *ControlledGuest->GetGuestData().GuestName);
            
            // Check what state we're in and what to do next
            switch (ControlledGuest->CurrentBehaviorState)
            {
                case EGuestBehaviorState::Approaching:
                {
                    // Reached check-in desk wait point
                    ACheckInDesk* Desk = FindCheckInDesk();
                    if (Desk)
                    {
                        Desk->AddGuestToQueue(ControlledGuest);
                    }
                    break;
                }
                
                case EGuestBehaviorState::GoingToRoom:
                {
                    // Check if we just reached a door to interact with
                    if (TargetInteractable)
                    {
                        // We reached the door - interact with it (open it)
                        TargetInteractable->StartInteraction(ControlledGuest);
                        TargetInteractable = nullptr;
                        
                        // Now move into the room
                        if (ControlledGuest->AssignedRoom)
                        {
                            MoveToRoom(ControlledGuest->AssignedRoom);
                        }
                    }
                    else
                    {
                        // We're inside the room now
                        ControlledGuest->Server_SetBehaviorState(EGuestBehaviorState::InRoom);
                    }
                    break;
                }
                
                case EGuestBehaviorState::Leaving:
                {
                    // Reached exit - destroy guest
                    if (ControlledGuest)
                    {
                        ControlledGuest->Destroy();
                    }
                    break;
                }
                
                default:
                    break;
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
            UE_LOG(LogTemp, Error, TEXT("GuestAI: %s pathfinding invalid"), 
                   *ControlledGuest->GetGuestData().GuestName);
            break;
        }
        
        default:
            break;
    }
}

ACheckInDesk* AGuestAIController::FindCheckInDesk()
{
    // Find check-in desk in level
    TArray<AActor*> FoundDesks;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ACheckInDesk::StaticClass(), FoundDesks);
    
    if (FoundDesks.Num() > 0)
    {
        return Cast<ACheckInDesk>(FoundDesks[0]);
    }
    
    return nullptr;
}