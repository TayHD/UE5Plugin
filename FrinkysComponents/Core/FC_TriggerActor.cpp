// Copyright Frinky 2022

#include "FC_TriggerActor.h"
#include "Net/UnrealNetwork.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "FC_TriggerSubsystem.h"
#include "GameplayTagAssetInterface.h"

AFC_TriggerActor::AFC_TriggerActor()
{
    PrimaryActorTick.bCanEverTick = false;
    bReplicates = true;

    // Create and setup trigger volume
    TriggerVolume = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerVolume"));
    RootComponent = TriggerVolume;
    
    TriggerVolume->SetCollisionProfileName(TEXT("Trigger"));
    TriggerVolume->SetGenerateOverlapEvents(true);
    
    // Default properties
    bCanTriggerMultipleTimes = false;
    bEnabled = true;
    bDebugVisualization = false;
    bHasBeenTriggered = false;
}

void AFC_TriggerActor::BeginPlay()
{
    Super::BeginPlay();

    // Bind overlap events
    if (TriggerVolume)
    {
        TriggerVolume->OnComponentBeginOverlap.AddDynamic(this, &AFC_TriggerActor::OnTriggerBeginOverlap);
        TriggerVolume->OnComponentEndOverlap.AddDynamic(this, &AFC_TriggerActor::OnTriggerEndOverlap);
    }

    // Setup debug visualization
    if (bDebugVisualization && GetWorld()->IsGameWorld())
    {
        const FColor DebugColor = bEnabled ? FColor::Green : FColor::Red;
        DrawDebugBox(GetWorld(), GetActorLocation(), TriggerVolume->GetScaledBoxExtent(), 
            GetActorRotation().Quaternion(), DebugColor, true, -1.0f, 0, 2.0f);
    }
}

void AFC_TriggerActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    // Clean up debug visualization
    if (bDebugVisualization)
    {
        FlushPersistentDebugLines(GetWorld());
    }

    Super::EndPlay(EndPlayReason);
}

void AFC_TriggerActor::OnRep_ReplicatedMovement()
{
    Super::OnRep_ReplicatedMovement();
    UpdateDebugVisualization();
}

void AFC_TriggerActor::UpdateDebugVisualization()
{
    if (!bDebugVisualization || !GetWorld())
        return;

    // Clear old debug visualization
    FlushPersistentDebugLines(GetWorld());
    
    // Draw new debug box
    const FColor DebugColor = bEnabled ? FColor::Green : FColor::Red;
    DrawDebugBox(GetWorld(), GetActorLocation(), TriggerVolume->GetScaledBoxExtent(),
        GetActorRotation().Quaternion(), DebugColor, true, -1.0f, 0, 2.0f);
}

void AFC_TriggerActor::OnTriggerBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, 
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (HasAuthority() && CanTrigger())
    {
        // For each event, if no target is set yet, set the overlapping actor as the target
        for (FTriggerEventData& Event : Events)
        {
            if (!Event.TargetActor)
            {
                Event.TargetActor = OtherActor;
            }
        }

        if (Events.Num() > 0)
        {
            ExecuteEvents();
            OnTriggerActivated.Broadcast(this);

            if (!bCanTriggerMultipleTimes)
            {
                bHasBeenTriggered = true;
                SetEnabled(false);
            }
        }
    }
}



void AFC_TriggerActor::OnTriggerEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    if (HasAuthority())
    {
        OnTriggerDeactivated.Broadcast(this);
    }
}

void AFC_TriggerActor::SetEnabled(bool bNewEnabled)
{
    if (HasAuthority() && bEnabled != bNewEnabled)
    {
        bEnabled = bNewEnabled;
        OnRep_Enabled();
        UpdateDebugVisualization();
    }
}

void AFC_TriggerActor::OnRep_Enabled()
{
    // Update collision
    if (TriggerVolume)
    {
        TriggerVolume->SetGenerateOverlapEvents(bEnabled);
    }

    OnTriggerStateChanged.Broadcast(this);
}

void AFC_TriggerActor::ExecuteEvents()
{
    // Get the trigger subsystem
    if (UGameInstance* GameInstance = GetWorld()->GetGameInstance())
    {
        if (UFC_TriggerSubsystem* TriggerSubsystem = GameInstance->GetSubsystem<UFC_TriggerSubsystem>())
        {
            // Execute each event in sequence
            for (const FTriggerEventData& Event : Events)
            {
                if (Event.Delay > 0.0f)
                {
                    // Schedule delayed event execution
                    FTimerHandle TimerHandle;
                    FTriggerEventData DelayedEvent = Event;
                    GetWorld()->GetTimerManager().SetTimer(TimerHandle, 
                        [this, DelayedEvent, TriggerSubsystem]()
                        {
                            TriggerSubsystem->ExecuteEvent(DelayedEvent, this);
                        }, 
                        Event.Delay, false);
                }
                else
                {
                    // Execute immediately
                    TriggerSubsystem->ExecuteEvent(Event, this);
                }

                // If this is a blocking event, break here
                if (Event.bBlocking)
                {
                    break;
                }
            }
        }
    }
}

void AFC_TriggerActor::Reset()
{
    if (HasAuthority())
    {
        bHasBeenTriggered = false;
        SetEnabled(true);
    }
}

bool AFC_TriggerActor::CanTrigger() const
{
    return bEnabled && (bCanTriggerMultipleTimes || !bHasBeenTriggered);
}

void AFC_TriggerActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(AFC_TriggerActor, bEnabled);
    DOREPLIFETIME(AFC_TriggerActor, bHasBeenTriggered);
}

#if WITH_EDITOR
void AFC_TriggerActor::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
    Super::PostEditChangeProperty(PropertyChangedEvent);

    // Update debug visualization when properties change in editor
    if (PropertyChangedEvent.Property)
    {
        UpdateDebugVisualization();
    }
}

void AFC_TriggerActor::PostEditMove(bool bFinished)
{
    Super::PostEditMove(bFinished);
    UpdateDebugVisualization();
}
#endif