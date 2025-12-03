// Copyright Frinky 2022

#include "FC_TriggerSubsystem.h"
#include "FrinkysComponents/General/FC_Functions.h"
#include "FrinkysComponents/Inventory/FC_InventoryComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "Blueprint/AIBlueprintHelperLibrary.h"

UFC_TriggerSubsystem::UFC_TriggerSubsystem()
    : bDebugVisualizationEnabled(false)
{
}

void UFC_TriggerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(EventUpdateTimerHandle, 
            FTimerDelegate::CreateUObject(this, &UFC_TriggerSubsystem::UpdateActiveEvents),
            0.1f, true);
    }
}

void UFC_TriggerSubsystem::Deinitialize()
{
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(EventUpdateTimerHandle);
    }

    ActiveEvents.Empty();
    ActiveTriggers.Empty();

    Super::Deinitialize();
}

void UFC_TriggerSubsystem::RegisterTrigger(AFC_TriggerActor* Trigger)
{
    if (Trigger && !ActiveTriggers.Contains(Trigger))
    {
        ActiveTriggers.Add(Trigger);
        if (bDebugVisualizationEnabled)
        {
            Trigger->bDebugVisualization = true;
        }
    }
}

void UFC_TriggerSubsystem::UnregisterTrigger(AFC_TriggerActor* Trigger)
{
    ActiveTriggers.Remove(Trigger);
}

void UFC_TriggerSubsystem::ExecuteEvent(const FTriggerEventData& Event, AFC_TriggerActor* TriggerActor)
{
    if (!CheckEventConditions(Event, TriggerActor))
    {
        if (bDebugVisualizationEnabled)
        {
            UE_LOG(LogTemp, Warning, TEXT("Event conditions not met for trigger %s"), 
                *TriggerActor->GetName());
        }
        return;
    }

    StartTrackingEvent(Event, TriggerActor);

    switch (Event.EventType)
    {
        case ETriggerEventType::PlayerControl:
            ExecutePlayerControlEvent(Event, TriggerActor);
            break;
        case ETriggerEventType::CameraControl:
            ExecuteCameraControlEvent(Event, TriggerActor);
            break;
        case ETriggerEventType::Animation:
            ExecuteAnimationEvent(Event, TriggerActor);
            break;
        case ETriggerEventType::WorldControl:
            ExecuteWorldControlEvent(Event, TriggerActor);
            break;
        case ETriggerEventType::Narrative:
            ExecuteNarrativeEvent(Event, TriggerActor);
            break;
    }
}

void UFC_TriggerSubsystem::StartTrackingEvent(const FTriggerEventData& Event, AFC_TriggerActor* TriggerActor)
{
    if (UWorld* World = GetWorld())
    {
        FActiveEventInfo EventInfo;
        EventInfo.Event = Event;
        EventInfo.TriggerActor = TriggerActor;
        EventInfo.StartTime = World->GetTimeSeconds();
        EventInfo.EndTime = EventInfo.StartTime + Event.Duration;
        EventInfo.bBlocking = Event.bBlocking;

        ActiveEvents.Add(EventInfo);
    }
}

void UFC_TriggerSubsystem::UpdateActiveEvents()
{
    if (UWorld* World = GetWorld())
    {
        const float CurrentTime = World->GetTimeSeconds();
        
        for (int32 i = ActiveEvents.Num() - 1; i >= 0; --i)
        {
            FActiveEventInfo& EventInfo = ActiveEvents[i];
            
            if (CurrentTime >= EventInfo.EndTime)
            {
                OnEventCompleted(EventInfo.Event, EventInfo.TriggerActor);
                ActiveEvents.RemoveAt(i);
            }
        }
    }
}

void UFC_TriggerSubsystem::OnEventCompleted(const FTriggerEventData& Event, AFC_TriggerActor* TriggerActor)
{
    if (bDebugVisualizationEnabled)
    {
        UE_LOG(LogTemp, Log, TEXT("Event completed for trigger %s"), 
            *TriggerActor->GetName());
    }

    if (TriggerActor)
    {
        TriggerActor->OnTriggerStateChanged.Broadcast(TriggerActor);
    }
}

void UFC_TriggerSubsystem::ExecutePlayerControlEvent(const FTriggerEventData& Event, AFC_TriggerActor* TriggerActor)
{
    if (!Event.TargetActor || !TriggerActor)
        return;

    switch (Event.PlayerAction)
    {
        case ETriggerPlayerAction::MoveToPoint:
        {
            if (ACharacter* Character = Cast<ACharacter>(Event.TargetActor))
            {
                if (AController* Controller = Character->GetController())
                {
                    UAIBlueprintHelperLibrary::SimpleMoveToLocation(Controller, Event.TargetLocation);
                }
            }
            break;
        }
        case ETriggerPlayerAction::EnableInput:
        {
            if (APawn* Pawn = Cast<APawn>(Event.TargetActor))
            {
                if (APlayerController* PC = Cast<APlayerController>(Pawn->GetController()))
                {
                    PC->EnableInput(PC);
                }
            }
            break;
        }
        case ETriggerPlayerAction::DisableInput:
        {
            if (APawn* Pawn = Cast<APawn>(Event.TargetActor))
            {
                if (APlayerController* PC = Cast<APlayerController>(Pawn->GetController()))
                {
                    PC->DisableInput(PC);
                }
            }
            break;
        }
        case ETriggerPlayerAction::SetSpeed:
        {
            if (ACharacter* Character = Cast<ACharacter>(Event.TargetActor))
            {
                if (UCharacterMovementComponent* MovementComp = Cast<UCharacterMovementComponent>(Character->GetMovementComponent()))
                {
                    MovementComp->MaxWalkSpeed = 600.0f; // Default speed for now
                }
            }
            break;
        }
    }
}

