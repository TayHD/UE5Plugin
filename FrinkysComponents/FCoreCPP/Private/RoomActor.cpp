// RoomActor.cpp
#include "RoomActor.h"
#include "Net/UnrealNetwork.h"

ARoomActor::ARoomActor()
{
    PrimaryActorTick.bCanEverTick = false;
    bReplicates = true;
    
    // Create root
    RoomRoot = CreateDefaultSubobject<USceneComponent>(TEXT("RoomRoot"));
    RootComponent = RoomRoot;
    
    // Create room volume (for visualization in editor)
    RoomVolume = CreateDefaultSubobject<UBoxComponent>(TEXT("RoomVolume"));
    RoomVolume->SetupAttachment(RoomRoot);
    RoomVolume->SetBoxExtent(FVector(300.0f, 300.0f, 150.0f)); // Default room size
    RoomVolume->SetCollisionEnabled(ECollisionEnabled::NoCollision); // Just for visualization
    
    // Initialize state
    CurrentState = ERoomState::Vacant_Clean;
    CleanlinessLevel = 100.0f;
    bIsDoorLocked = false;
    CurrentGuest = nullptr;
    
    // Default required cleaning tasks
    RequiredCleaningTasks.Add(ECleaningTask::MakeBed);
    RequiredCleaningTasks.Add(ECleaningTask::CleanBathroom);
    RequiredCleaningTasks.Add(ECleaningTask::EmptyTrash);
}

void ARoomActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    
    DOREPLIFETIME(ARoomActor, CurrentState);
    DOREPLIFETIME(ARoomActor, CurrentGuest);
    DOREPLIFETIME(ARoomActor, CleanlinessLevel);
    DOREPLIFETIME(ARoomActor, ActiveIssues);
    DOREPLIFETIME(ARoomActor, bIsDoorLocked);
}

void ARoomActor::BeginPlay()
{
    Super::BeginPlay();
    
    // Set initial door state
    if (CurrentState == ERoomState::Occupied)
    {
        LockDoor();
    }
    else
    {
        UnlockDoor();
    }
}

void ARoomActor::Server_AssignGuest_Implementation(AActor* Guest)
{
    if (!HasAuthority())
    {
        return;
    }
    
    // Validate
    if (!Guest)
    {
        UE_LOG(LogTemp, Warning, TEXT("Room %d: Tried to assign null guest"), RoomNumber);
        return;
    }
    
    if (CurrentState != ERoomState::Vacant_Clean)
    {
        UE_LOG(LogTemp, Warning, TEXT("Room %d: Cannot assign guest, room not clean (State: %d)"), 
               RoomNumber, (int32)CurrentState);
        return;
    }
    
    // Assign guest
    CurrentGuest = Guest;
    CurrentState = ERoomState::Occupied;
    
    // Lock door
    LockDoor();
    
    // Trigger replication
    OnRep_RoomState();
    
    // Broadcast events
    OnGuestAssigned.Broadcast(this, Guest);
    
    UE_LOG(LogTemp, Log, TEXT("Room %d: Guest assigned (%s)"), 
           RoomNumber, *Guest->GetName());
}

void ARoomActor::Server_CheckOutGuest_Implementation()
{
    if (!HasAuthority())
    {
        return;
    }
    
    if (!CurrentGuest)
    {
        UE_LOG(LogTemp, Warning, TEXT("Room %d: No guest to check out"), RoomNumber);
        return;
    }
    
    // Clear guest
    AActor* PreviousGuest = CurrentGuest;
    CurrentGuest = nullptr;
    
    // Room is now dirty and needs cleaning
    CurrentState = ERoomState::Vacant_Dirty;
    CleanlinessLevel = 0.0f;
    CompletedTasks.Empty();
    
    // Unlock door so players can enter to clean
    UnlockDoor();
    
    // Trigger replication
    OnRep_RoomState();
    OnRep_Cleanliness();
    
    UE_LOG(LogTemp, Log, TEXT("Room %d: Guest checked out, room now dirty"), RoomNumber);
}

