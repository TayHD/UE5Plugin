// InteractableActor.cpp
#include "InteractableActor.h"
#include "GuestPawn.h"
#include "HotelPlayerController.h"
#include "Net/UnrealNetwork.h"
#include "TimerManager.h"

AInteractableActor::AInteractableActor()
{
    PrimaryActorTick.bCanEverTick = false;
    bReplicates = true;
    
    // Create root
    InteractableRoot = CreateDefaultSubobject<USceneComponent>(TEXT("InteractableRoot"));
    RootComponent = InteractableRoot;
    
    // Create player interaction point
    PlayerInteractionPoint = CreateDefaultSubobject<UFC_InteractionPoint>(TEXT("PlayerInteractionPoint"));
    PlayerInteractionPoint->SetupAttachment(InteractableRoot);
    PlayerInteractionPoint->InteractionName = InteractionName;
    PlayerInteractionPoint->InteractionTime = 0.0f; // Instant by default
    
    // Initialize state
    bIsCurrentlyInUse = false;
    CurrentUser = nullptr;
    InteractionStartTime = 0.0f;
}

void AInteractableActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    
    DOREPLIFETIME(AInteractableActor, bIsCurrentlyInUse);
    DOREPLIFETIME(AInteractableActor, CurrentUser);
    DOREPLIFETIME(AInteractableActor, InteractionStartTime);
}

void AInteractableActor::BeginPlay()
{
    Super::BeginPlay();
    
    // Bind player interaction
    if (PlayerInteractionPoint)
    {
        PlayerInteractionPoint->OnInteract.AddDynamic(this, &AInteractableActor::OnPlayerInteracted);
        PlayerInteractionPoint->InteractionName = InteractionName;
        
        // Disable if players can't use this
        if (!bPlayerCanInteract)
        {
            PlayerInteractionPoint->isEnabled = false;
        }
    }
}

void AInteractableActor::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

bool AInteractableActor::StartInteraction(AActor* Interactor)
{
    // This should only be called on server (via PlayerController RPC or AI)
    if (!HasAuthority())
    {
        UE_LOG(LogTemp, Error, TEXT("InteractableActor: StartInteraction called on client! This should never happen."));
        return false;
    }
    
    if (!Interactor)
    {
        UE_LOG(LogTemp, Warning, TEXT("InteractableActor: Null interactor"));
        return false;
    }
    
    // Check if can interact
    if (!CanInteract(Interactor))
    {
        UE_LOG(LogTemp, Warning, TEXT("InteractableActor: %s cannot interact with %s"), 
               *Interactor->GetName(), *GetName());
        return false;
    }
    
    // Mark as in use
    bIsCurrentlyInUse = true;
    CurrentUser = Interactor;
    InteractionStartTime = GetWorld()->GetTimeSeconds();
    
    // Handle based on type
    switch (InteractionType)
    {
        case EInteractionType::Instant:
            // Complete immediately
            OnInteractionStarted(Interactor);
            CompleteInteraction(Interactor);
            break;
            
        case EInteractionType::Duration:
        case EInteractionType::Animated:
            // Start timer for completion
            OnInteractionStarted(Interactor);
            GetWorldTimerManager().SetTimer(InteractionTimer, [this, Interactor]()
            {
                CompleteInteraction(Interactor);
            }, InteractionDuration, false);
            break;
            
        case EInteractionType::Transaction:
            // Check affordability, then complete
            if (CanAffordInteraction(Interactor))
            {
                OnInteractionStarted(Interactor);
                CompleteInteraction(Interactor);
            }
            else
            {
                CancelInteraction(Interactor);
                return false;
            }
            break;
    }
    
    // Broadcast event
    OnInteractionStartedEvent.Broadcast(Interactor, this);
    
    UE_LOG(LogTemp, Log, TEXT("InteractableActor: %s started interacting with %s"), 
           *Interactor->GetName(), *GetName());
    
    return true;
}

bool AInteractableActor::CanInteract(AActor* Interactor) const
{
    if (!Interactor)
    {
        return false;
    }
    
    // Check if already in use (unless multiple allowed)
    if (bIsCurrentlyInUse && !bCanBeUsedByMultiple)
    {
        return false;
    }
    
    // Check if this actor type can interact
    bool bIsPlayer = Interactor->IsA(APawn::StaticClass()) && Cast<APawn>(Interactor)->IsPlayerControlled();
    bool bIsAI = Interactor->IsA(AGuestPawn::StaticClass());
    
    if (bIsPlayer && !bPlayerCanInteract)
    {
        return false;
    }
    
    if (bIsAI && !bAICanInteract)
    {
        return false;
    }
    
    return true;
}

void AInteractableActor::CompleteInteraction(AActor* Interactor)
{
    if (!HasAuthority())
    {
        UE_LOG(LogTemp, Error, TEXT("InteractableActor: CompleteInteraction called on client!"));
        return;
    }
    
    if (!Interactor)
    {
        return;
    }
    
    // Call Blueprint event
    OnInteractionCompleted(Interactor);
    OnInteractionCompletedEvent.Broadcast(Interactor, this);
    
    // Clear state
    bIsCurrentlyInUse = false;
    CurrentUser = nullptr;
    
    // Clear timer if it exists
    GetWorldTimerManager().ClearTimer(InteractionTimer);
    
    UE_LOG(LogTemp, Log, TEXT("InteractableActor: %s completed interaction with %s"), 
           *Interactor->GetName(), *GetName());
}

void AInteractableActor::CancelInteraction(AActor* Interactor)
{
    if (!HasAuthority())
    {
        UE_LOG(LogTemp, Error, TEXT("InteractableActor: CancelInteraction called on client!"));
        return;
    }
    
    if (!Interactor)
    {
        return;
    }
    
    // Call Blueprint event
    OnInteractionCancelled(Interactor);
    
    // Clear state
    bIsCurrentlyInUse = false;
    CurrentUser = nullptr;
    
    // Clear timer
    GetWorldTimerManager().ClearTimer(InteractionTimer);
    
    UE_LOG(LogTemp, Log, TEXT("InteractableActor: %s cancelled interaction with %s"), 
           *Interactor->GetName(), *GetName());
}

void AInteractableActor::OnInteractionStarted_Implementation(AActor* Interactor)
{
    // Override in child classes
}

void AInteractableActor::OnInteractionCompleted_Implementation(AActor* Interactor)
{
    // Override in child classes
}

void AInteractableActor::OnInteractionCancelled_Implementation(AActor* Interactor)
{
    // Override in child classes
}

void AInteractableActor::OnPlayerInteracted(APawn* Interactor)
{
    // Player pressed E - call server RPC on their PlayerController
    AHotelPlayerController* PC = Cast<AHotelPlayerController>(Interactor->GetController());
    if (!PC)
    {
        UE_LOG(LogTemp, Warning, TEXT("InteractableActor: Interactor has no HotelPlayerController"));
        return;
    }
    
    // Call server RPC to start interaction
    PC->Server_RequestInteraction(this);
}

bool AInteractableActor::CanAffordInteraction(AActor* Interactor) const
{
    // TODO: Check if actor has enough money
    // For now, assume they can afford it
    // Will integrate with economy system later
    return true;
}