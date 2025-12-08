// DoorActor.cpp
#include "DoorActor.h"
#include "RoomActor.h"
#include "Net/UnrealNetwork.h"
#include "TimerManager.h"

ADoorActor::ADoorActor()
{
    PrimaryActorTick.bCanEverTick = true;
    bReplicates = true;
    
    // Create root
    DoorRoot = CreateDefaultSubobject<USceneComponent>(TEXT("DoorRoot"));
    RootComponent = DoorRoot;
    
    // Create door frame (static)
    DoorFrame = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DoorFrame"));
    DoorFrame->SetupAttachment(DoorRoot);
    DoorFrame->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    DoorFrame->SetCollisionResponseToAllChannels(ECR_Block);
    
    // Create hinge pivot (this is what rotates)
    HingePivot = CreateDefaultSubobject<USceneComponent>(TEXT("HingePivot"));
    HingePivot->SetupAttachment(DoorRoot);
    HingePivot->SetMobility(EComponentMobility::Movable);
    
    // Create door mesh (attaches to hinge pivot so it rotates around hinge)
    DoorMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DoorMesh"));
    DoorMesh->SetupAttachment(HingePivot); // Attaches to hinge, not root
    DoorMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision); // Frame handles collision
    
    // Create interaction point
    InteractionPoint = CreateDefaultSubobject<UFC_InteractionPoint>(TEXT("InteractionPoint"));
    InteractionPoint->SetupAttachment(DoorRoot);
    InteractionPoint->InteractionName = "Open Door";
    InteractionPoint->InteractionTime = 0.0f; // Instant
    
    // Create doorway blocker (invisible collision)
    DoorwayBlocker = CreateDefaultSubobject<UBoxComponent>(TEXT("DoorwayBlocker"));
    DoorwayBlocker->SetupAttachment(HingePivot); // CHANGED: Was DoorRoot, now HingePivot
    DoorwayBlocker->SetBoxExtent(FVector(10.0f, 50.0f, 100.0f)); // Thin blocking volume
    DoorwayBlocker->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    DoorwayBlocker->SetCollisionResponseToAllChannels(ECR_Ignore);
    DoorwayBlocker->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
    
    // Initialize state
    bIsOpen = false;
    bIsLocked = false;
    CurrentDoorRotation = 0.0f;
    TargetDoorRotation = 0.0f;
}

void ADoorActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    
    DOREPLIFETIME(ADoorActor, bIsOpen);
    DOREPLIFETIME(ADoorActor, bIsLocked);
}

void ADoorActor::BeginPlay()
{
    Super::BeginPlay();
    
    // Bind interaction
    if (InteractionPoint)
    {
        InteractionPoint->OnInteract.AddDynamic(this, &ADoorActor::OnInteracted);
    }
    
    // Apply starting locked state
    if (bStartsLocked)
    {
        bIsLocked = true;
        OnRep_DoorLocked();
    }
    
    // Set initial blocker state
    if (bIsOpen)
    {
        DoorwayBlocker->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        TargetDoorRotation = OpenAngle;
        CurrentDoorRotation = OpenAngle;
    }
    else
    {
        DoorwayBlocker->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
        TargetDoorRotation = 0.0f;
        CurrentDoorRotation = 0.0f;
    }
}

void ADoorActor::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    // Animate door rotation
    AnimateDoor(DeltaTime);
}

