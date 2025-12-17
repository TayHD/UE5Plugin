// GuestPawn.h
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "GuestData.h"
#include "GuestPawn.generated.h"

// Forward declarations
class ARoomActor;
class AGuestAIController;

// ENUM MUST BE DECLARED FIRST
UENUM(BlueprintType)
enum class EGuestBehaviorState : uint8
{
    Approaching     UMETA(DisplayName = "Approaching Hotel"),
    WaitingAtDesk   UMETA(DisplayName = "Waiting at Check-In"),
    GoingToRoom     UMETA(DisplayName = "Going to Room"),
    InRoom          UMETA(DisplayName = "In Room"),
    Shopping        UMETA(DisplayName = "Shopping"),
    Complaining     UMETA(DisplayName = "Complaining"),
    CheckingOut     UMETA(DisplayName = "Checking Out"),
    Leaving         UMETA(DisplayName = "Leaving Hotel"),
    Panicked        UMETA(DisplayName = "Panicked")
};

// NOW declare the delegate (after the enum)
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnGuestStateChanged, class AGuestPawn*, Guest, EGuestBehaviorState, NewState);

UCLASS()
class FCORECPP_API AGuestPawn : public ACharacter
{
    GENERATED_BODY()
    
public:
    AGuestPawn();
    
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
    virtual void BeginPlay() override;
    
    // ===== GUEST DATA =====
    
    // All information about this guest (replicated for UI)
    UPROPERTY(Replicated, BlueprintReadOnly, Category = "Guest Data")
    FGuestData GuestInfo;
    
    // ===== STATE =====
    
    UPROPERTY(ReplicatedUsing=OnRep_BehaviorState, BlueprintReadOnly, Category = "Guest State")
    EGuestBehaviorState CurrentBehaviorState;
    
    UPROPERTY(Replicated, BlueprintReadOnly, Category = "Guest State")
    ARoomActor* AssignedRoom;
    
    UPROPERTY(Replicated, BlueprintReadOnly, Category = "Guest State")
    bool bHasPaid;
    
    UPROPERTY(Replicated, BlueprintReadOnly, Category = "Guest State")
    bool bHasCheckedIn;
    
    UPROPERTY(Replicated, BlueprintReadOnly, Category = "Guest State")
    float TimeInHotel; // How long they've been here (in seconds)
    
    // ===== SETTINGS =====
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Guest Settings")
    float WalkSpeed = 150.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Guest Settings")
    float MaxStayDuration = 300.0f; // 5 minutes in room before checking out
    
    // ===== FUNCTIONS =====
    
    // Initialize this guest with data
    UFUNCTION(BlueprintCallable, Category = "Guest")
    void InitializeGuest(FGuestData Data);
    
    // Set behavior state (called by AI controller)
    UFUNCTION(BlueprintCallable, Server, Reliable, Category = "Guest")
    void Server_SetBehaviorState(EGuestBehaviorState NewState);
    
    // Assign a room to this guest (called by check-in desk)
    UFUNCTION(BlueprintCallable, Server, Reliable, Category = "Guest")
    void Server_AssignRoom(ARoomActor* Room);
    
    // Mark as checked in (payment processed)
    UFUNCTION(BlueprintCallable, Server, Reliable, Category = "Guest")
    void Server_MarkCheckedIn();
    
    // Check out and leave hotel
    UFUNCTION(BlueprintCallable, Server, Reliable, Category = "Guest")
    void Server_CheckOut();
    
    // Query functions
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Guest")
    bool IsInRoom() const { return CurrentBehaviorState == EGuestBehaviorState::InRoom; }
    
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Guest")
    bool IsWaiting() const { return CurrentBehaviorState == EGuestBehaviorState::WaitingAtDesk; }
    
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Guest")
    FGuestData GetGuestData() const { return GuestInfo; }
    
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Guest")
    EGuestType GetGuestType() const { return GuestInfo.GuestType; }
    
    // ===== REPLICATION CALLBACKS =====
    
    UFUNCTION()
    void OnRep_BehaviorState();
    
    // ===== BLUEPRINT EVENTS =====
    
    UPROPERTY(BlueprintAssignable, Category = "Guest Events")
    FOnGuestStateChanged OnBehaviorStateChanged;
    
protected:
    // Timer for auto-checkout
    FTimerHandle CheckOutTimer;
    
    UFUNCTION()
    void StartCheckOutTimer();
};