void ARoomActor::Server_CompleteCleaningTask_Implementation(ECleaningTask Task)
{
    if (!HasAuthority())
    {
        return;
    }
    
    // Check if task is required
    if (!RequiredCleaningTasks.Contains(Task))
    {
        UE_LOG(LogTemp, Warning, TEXT("Room %d: Task not required for this room"), RoomNumber);
        return;
    }
    
    // Check if already completed
    if (CompletedTasks.Contains(Task))
    {
        UE_LOG(LogTemp, Warning, TEXT("Room %d: Task already completed"), RoomNumber);
        return;
    }
    
    // Mark task as completed
    CompletedTasks.Add(Task);
    
    // Increase cleanliness
    CleanlinessLevel = FMath::Clamp(CleanlinessLevel + CleanlinessPerTask, 0.0f, 100.0f);
    
    // Check if all tasks are done
    if (IsFullyCleaned())
    {
        CurrentState = ERoomState::Vacant_Clean;
        CleanlinessLevel = 100.0f;
        UE_LOG(LogTemp, Log, TEXT("Room %d: Cleaning complete, room ready for guests"), RoomNumber);
    }
    
    // Trigger replication
    OnRep_Cleanliness();
    OnRep_RoomState();
    
    UE_LOG(LogTemp, Log, TEXT("Room %d: Completed task (Cleanliness: %.1f%%)"), 
           RoomNumber, CleanlinessLevel);
}

void ARoomActor::Server_AddIssue_Implementation(ERoomIssue Issue)
{
    if (!HasAuthority())
    {
        return;
    }
    
    if (Issue == ERoomIssue::None)
    {
        return;
    }
    
    // Don't add duplicates
    if (ActiveIssues.Contains(Issue))
    {
        return;
    }
    
    ActiveIssues.Add(Issue);
    
    // If room was clean, mark as maintenance
    if (CurrentState == ERoomState::Vacant_Clean)
    {
        CurrentState = ERoomState::Maintenance;
        OnRep_RoomState();
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Room %d: Issue added - %d"), RoomNumber, (int32)Issue);
}

void ARoomActor::Server_ResolveIssue_Implementation(ERoomIssue Issue)
{
    if (!HasAuthority())
    {
        return;
    }
    
    // Remove issue
    ActiveIssues.Remove(Issue);
    
    // If no more issues and room is in maintenance, restore previous state
    if (ActiveIssues.Num() == 0 && CurrentState == ERoomState::Maintenance)
    {
        if (CleanlinessLevel >= 100.0f)
        {
            CurrentState = ERoomState::Vacant_Clean;
        }
        else
        {
            CurrentState = ERoomState::Vacant_Dirty;
        }
        OnRep_RoomState();
    }
    
    UE_LOG(LogTemp, Log, TEXT("Room %d: Issue resolved - %d"), RoomNumber, (int32)Issue);
}

void ARoomActor::Server_SetRoomState_Implementation(ERoomState NewState)
{
    if (!HasAuthority())
    {
        return;
    }
    
    CurrentState = NewState;
    OnRep_RoomState();
    
    UE_LOG(LogTemp, Log, TEXT("Room %d: State set to %d"), RoomNumber, (int32)NewState);
}

void ARoomActor::LockDoor()
{
    bIsDoorLocked = true;
    // TODO: When DoorActor exists, call RoomDoor->SetLocked(true);
    UE_LOG(LogTemp, Log, TEXT("Room %d: Door locked"), RoomNumber);
}

void ARoomActor::UnlockDoor()
{
    bIsDoorLocked = false;
    // TODO: When DoorActor exists, call RoomDoor->SetLocked(false);
    UE_LOG(LogTemp, Log, TEXT("Room %d: Door unlocked"), RoomNumber);
}

int32 ARoomActor::GetRemainingCleaningTasks() const
{
    return RequiredCleaningTasks.Num() - CompletedTasks.Num();
}

void ARoomActor::OnRep_RoomState()
{
    // Visual feedback based on state (can be expanded in Blueprint)
    OnRoomStateChanged.Broadcast(this);
    
    // Update door lock state
    if (CurrentState == ERoomState::Occupied)
    {
        LockDoor();
    }
    else
    {
        UnlockDoor();
    }
}

void ARoomActor::OnRep_Cleanliness()
{
    // Visual feedback for cleanliness (can be expanded in Blueprint)
    // Could show dust particles, dirty textures, etc.
}

bool ARoomActor::IsFullyCleaned() const
{
    // Check if all required tasks are completed
    for (ECleaningTask Task : RequiredCleaningTasks)
    {
        if (!CompletedTasks.Contains(Task))
        {
            return false;
        }
    }
    return true;
}

void ARoomActor::UpdateRoomStateFromCleanliness()
{
    if (CleanlinessLevel >= 100.0f && CurrentState == ERoomState::Vacant_Dirty)
    {
        CurrentState = ERoomState::Vacant_Clean;
        OnRep_RoomState();
    }
}