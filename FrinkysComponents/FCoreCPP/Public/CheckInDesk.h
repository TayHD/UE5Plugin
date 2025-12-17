// CheckInDesk.h
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FrinkysComponents/General/FC_InteractionPoint.h"
#include "GuestData.h"
#include "CheckInDesk.generated.h"

// Forward declarations
class AGuestPawn;
class ARoomActor;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCheckInStarted, class AGuestPawn*, Guest, class APawn*, Clerk);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnCheckInCompleted, class AGuestPawn*, Guest, bool, bApproved, int32, RoomNumber);

USTRUCT(BlueprintType)
struct FGuestCheckInResponse
{
    GENERATED_BODY()
    
    UPROPERTY(BlueprintReadWrite)
    bool bApproved; // Did clerk approve check-in?
    
    UPROPERTY(BlueprintReadWrite)
    int32 AssignedRoomNumber; // Which room to give them (-1 if denied)
    
    UPROPERTY(BlueprintReadWrite)
    FString RejectionReason; // Optional: "Expired card", "Suspicious ID", etc.
    
    FGuestCheckInResponse()
    {
        bApproved = false;
        AssignedRoomNumber = -1;
        RejectionReason = "";
    }
};

UCLASS()
class FCORECPP_API ACheckInDesk : public AActor
{
    GENERATED_BODY()
    
public:
    ACheckInDesk();
    
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
    virtual void BeginPlay() override;
    
    // ===== COMPONENTS =====
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* DeskRoot;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* DeskMesh;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UFC_InteractionPoint* DeskInteraction;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* GuestWaitPoint; // Where guests stand in line
    
    // ===== STATE =====
    
    // The guest currently being served
    UPROPERTY(Replicated, BlueprintReadOnly, Category = "Desk State")
    AGuestPawn* CurrentGuest;
    
    // Queue of waiting guests
    UPROPERTY(Replicated, BlueprintReadOnly, Category = "Desk State")
    TArray<AGuestPawn*> GuestQueue;
    
    // Is a player currently serving someone?
    UPROPERTY(Replicated, BlueprintReadOnly, Category = "Desk State")
    bool bIsOccupied;
    
    // The player currently at the desk
    UPROPERTY(Replicated, BlueprintReadOnly, Category = "Desk State")
    APawn* CurrentClerk;
    
    // ===== SETTINGS =====
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Desk Settings")
    float QueueSpacing = 100.0f; // Distance between guests in line
    
    // ===== FUNCTIONS =====
    
    // Guest arrives at desk, joins queue (called by AI on server)
    UFUNCTION(BlueprintCallable, Category = "Check-In")
    void AddGuestToQueue(AGuestPawn* Guest);
    
    // Player interacts with desk to serve guest (called locally, opens UI)
    UFUNCTION(BlueprintCallable, Category = "Check-In")
    void OpenCheckInUI(APawn* Clerk);
    
    // Process check-in (called by PlayerController's Server RPC on server)
    // NOT an RPC - just a regular function that runs on server
    UFUNCTION(BlueprintCallable, Category = "Check-In")
    void ProcessCheckIn(APawn* Clerk, FGuestCheckInResponse Response);
    
    // Move to next guest in queue (server only)
    UFUNCTION(BlueprintCallable, Category = "Check-In")
    void ServeNextGuest();
    
    // Get first available clean room
    UFUNCTION(BlueprintCallable, Category = "Check-In")
    ARoomActor* FindAvailableRoom();
    
    // Query functions
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Check-In")
    int32 GetQueueLength() const { return GuestQueue.Num(); }
    
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Check-In")
    bool HasWaitingGuests() const { return GuestQueue.Num() > 0; }
    
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Check-In")
    AGuestPawn* GetCurrentGuest() const { return CurrentGuest; }
    
    // ===== BLUEPRINT EVENTS =====
    
    UPROPERTY(BlueprintAssignable, Category = "Check-In Events")
    FOnCheckInStarted OnCheckInStarted;
    
    UPROPERTY(BlueprintAssignable, Category = "Check-In Events")
    FOnCheckInCompleted OnCheckInCompleted;
    
private:
    // Interaction callback
    UFUNCTION()
    void OnDeskInteracted(APawn* Interactor);
    
    // Update guest positions in queue
    void UpdateQueuePositions();
};