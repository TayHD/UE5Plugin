// DoorActor.h
#pragma once

#include "CoreMinimal.h"
#include "InteractableActor.h"
#include "DoorActor.generated.h"

// Forward declarations
class UStaticMeshComponent;
class UNavModifierComponent;

UENUM(BlueprintType)
enum class EDoorState : uint8
{
    Closed  UMETA(DisplayName = "Closed"),
    Opening UMETA(DisplayName = "Opening"),
    Open    UMETA(DisplayName = "Open"),
    Closing UMETA(DisplayName = "Closing")
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
    UStaticMeshComponent* DoorMesh;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* DoorFrame;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UNavModifierComponent* NavModifier;
    
    // ===== PROPERTIES =====
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Door")
    float OpenAngle = 90.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Door")
    float OpenSpeed = 2.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Door")
    float AutoCloseDelay = 3.0f;
    
    // ===== STATE (Replicated) =====
    
    UPROPERTY(ReplicatedUsing=OnRep_DoorState, BlueprintReadOnly, Category = "Door State")
    EDoorState CurrentState;
    
    UPROPERTY(ReplicatedUsing=OnRep_Locked, BlueprintReadOnly, Category = "Door State")
    bool bIsLocked;
    
    // ===== INTERACTION (from InteractableActor) =====
    
    virtual void StartInteraction_Implementation(APawn* InteractingPawn) override;
    virtual bool CanInteract_Implementation(APawn* InteractingPawn) const override;
    
    // ===== DOOR FUNCTIONS =====
    
    UFUNCTION(BlueprintCallable, Category = "Door")
    void Open();
    
    UFUNCTION(BlueprintCallable, Category = "Door")
    void Close();
    
    UFUNCTION(BlueprintCallable, Category = "Door")
    void SetLocked(bool bLocked);
    
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Door")
    bool IsOpen() const { return CurrentState == EDoorState::Open; }
    
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Door")
    bool IsLocked() const { return bIsLocked; }
    
    // ===== REPLICATION CALLBACKS =====
    
    UFUNCTION()
    void OnRep_DoorState();
    
    UFUNCTION()
    void OnRep_Locked();
    
protected:
    FRotator ClosedRotation;
    FRotator OpenRotation;
    FTimerHandle AutoCloseTimer;
    
    void UpdateDoorRotation(float DeltaTime);
    void ScheduleAutoClose();
};