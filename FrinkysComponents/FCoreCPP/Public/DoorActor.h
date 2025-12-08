// DoorActor.h
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/BoxComponent.h"
#include "FrinkysComponents/General/FC_InteractionPoint.h"
#include "DoorActor.generated.h"

// Forward declarations
class ARoomActor;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDoorStateChanged, bool, bIsOpen);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDoorLocked, bool, bLocked);

UCLASS()
class FCORECPP_API ADoorActor : public AActor
{
    GENERATED_BODY()
    
public:
    ADoorActor();
    
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
    
    // ===== COMPONENTS =====
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* DoorRoot;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* DoorFrame;
    
    // Hinge pivot point (movable in editor - position this at the door hinge)
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* HingePivot;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* DoorMesh;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UFC_InteractionPoint* InteractionPoint;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UBoxComponent* DoorwayBlocker; // Blocks movement when door closed
    
    // ===== REFERENCES =====
    
    // The room this door belongs to (optional, for auto-lock behavior)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Door Settings")
    ARoomActor* AssociatedRoom;
    
    // ===== STATE =====
    
    UPROPERTY(ReplicatedUsing=OnRep_DoorOpen, BlueprintReadOnly, Category = "Door State")
    bool bIsOpen;
    
    UPROPERTY(ReplicatedUsing=OnRep_DoorLocked, BlueprintReadOnly, Category = "Door State")
    bool bIsLocked;
    
    // ===== SETTINGS =====
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Door Settings")
    float OpenAngle = 90.0f; // How far door swings open
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Door Settings")
    float DoorOpenSpeed = 3.0f; // How fast door animates
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Door Settings")
    bool bAutoClose = false; // Automatically close after delay
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Door Settings", meta = (EditCondition = "bAutoClose"))
    float AutoCloseDelay = 3.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Door Settings")
    bool bStartsLocked = false;
    
    // ===== FUNCTIONS =====
    
    // Toggle door open/closed (called by interaction)
    UFUNCTION(BlueprintCallable, Server, Reliable, Category = "Door")
    void Server_ToggleDoor(APawn* Interactor);
    
    // Set locked state
    UFUNCTION(BlueprintCallable, Server, Reliable, Category = "Door")
    void Server_SetLocked(bool bLocked);
    
    // Force open/close (admin/scripted)
    UFUNCTION(BlueprintCallable, Server, Reliable, Category = "Door")
    void Server_SetOpen(bool bOpen);
    
    // Query functions
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Door")
    bool IsOpen() const { return bIsOpen; }
    
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Door")
    bool IsLocked() const { return bIsLocked; }
    
    // ===== REPLICATION CALLBACKS =====
    
    UFUNCTION()
    void OnRep_DoorOpen();
    
    UFUNCTION()
    void OnRep_DoorLocked();
    
    // ===== BLUEPRINT EVENTS =====
    
    UPROPERTY(BlueprintAssignable, Category = "Door Events")
    FOnDoorStateChanged OnDoorOpened;
    
    UPROPERTY(BlueprintAssignable, Category = "Door Events")
    FOnDoorStateChanged OnDoorClosed;
    
    UPROPERTY(BlueprintAssignable, Category = "Door Events")
    FOnDoorLocked OnDoorLockChanged;
    
private:
    // Interaction callback
    UFUNCTION()
    void OnInteracted(APawn* Interactor);
    
    // Animation
    void AnimateDoor(float DeltaTime);
    float CurrentDoorRotation; // Current yaw angle
    float TargetDoorRotation;  // Target yaw angle
    
    // Auto-close timer
    FTimerHandle AutoCloseTimer;
    void AutoCloseDoor();
};