// HotelPlayerController.cpp
#include "HotelPlayerController.h"
#include "CheckInDesk.h"
#include "GuestPawn.h"
#include "InteractableActor.h"

AHotelPlayerController::AHotelPlayerController()
{
    // Constructor
}

void AHotelPlayerController::BeginPlay()
{
    Super::BeginPlay();
}

void AHotelPlayerController::Server_RequestCheckIn_Implementation(ACheckInDesk* Desk, FGuestCheckInResponse Response)
{
    // This runs on the server (called by client via RPC)
    
    if (!HasAuthority())
    {
        UE_LOG(LogTemp, Error, TEXT("HotelPC: Server_RequestCheckIn called on client! This should never happen."));
        return;
    }
    
    // Validate desk exists
    if (!Desk)
    {
        UE_LOG(LogTemp, Warning, TEXT("HotelPC: Server_RequestCheckIn - Desk is null"));
        return;
    }
    
    // Validate player is actually at the desk (optional anti-cheat)
    if (GetPawn())
    {
        float DistanceToDesk = FVector::Dist(GetPawn()->GetActorLocation(), Desk->GetActorLocation());
        if (DistanceToDesk > 500.0f) // Max interaction distance
        {
            UE_LOG(LogTemp, Warning, TEXT("HotelPC: Player %s tried to check-in from %f units away (too far!)"), 
                   *GetName(), DistanceToDesk);
            return;
        }
    }
    
    // Validate there's a guest to process
    if (!Desk->GetCurrentGuest())
    {
        UE_LOG(LogTemp, Warning, TEXT("HotelPC: No guest at desk to process"));
        return;
    }
    
    // All validation passed - process check-in
    Desk->ProcessCheckIn(GetPawn(), Response);
    
    UE_LOG(LogTemp, Log, TEXT("HotelPC: Player %s processed check-in (Approved: %s)"), 
           *GetName(), Response.bApproved ? TEXT("Yes") : TEXT("No"));
}

void AHotelPlayerController::Server_RequestInteraction_Implementation(AInteractableActor* Interactable)
{
    // This runs on the server (called by client via RPC)
    
    if (!HasAuthority())
    {
        UE_LOG(LogTemp, Error, TEXT("HotelPC: Server_RequestInteraction called on client! This should never happen."));
        return;
    }
    
    // Validate interactable exists
    if (!Interactable)
    {
        UE_LOG(LogTemp, Warning, TEXT("HotelPC: Server_RequestInteraction - Interactable is null"));
        return;
    }
    
    // Validate player is close enough (anti-cheat)
    if (GetPawn())
    {
        float DistanceToInteractable = FVector::Dist(GetPawn()->GetActorLocation(), Interactable->GetActorLocation());
        if (DistanceToInteractable > 500.0f) // Max interaction distance
        {
            UE_LOG(LogTemp, Warning, TEXT("HotelPC: Player %s tried to interact from %f units away (too far!)"), 
                   *GetName(), DistanceToInteractable);
            return;
        }
    }
    
    // All validation passed - start interaction
    Interactable->StartInteraction(GetPawn());
    
    UE_LOG(LogTemp, Log, TEXT("HotelPC: Player %s interacted with %s"), 
           *GetName(), *Interactable->GetName());
}