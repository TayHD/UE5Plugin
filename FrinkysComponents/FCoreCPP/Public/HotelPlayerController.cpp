// HotelPlayerController.cpp
#include "HotelPlayerController.h"
#include "CheckInDesk.h"
#include "GuestPawn.h"
#include "DoorActor.h"

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
    float DistanceToDesk = FVector::Dist(GetPawn()->GetActorLocation(), Desk->GetActorLocation());
    if (DistanceToDesk > 500.0f) // Max interaction distance
    {
        UE_LOG(LogTemp, Warning, TEXT("HotelPC: Player %s tried to check-in from %f units away (too far!)"), 
               *GetName(), DistanceToDesk);
        return;
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

void AHotelPlayerController::Server_RequestDoorInteraction_Implementation(ADoorActor* Door, bool bWantsOpen)
{
    // This runs on the server (called by client via RPC)
    
    if (!HasAuthority())
    {
        UE_LOG(LogTemp, Error, TEXT("HotelPC: Server_RequestDoorInteraction called on client! This should never happen."));
        return;
    }
    
    // Validate door exists
    if (!Door)
    {
        UE_LOG(LogTemp, Warning, TEXT("HotelPC: Server_RequestDoorInteraction - Door is null"));
        return;
    }
    
    // Validate player is close enough (anti-cheat)
    if (GetPawn())
    {
        float DistanceToDoor = FVector::Dist(GetPawn()->GetActorLocation(), Door->GetActorLocation());
        if (DistanceToDoor > 500.0f) // Max interaction distance
        {
            UE_LOG(LogTemp, Warning, TEXT("HotelPC: Player %s tried to interact with door from %f units away (too far!)"), 
                   *GetName(), DistanceToDoor);
            return;
        }
    }
    
    // All validation passed - interact with door
    if (bWantsOpen)
    {
        Door->OpenDoor();
    }
    else
    {
        Door->CloseDoor();
    }
    
    UE_LOG(LogTemp, Log, TEXT("HotelPC: Player %s %s door"), 
           *GetName(), bWantsOpen ? TEXT("opened") : TEXT("closed"));
}