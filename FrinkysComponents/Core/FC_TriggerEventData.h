// Copyright Frinky 2022

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "FC_TriggerEventData.generated.h"

UENUM(BlueprintType)
enum class ETriggerEventType : uint8
{
    None                    UMETA(DisplayName = "None"),
    PlayerControl          UMETA(DisplayName = "Player Control"),
    CameraControl          UMETA(DisplayName = "Camera Control"),
    Animation              UMETA(DisplayName = "Animation"),
    WorldControl           UMETA(DisplayName = "World Control"),
    Narrative              UMETA(DisplayName = "Narrative")
};

UENUM(BlueprintType, Meta = (DisplayName = "Player Actions"))
enum class ETriggerPlayerAction : uint8
{
    None            UMETA(DisplayName = "None"),
    MoveToPoint     UMETA(DisplayName = "Move Player To Location"),
    EnableInput     UMETA(DisplayName = "Enable Player Control"),
    DisableInput    UMETA(DisplayName = "Disable Player Control"),
    SetSpeed        UMETA(DisplayName = "Set Player Speed")
};

UENUM(BlueprintType, Meta = (DisplayName = "Camera Actions"))
enum class ETriggerCameraAction : uint8
{
    None            UMETA(DisplayName = "None"),
    LookAtTarget    UMETA(DisplayName = "Look At Target"),
    SetFOV          UMETA(DisplayName = "Change Camera FOV")
};

UENUM(BlueprintType, Meta = (DisplayName = "World Actions"))
enum class ETriggerWorldAction : uint8
{
    None             UMETA(DisplayName = "None"),
    SpawnActor       UMETA(DisplayName = "Spawn Actor"),
    DestroyActor     UMETA(DisplayName = "Destroy Actor"),
    ShowActor        UMETA(DisplayName = "Show Actor"),
    HideActor        UMETA(DisplayName = "Hide Actor"),
    LoadLevel        UMETA(DisplayName = "Change Level")
};

UENUM(BlueprintType)
enum class ETriggerConditionType : uint8
{
    None                    UMETA(DisplayName = "None"),
    StateCondition         UMETA(DisplayName = "State Check"),
    InventoryCondition     UMETA(DisplayName = "Inventory Check"),
    PlayerActionCondition  UMETA(DisplayName = "Player Action"),
    LookAtCondition       UMETA(DisplayName = "Look At Target")
};

/**
 * Data structure defining conditions that must be met for an event to execute
 */
USTRUCT(BlueprintType)
struct FRINKYSCOMPONENTS_API FTriggerConditionData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Condition")
    ETriggerConditionType ConditionType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Condition")
    FGameplayTag StateTag;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Condition")
    TSubclassOf<AActor> RequiredItem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Condition")
    AActor* LookAtTarget;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Condition")
    float RequiredLookAtTime;

    FTriggerConditionData()
        : ConditionType(ETriggerConditionType::None)
        , RequiredLookAtTime(0.0f)
    {}
};

/**
 * Base data structure for trigger events
 */
USTRUCT(BlueprintType, Meta = (DisplayName = "Trigger Event"))
struct FRINKYSCOMPONENTS_API FTriggerEventData
{
    GENERATED_BODY()

    // Main event settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "1. Event Type", meta = (DisplayName = "Event Type"))
    ETriggerEventType EventType;

    // Action selection based on event type
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "2. Action", meta = (EditCondition = "EventType == ETriggerEventType::PlayerControl", EditConditionHides))
    ETriggerPlayerAction PlayerAction;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "2. Action", meta = (EditCondition = "EventType == ETriggerEventType::CameraControl", EditConditionHides))
    ETriggerCameraAction CameraAction;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "2. Action", meta = (EditCondition = "EventType == ETriggerEventType::WorldControl", EditConditionHides))
    ETriggerWorldAction WorldAction;

    // Common parameters
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "3. Parameters", meta = (DisplayName = "Target Actor"))
    AActor* TargetActor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "3. Parameters", meta = (EditCondition = "PlayerAction == ETriggerPlayerAction::MoveToPoint", EditConditionHides))
    FVector TargetLocation;

    // Camera control parameters
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "3. Parameters", meta = (EditCondition = "CameraAction == ETriggerCameraAction::LookAtTarget", EditConditionHides))
    float RotationSpeed = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "3. Parameters", meta = (EditCondition = "CameraAction == ETriggerCameraAction::LookAtTarget", EditConditionHides))
    FVector TargetOffset = FVector(0.0f, 0.0f, 0.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "3. Parameters", meta = (EditCondition = "CameraAction == ETriggerCameraAction::LookAtTarget", EditConditionHides))
    float VerticalOffset = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "3. Parameters", meta = (EditCondition = "CameraAction == ETriggerCameraAction::LookAtTarget", EditConditionHides))
    bool bLockPlayerMovement = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "3. Parameters", meta = (EditCondition = "CameraAction == ETriggerCameraAction::SetFOV", EditConditionHides))
    float TargetFOV = 90.0f;

    // World control parameters
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "3. Parameters", meta = (EditCondition = "WorldAction == ETriggerWorldAction::SpawnActor", EditConditionHides))
    TSubclassOf<AActor> ActorToSpawn;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "3. Parameters", meta = (EditCondition = "WorldAction == ETriggerWorldAction::LoadLevel", EditConditionHides))
    FName LevelName;

    // Timing settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "4. Timing", meta = (DisplayName = "Delay Before Start (seconds)"))
    float Delay = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "4. Timing", meta = (DisplayName = "Duration (seconds)"))
    float Duration = 0.0f;

    // Advanced settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "5. Advanced", meta = (DisplayName = "Wait For Completion"))
    bool bBlocking = false;

    // Conditions that must be met
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "5. Advanced")
    TArray<FTriggerConditionData> Conditions;

    FTriggerEventData()
           : EventType(ETriggerEventType::None)
           , PlayerAction(ETriggerPlayerAction::None)
           , CameraAction(ETriggerCameraAction::None)
           , WorldAction(ETriggerWorldAction::None)
           , RotationSpeed(5.0f)
           , TargetOffset(FVector(0.0f, 0.0f, 0.0f))
           , VerticalOffset(0.0f)
           , bLockPlayerMovement(true)
           , TargetFOV(90.0f)
           , Delay(0.0f)
           , Duration(0.0f)
           , bBlocking(false)
    {}
};