void UFC_TriggerSubsystem::ExecuteCameraControlEvent(const FTriggerEventData& Event, AFC_TriggerActor* TriggerActor)
{
    if (!TriggerActor || !Event.TargetActor)
        return;

    APlayerController* PC = UGameplayStatics::GetPlayerController(TriggerActor, 0);
    if (!PC)
        return;

    switch (Event.CameraAction)
    {
        case ETriggerCameraAction::LookAtTarget:
        {
            // Store the pawn for easy access
            APawn* PlayerPawn = PC->GetPawn();
            if (!PlayerPawn)
                return;

            // Disable input based on settings
            if (Event.bLockPlayerMovement)
            {
                PC->DisableInput(PC);
            }
            else
            {
                PC->SetIgnoreLookInput(true);
            }
            
            // Calculate target position with offsets
            FVector TargetBaseLocation = Event.TargetActor->GetActorLocation();
            FVector FinalTargetLocation = TargetBaseLocation + 
                                        Event.TargetOffset + 
                                        FVector(0.0f, 0.0f, Event.VerticalOffset);
            
            // Set up timer for continuous rotation update
            FTimerHandle* UpdateTimerHandle = new FTimerHandle();
            GetWorld()->GetTimerManager().SetTimer(
                *UpdateTimerHandle,
                [this, PC, PlayerPawn, FinalTargetLocation, Event]()
                {
                    if (!PC || !PlayerPawn)
                        return;

                    // Get current camera location
                    FVector CameraLocation = PlayerPawn->GetActorLocation();
                    if (PC->PlayerCameraManager)
                    {
                        CameraLocation = PC->PlayerCameraManager->GetCameraLocation();
                    }

                    // Calculate direction to target
                    FVector DirectionToTarget = (FinalTargetLocation - CameraLocation).GetSafeNormal();
                    FRotator TargetRotation = DirectionToTarget.Rotation();
                    
                    // Smoothly interpolate to target rotation
                    FRotator NewRotation = FMath::RInterpTo(
                        PC->GetControlRotation(),
                        TargetRotation,
                        GetWorld()->GetDeltaSeconds(),
                        Event.RotationSpeed
                    );

                    PC->SetControlRotation(NewRotation);
                },
                0.0f,  // Initial delay
                true,  // Looping
                0.0f   // Start immediately
            );

            // Set up timer to restore control after duration
            FTimerHandle* RestoreTimerHandle = new FTimerHandle();
            GetWorld()->GetTimerManager().SetTimer(
                *RestoreTimerHandle,
                [PC, UpdateTimerHandle, Event, this]()
                {
                    if (!PC)
                        return;

                    // Clear the update timer
                    GetWorld()->GetTimerManager().ClearTimer(*UpdateTimerHandle);
                    delete UpdateTimerHandle;

                    // Restore input based on what was disabled
                    if (Event.bLockPlayerMovement)
                    {
                        PC->EnableInput(PC);
                    }
                    else
                    {
                        PC->SetIgnoreLookInput(false);
                    }
                },
                Event.Duration,
                false
            );
            break;
        }
        case ETriggerCameraAction::SetFOV:
        {
            if (PC->PlayerCameraManager)
            {
                PC->PlayerCameraManager->SetFOV(Event.TargetFOV);
            }
            break;
        }
        default:
            break;
    }
}

void UFC_TriggerSubsystem::ExecuteWorldControlEvent(const FTriggerEventData& Event, AFC_TriggerActor* TriggerActor)
{
    if (!TriggerActor)
        return;

    UWorld* World = TriggerActor->GetWorld();
    if (!World)
        return;

    switch (Event.WorldAction)
    {
        case ETriggerWorldAction::SpawnActor:
        {
            if (Event.ActorToSpawn)
            {
                FTransform SpawnTransform = TriggerActor->GetActorTransform();
                World->SpawnActor<AActor>(Event.ActorToSpawn, SpawnTransform);
            }
            break;
        }
        case ETriggerWorldAction::DestroyActor:
        {
            if (Event.TargetActor)
            {
                Event.TargetActor->Destroy();
            }
            break;
        }
        case ETriggerWorldAction::ShowActor:
        {
            if (Event.TargetActor)
            {
                Event.TargetActor->SetActorHiddenInGame(false);
                Event.TargetActor->SetActorEnableCollision(true);
            }
            break;
        }
        case ETriggerWorldAction::HideActor:
        {
            if (Event.TargetActor)
            {
                Event.TargetActor->SetActorHiddenInGame(true);
                Event.TargetActor->SetActorEnableCollision(false);
            }
            break;
        }
        case ETriggerWorldAction::LoadLevel:
        {
            if (!Event.LevelName.IsNone())
            {
                UGameplayStatics::OpenLevel(World, Event.LevelName);
            }
            break;
        }
    }
}

