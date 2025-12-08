// GuestAIController.h
#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Navigation/PathFollowingComponent.h"
#include "GuestAIController.generated.h"

// Forward declarations
class AGuestPawn;
class ACheckInDesk;
class ARoomActor;

UCLASS()
class FCORECPP_API AGuestAIController : public AAIController
{
	GENERATED_BODY()
    
public:
	AGuestAIController();
    
	virtual void BeginPlay() override;
	virtual void OnPossess(APawn* InPawn) override;
    
	// ===== NAVIGATION =====
    
	// Move to check-in desk and join queue
	UFUNCTION(BlueprintCallable, Category = "Guest AI")
	void MoveToCheckInDesk();
    
	// Move to assigned room
	UFUNCTION(BlueprintCallable, Category = "Guest AI")
	void MoveToRoom(ARoomActor* Room);
    
	// Move to exit and leave hotel
	UFUNCTION(BlueprintCallable, Category = "Guest AI")
	void MoveToExit();
    
	// Stop all movement
	UFUNCTION(BlueprintCallable, Category = "Guest AI")
	void StopMovement();
    
	// ===== REFERENCES =====
    
	UPROPERTY(BlueprintReadOnly, Category = "Guest AI")
	AGuestPawn* ControlledGuest;
    
	// ===== SETTINGS =====
    
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Guest AI")
	float AcceptanceRadius = 100.0f; // How close to get to destination
    
private:
	// Movement completion callbacks
	UFUNCTION()
	void OnMoveCompleted(FAIRequestID RequestID, EPathFollowingResult::Type Result);
    
	// Find the check-in desk in the level
	ACheckInDesk* FindCheckInDesk();
};