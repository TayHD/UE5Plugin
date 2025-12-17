// HotelPlayerController.h
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "CheckInDesk.h" // For FGuestCheckInResponse struct
#include "HotelPlayerController.generated.h"

// Forward declarations
class ACheckInDesk;
class AGuestPawn;
class ADoorActor;

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
    
	// ===== DOOR INTERACTION =====
    
	// Client calls this to request door open/close
	UFUNCTION(BlueprintCallable, Server, Reliable, Category = "Hotel Interactions")
	void Server_RequestDoorInteraction(ADoorActor* Door, bool bWantsOpen);
    
	// ===== FUTURE INTERACTIONS =====
    
	// Examples of future interactions that will follow same pattern:
	// Server_RequestVendingPurchase(AVendingMachine* Machine, int32 ItemID)
	// Server_RequestPhoneCall(APhoneActor* Phone, EPhoneCallType CallType)
    
protected:
	virtual void BeginPlay() override;
};