void UFC_TriggerSubsystem::ExecuteAnimationEvent(const FTriggerEventData& Event, AFC_TriggerActor* TriggerActor)
{
    if (!Event.TargetActor)
        return;

    ACharacter* Character = Cast<ACharacter>(Event.TargetActor);
    if (!Character)
        return;

    UAnimInstance* AnimInstance = Character->GetMesh()->GetAnimInstance();
    if (!AnimInstance)
        return;

    // Basic implementation - will need to be expanded with specific animation parameters
    AnimInstance->StopAllMontages(0.25f);
}

void UFC_TriggerSubsystem::ExecuteNarrativeEvent(const FTriggerEventData& Event, AFC_TriggerActor* TriggerActor)
{
    if (!TriggerActor)
        return;

    // Log the narrative event for now - will need to be expanded with UI/dialog system integration
    UE_LOG(LogTemp, Log, TEXT("Narrative Event from trigger: %s"), *TriggerActor->GetName());
}

bool UFC_TriggerSubsystem::CheckEventConditions(const FTriggerEventData& Event, AFC_TriggerActor* TriggerActor)
{
    for (const FTriggerConditionData& Condition : Event.Conditions)
    {
        bool bConditionMet = false;

        switch (Condition.ConditionType)
        {
            case ETriggerConditionType::StateCondition:
                bConditionMet = CheckStateCondition(Condition);
                break;
            case ETriggerConditionType::InventoryCondition:
                bConditionMet = CheckInventoryCondition(Condition);
                break;
            case ETriggerConditionType::PlayerActionCondition:
                bConditionMet = CheckPlayerActionCondition(Condition);
                break;
            case ETriggerConditionType::LookAtCondition:
                bConditionMet = CheckLookAtCondition(Condition);
                break;
        }

        if (!bConditionMet)
        {
            return false;
        }
    }

    return true;
}

bool UFC_TriggerSubsystem::CheckStateCondition(const FTriggerConditionData& Condition)
{
    // Basic implementation - will need integration with game state system
    return true;
}

bool UFC_TriggerSubsystem::CheckInventoryCondition(const FTriggerConditionData& Condition)
{
    if (!Condition.RequiredItem)
        return false;

    ACharacter* PlayerCharacter = UGameplayStatics::GetPlayerCharacter(this, 0);
    if (!PlayerCharacter)
        return false;

    UFC_InventoryComponent* Inventory = UFC_Functions::GetInventoryComponent(PlayerCharacter);
    if (!Inventory)
        return false;

    return Inventory->HasItemOfClass(Condition.RequiredItem) != nullptr;
}

bool UFC_TriggerSubsystem::CheckPlayerActionCondition(const FTriggerConditionData& Condition)
{
    // Basic implementation - will need integration with input system
    return true;
}

bool UFC_TriggerSubsystem::CheckLookAtCondition(const FTriggerConditionData& Condition)
{
    if (!Condition.LookAtTarget)
        return false;

    APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
    if (!PC)
        return false;

    FVector CameraLocation;
    FRotator CameraRotation;
    PC->GetPlayerViewPoint(CameraLocation, CameraRotation);

    FVector DirectionToTarget = (Condition.LookAtTarget->GetActorLocation() - CameraLocation).GetSafeNormal();
    FVector CameraForward = CameraRotation.Vector();

    const float MinDotProduct = 0.866f; // About 30 degrees
    return (FVector::DotProduct(DirectionToTarget, CameraForward) > MinDotProduct);
}

void UFC_TriggerSubsystem::EnableDebugVisualization(bool bEnable)
{
    bDebugVisualizationEnabled = bEnable;

    for (AFC_TriggerActor* Trigger : ActiveTriggers)
    {
        if (Trigger)
        {
            Trigger->bDebugVisualization = bEnable;
        }
    }
}

void UFC_TriggerSubsystem::DebugPrintActiveEvents() const
{
    if (!bDebugVisualizationEnabled)
        return;

    UE_LOG(LogTemp, Log, TEXT("=== Active Events ==="));
    for (const FActiveEventInfo& EventInfo : ActiveEvents)
    {
        UE_LOG(LogTemp, Log, TEXT("Trigger: %s, Event Type: %s, Time Remaining: %f"),
            *EventInfo.TriggerActor->GetName(),
            *UEnum::GetValueAsString(EventInfo.Event.EventType),
            EventInfo.EndTime - GetWorld()->GetTimeSeconds());
    }
    UE_LOG(LogTemp, Log, TEXT("=================="));
}