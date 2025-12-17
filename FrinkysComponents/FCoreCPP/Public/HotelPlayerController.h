// HotelPlayerController.h
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "CheckInDesk.h" // For FGuestCheckInResponse struct
#include "HotelPlayerController.generated.h"

// Forward declarations
class ACheckInDesk;
class AGuestPawn;
class AInteractableActor;

UCLASS()
class FCORECPP_API AHotelPlayerController : public APlayerController
{
	GENERATED_BODY()
    
public:
	AHotelPlayerController();
    
	// ===== CHECK-IN DESK INTERACTION =====
    
	// Client calls this to request check-in processing
	// Server validates and processes via CheckInDesk
	UFUNCTION(BlueprintCallable, Server, Reliable, Category = "Hotel Interactions")
	void Server_RequestCheckIn(ACheckInDesk* Desk, FGuestCheckInResponse Response);
    
	// ===== INTERACTABLE ACTOR (UNIVERSAL) =====
    
	// Universal interaction RPC for all InteractableActors (including doors)
	UFUNCTION(BlueprintCallable, Server, Reliable, Category = "Hotel Interactions")
	void Server_RequestInteraction(AInteractableActor* Interactable);
    
protected:
	virtual void BeginPlay() override;
};