// GuestAIController.h
#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "GuestAIController.generated.h"

// Forward declarations
class AGuestPawn;
class ACheckInDesk;
class ARoomActor;
class AInteractableActor;

UCLASS()
class FCORECPP_API AGuestAIController : public AAIController
{
	GENERATED_BODY()
    
public:
	AGuestAIController();
    
	virtual void OnPossess(APawn* InPawn) override;
    
	// ===== PROPERTIES =====
    
	UPROPERTY(BlueprintReadOnly, Category = "Guest AI")
	AGuestPawn* ControlledGuest;
    
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Guest AI")
	float AcceptanceRadius = 100.0f; // How close to get to destination
    
	// Current interactable the AI is moving to
	UPROPERTY(BlueprintReadOnly, Category = "Guest AI")
	AInteractableActor* TargetInteractable;
    
	// ===== NAVIGATION FUNCTIONS =====
    
	// Move to check-in desk
	UFUNCTION(BlueprintCallable, Category = "Guest AI")
	void MoveToCheckInDesk();
    
	// Move to assigned room
	UFUNCTION(BlueprintCallable, Category = "Guest AI")
	void MoveToRoom(ARoomActor* Room);
    
	// Move to hotel exit
	UFUNCTION(BlueprintCallable, Category = "Guest AI")
	void MoveToExit();
    
	// Stop guest movement
	UFUNCTION(BlueprintCallable, Category = "Guest AI")
	void StopGuestMovement();
    
	// ===== AI INTERACTION =====
    
	// Move to an interactable and use it when reached
	UFUNCTION(BlueprintCallable, Category = "Guest AI")
	void MoveToAndInteract(AInteractableActor* Interactable);
    
protected:
	// Movement completion handler
	void OnMoveCompleted(FAIRequestID RequestID, const FPathFollowingResult& Result);
    
	// Helper to find check-in desk
	ACheckInDesk* FindCheckInDesk();
};