void ADoorActor::Server_ToggleDoor_Implementation(APawn* Interactor)
{
    if (!HasAuthority())
    {
        return;
    }
    
    // Can't open if locked
    if (bIsLocked)
    {
        UE_LOG(LogTemp, Warning, TEXT("Door is locked, cannot open"));
        return;
    }
    
    // Toggle state
    bIsOpen = !bIsOpen;
    
    // Determine which side player is on (for opening direction)
    if (bIsOpen && Interactor)
    {
        FVector DoorForward = GetActorForwardVector();
        FVector ToPlayer = (Interactor->GetActorLocation() - GetActorLocation()).GetSafeNormal();
        float DotProduct = FVector::DotProduct(DoorForward, ToPlayer);
        
        // If player is in front of door (positive dot), open away from them (positive angle)
        // If player is behind door (negative dot), open toward them (negative angle)
        TargetDoorRotation = (DotProduct > 0.0f) ? OpenAngle : -OpenAngle;
        
        UE_LOG(LogTemp, Log, TEXT("Door opening %s player (Dot: %.2f, Angle: %.1f)"), 
               (DotProduct > 0.0f) ? TEXT("away from") : TEXT("toward"), DotProduct, TargetDoorRotation);
    }
    else
    {
        // Closing - always return to 0
        TargetDoorRotation = 0.0f;
    }
    
    // Trigger replication
    OnRep_DoorOpen();
    
    // Setup auto-close timer
    if (bIsOpen && bAutoClose)
    {
        GetWorldTimerManager().SetTimer(AutoCloseTimer, this, &ADoorActor::AutoCloseDoor, AutoCloseDelay, false);
    }
    else
    {
        GetWorldTimerManager().ClearTimer(AutoCloseTimer);
    }
    
    UE_LOG(LogTemp, Log, TEXT("Door toggled: %s"), bIsOpen ? TEXT("OPEN") : TEXT("CLOSED"));
}

void ADoorActor::Server_SetLocked_Implementation(bool bLocked)
{
    if (!HasAuthority())
    {
        return;
    }
    
    bIsLocked = bLocked;
    
    // If locking, close the door first
    if (bIsLocked && bIsOpen)
    {
        bIsOpen = false;
        TargetDoorRotation = 0.0f; // Force close
        OnRep_DoorOpen();
    }
    
    // Trigger replication
    OnRep_DoorLocked();
    
    UE_LOG(LogTemp, Log, TEXT("Door lock set: %s"), bIsLocked ? TEXT("LOCKED") : TEXT("UNLOCKED"));
}

void ADoorActor::Server_SetOpen_Implementation(bool bOpen)
{
    if (!HasAuthority())
    {
        return;
    }
    
    // Can't force open if locked (unless unlocking first)
    if (bIsLocked && bOpen)
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot force open locked door"));
        return;
    }
    
    bIsOpen = bOpen;
    TargetDoorRotation = bOpen ? OpenAngle : 0.0f; // Force specific direction when scripted
    OnRep_DoorOpen();
}

void ADoorActor::OnRep_DoorOpen()
{
    // Update collision blocker
    if (bIsOpen)
    {
        DoorwayBlocker->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        OnDoorOpened.Broadcast(true);
    }
    else
    {
        DoorwayBlocker->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
        OnDoorClosed.Broadcast(false);
    }
    
    // Update interaction text
    if (InteractionPoint)
    {
        InteractionPoint->InteractionName = bIsOpen ? "Close Door" : "Open Door";
    }
}

void ADoorActor::OnRep_DoorLocked()
{
    // Update interaction text
    if (InteractionPoint)
    {
        if (bIsLocked)
        {
            InteractionPoint->InteractionName = "Locked";
            InteractionPoint->isEnabled = false; // Can't interact with locked doors
        }
        else
        {
            InteractionPoint->InteractionName = bIsOpen ? "Close Door" : "Open Door";
            InteractionPoint->isEnabled = true;
        }
    }
    
    // Broadcast event
    OnDoorLockChanged.Broadcast(bIsLocked);
}

void ADoorActor::OnInteracted(APawn* Interactor)
{
    // Call server to toggle door
    Server_ToggleDoor(Interactor);
}

void ADoorActor::AnimateDoor(float DeltaTime)
{
    // Smoothly interpolate to target rotation
    if (!FMath::IsNearlyEqual(CurrentDoorRotation, TargetDoorRotation, 0.1f))
    {
        CurrentDoorRotation = FMath::FInterpTo(CurrentDoorRotation, TargetDoorRotation, DeltaTime, DoorOpenSpeed);
        
        // Apply rotation to hinge pivot (which rotates the door mesh)
        if (HingePivot)
        {
            FRotator NewRotation = FRotator(0.0f, CurrentDoorRotation, 0.0f);
            HingePivot->SetRelativeRotation(NewRotation);
        }
    }
}

void ADoorActor::AutoCloseDoor()
{
    if (bIsOpen && !bIsLocked)
    {
        Server_SetOpen(false);
    }
}