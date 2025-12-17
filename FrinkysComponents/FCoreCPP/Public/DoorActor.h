// DoorActor.h
#pragma once

#include "CoreMinimal.h"
#include "InteractableActor.h"
#include "Components/BoxComponent.h"
#include "DoorActor.generated.h"

UENUM(BlueprintType)
enum class EDoorState : uint8
{
    Closed      UMETA(DisplayName = "Closed"),
    Opening     UMETA(DisplayName = "Opening"),
    Open        UMETA(DisplayName = "Open"),
    Closing     UMETA(DisplayName = "Closing")
};

UCLASS()
class FCORECPP_API ADoorActor : public AInteractableActor
{
    GENERATED_BODY()
    
public:
    ADoorActor();
    
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
    
    // ===== COMPONENTS =====
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* DoorFrame;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* HingePivot; // Pivot point for rotation
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* DoorMesh;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UBoxComponent* DoorwayBlocker; // Blocks doorway when closed
    
    // ===== STATE =====
    
    UPROPERTY(ReplicatedUsing=OnRep_DoorState, BlueprintReadOnly, Category = "Door State")
    EDoorState CurrentState;
    
    UPROPERTY(Replicated, BlueprintReadOnly, Category = "Door State")
    bool bIsLocked;
    
    // Replicated rotation values so clients can animate smoothly
    UPROPERTY(Replicated, BlueprintReadOnly, Category = "Door State")
    float CurrentRotation; // Current door rotation
    
    UPROPERTY(Replicated, BlueprintReadOnly, Category = "Door State")
    float TargetRotation; // Target rotation based on state
    
    // ===== SETTINGS =====
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Door Settings")
    float OpenAngle = 90.0f; // How far door opens (degrees)
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Door Settings")
    float OpenSpeed = 2.0f; // How fast door opens
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Door Settings")
    bool bStartsLocked = false;
    
    // ===== FUNCTIONS =====
    
    // Door-specific functions
    UFUNCTION(BlueprintCallable, Category = "Door")
    void OpenDoor();
    
    UFUNCTION(BlueprintCallable, Category = "Door")
    void CloseDoor();
    
    UFUNCTION(BlueprintCallable, Category = "Door")
    void SetLocked(bool bLocked);
    
    // Query functions
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Door")
    bool IsOpen() const { return CurrentState == EDoorState::Open || CurrentState == EDoorState::Opening; }
    
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Door")
    bool IsClosed() const { return CurrentState == EDoorState::Closed || CurrentState == EDoorState::Closing; }
    
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Door")
    bool IsLocked() const { return bIsLocked; }
    
    // ===== OVERRIDE INTERACTABLE FUNCTIONS =====
    
    virtual void OnInteractionStarted_Implementation(AActor* Interactor) override;
    virtual bool CanInteract(AActor* Interactor) const override;
    
    // ===== REPLICATION CALLBACKS =====
    
    UFUNCTION()
    void OnRep_DoorState();
    
private:
    // Animation (runs on all machines)
    void AnimateDoor(float DeltaTime);
};