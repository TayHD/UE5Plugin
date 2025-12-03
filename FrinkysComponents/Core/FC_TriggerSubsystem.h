// Copyright Frinky 2022

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "FC_TriggerEventData.h"
#include "FC_TriggerActor.h"
#include "FC_TriggerSubsystem.generated.h"

/**
 * Subsystem that manages all trigger actors and handles event execution
 */
UCLASS()
class FRINKYSCOMPONENTS_API UFC_TriggerSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UFC_TriggerSubsystem();

    // Begin USubsystem
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
    // End USubsystem

    // Register/unregister triggers with the subsystem
    void RegisterTrigger(AFC_TriggerActor* Trigger);
    void UnregisterTrigger(AFC_TriggerActor* Trigger);

    // Execute a single event
    UFUNCTION(BlueprintCallable, Category = "Trigger|Events")
    void ExecuteEvent(const FTriggerEventData& Event, AFC_TriggerActor* TriggerActor);

    // Check if event conditions are met
    UFUNCTION(BlueprintCallable, Category = "Trigger|Events")
    bool CheckEventConditions(const FTriggerEventData& Event, AFC_TriggerActor* TriggerActor);

    // Get all active triggers
    UFUNCTION(BlueprintCallable, Category = "Trigger")
    TArray<AFC_TriggerActor*> GetActiveTriggers() const { return ActiveTriggers; }

    // Debug functions
    UFUNCTION(BlueprintCallable, Category = "Trigger|Debug")
    void EnableDebugVisualization(bool bEnable);

    UFUNCTION(BlueprintCallable, Category = "Trigger|Debug")
    void DebugPrintActiveEvents() const;

protected:
    // Event execution handlers
    void ExecutePlayerControlEvent(const FTriggerEventData& Event, AFC_TriggerActor* TriggerActor);
    void ExecuteCameraControlEvent(const FTriggerEventData& Event, AFC_TriggerActor* TriggerActor);
    void ExecuteAnimationEvent(const FTriggerEventData& Event, AFC_TriggerActor* TriggerActor);
    void ExecuteWorldControlEvent(const FTriggerEventData& Event, AFC_TriggerActor* TriggerActor);
    void ExecuteNarrativeEvent(const FTriggerEventData& Event, AFC_TriggerActor* TriggerActor);

    // Condition checking
    bool CheckStateCondition(const FTriggerConditionData& Condition);
    bool CheckInventoryCondition(const FTriggerConditionData& Condition);
    bool CheckPlayerActionCondition(const FTriggerConditionData& Condition);
    bool CheckLookAtCondition(const FTriggerConditionData& Condition);

    // Active event tracking
    void StartTrackingEvent(const FTriggerEventData& Event, AFC_TriggerActor* TriggerActor);
    void StopTrackingEvent(const FTriggerEventData& Event, AFC_TriggerActor* TriggerActor);
    void UpdateActiveEvents();

    // Event completion callback
    void OnEventCompleted(const FTriggerEventData& Event, AFC_TriggerActor* TriggerActor);

private:
    // Currently active triggers
    UPROPERTY()
    TArray<AFC_TriggerActor*> ActiveTriggers;

    // Currently executing events
    struct FActiveEventInfo
    {
        FTriggerEventData Event;
        AFC_TriggerActor* TriggerActor;
        float StartTime;
        float EndTime;
        bool bBlocking;

        FActiveEventInfo() : TriggerActor(nullptr), StartTime(0.f), EndTime(0.f), bBlocking(false) {}
    };
    TArray<FActiveEventInfo> ActiveEvents;

    // Debug settings
    bool bDebugVisualizationEnabled;

    // Timer handle for updating active events
    FTimerHandle EventUpdateTimerHandle;
};