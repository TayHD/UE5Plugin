// Copyright Frinky 2022

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/BoxComponent.h"
#include "FC_TriggerEventData.h"
#include "GameplayTagAssetInterface.h"
#include "FC_TriggerActor.generated.h"



DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTriggerEvent, class AFC_TriggerActor*, Trigger);

/**
 * Base class for trigger volumes that can execute various game events
 */
UCLASS(Blueprintable, ClassGroup=(FRINKYCORE))
class FRINKYSCOMPONENTS_API AFC_TriggerActor : public AActor
{
    GENERATED_BODY()

public:
    AFC_TriggerActor();

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
    // The trigger volume
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Trigger")
    UBoxComponent* TriggerVolume;

    // Whether this trigger can be activated multiple times
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trigger|Settings")
    bool bCanTriggerMultipleTimes;

    // Whether the trigger is currently enabled
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trigger|Settings", ReplicatedUsing = OnRep_Enabled)
    bool bEnabled;

    // Whether debug visualization is enabled
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trigger|Debug")
    bool bDebugVisualization;

    // List of events to execute when triggered
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trigger|Events")
    TArray<FTriggerEventData> Events;

    // Called when the trigger is activated
    UPROPERTY(BlueprintAssignable, Category = "Trigger|Events")
    FOnTriggerEvent OnTriggerActivated;

    // Called when the trigger is deactivated
    UPROPERTY(BlueprintAssignable, Category = "Trigger|Events")
    FOnTriggerEvent OnTriggerDeactivated;

    // Called when the trigger is enabled/disabled
    UPROPERTY(BlueprintAssignable, Category = "Trigger|Events")
    FOnTriggerEvent OnTriggerStateChanged;

    // Whether this trigger has been activated
    UPROPERTY(BlueprintReadOnly, Replicated, Category = "Trigger")
    bool bHasBeenTriggered;

protected:
    // Overlap event handlers
    UFUNCTION()
    virtual void OnTriggerBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, 
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    virtual void OnTriggerEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

    // Rep notifies
    UFUNCTION()
    virtual void OnRep_Enabled();

public:
    // Enable/disable the trigger
    UFUNCTION(BlueprintCallable, Category = "Trigger")
    virtual void SetEnabled(bool bNewEnabled);

    // Execute the trigger's events
    UFUNCTION(BlueprintCallable, Category = "Trigger")
    virtual void ExecuteEvents();

    // Reset the trigger's state
    UFUNCTION(BlueprintCallable, Category = "Trigger")
    virtual void Reset();

    // Check if the trigger can be activated
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Trigger")
    virtual bool CanTrigger() const;

protected:
    // Network setup
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
    
    // Handle movement replication
    virtual void OnRep_ReplicatedMovement() override;
    
    // Update debug visualization
    void UpdateDebugVisualization();

#if WITH_EDITOR
    virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
    virtual void PostEditMove(bool bFinished) override;
#endif
};