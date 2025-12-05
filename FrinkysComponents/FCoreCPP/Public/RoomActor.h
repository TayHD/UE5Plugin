// RoomActor.h
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/BoxComponent.h"
#include "RoomActor.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRoomStateChanged, class ARoomActor*, Room);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnGuestAssigned, class ARoomActor*, Room, AActor*, Guest);

UENUM(BlueprintType)
enum class ERoomState : uint8
{
    Vacant_Clean    UMETA(DisplayName = "Vacant - Clean"),
    Vacant_Dirty    UMETA(DisplayName = "Vacant - Dirty"),
    Occupied        UMETA(DisplayName = "Occupied"),
    Maintenance     UMETA(DisplayName = "Maintenance")
};

UENUM(BlueprintType)
enum class ERoomIssue : uint8
{
    None            UMETA(DisplayName = "None"),
    DirtyBed        UMETA(DisplayName = "Dirty Bed"),
    DirtyBathroom   UMETA(DisplayName = "Dirty Bathroom"),
    EmptyTowels     UMETA(DisplayName = "Empty Towels"),
    EmptyToiletries UMETA(DisplayName = "Empty Toiletries"),
    BrokenAC        UMETA(DisplayName = "Broken AC"),
    BrokenTV        UMETA(DisplayName = "Broken TV"),
    BrokenLock      UMETA(DisplayName = "Broken Lock"),
    StrangeMess     UMETA(DisplayName = "Strange Mess") // Serial killer aftermath
};

UENUM(BlueprintType)
enum class ECleaningTask : uint8
{
    MakeBed         UMETA(DisplayName = "Make Bed"),
    CleanBathroom   UMETA(DisplayName = "Clean Bathroom"),
    EmptyTrash      UMETA(DisplayName = "Empty Trash"),
    RestockTowels   UMETA(DisplayName = "Restock Towels"),
    RestockToiletries UMETA(DisplayName = "Restock Toiletries")
};

UCLASS()
class FCORECPP_API ARoomActor : public AActor
{
    GENERATED_BODY()
    
public:
    ARoomActor();
    
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
    virtual void BeginPlay() override;
    
    // ===== COMPONENTS =====
    
    // Volume that defines room bounds
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UBoxComponent* RoomVolume;
    
    // Root scene component
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* RoomRoot;
    
    // ===== REFERENCES (Set in Editor) - WILL ADD BACK WHEN WE BUILD THOSE ACTORS =====
    
    // TODO: Uncomment when DoorActor is created
     UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Room References")
     ADoorActor* RoomDoor;
    
    // TODO: Uncomment when ACUnit is created
    // UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Room References")
    // AACUnit* ACUnit;
    
    // TODO: Uncomment when RoomLight is created
    // UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Room References")
    // TArray<ARoomLight*> RoomLights;
    
    // Cleaning interaction points (placed as child components in editor)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Room References")
    TArray<USceneComponent*> CleaningPoints;
    
    // ===== ROOM IDENTITY =====
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Room Settings", meta = (ClampMin = "1", ClampMax = "50"))
    int32 RoomNumber = 1;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Room Settings")
    float RoomRate = 75.0f; // Base rental price
    
    // ===== STATE =====
    
    UPROPERTY(ReplicatedUsing=OnRep_RoomState, BlueprintReadOnly, Category = "Room State")
    ERoomState CurrentState;
    
    // Using AActor for now, will change to AGuestPawn when we create it
    UPROPERTY(Replicated, BlueprintReadOnly, Category = "Room State")
    AActor* CurrentGuest;
    
    UPROPERTY(ReplicatedUsing=OnRep_Cleanliness, BlueprintReadOnly, Category = "Room State")
    float CleanlinessLevel; // 0-100
    
    UPROPERTY(Replicated, BlueprintReadOnly, Category = "Room State")
    TArray<ERoomIssue> ActiveIssues;
    
    UPROPERTY(Replicated, BlueprintReadOnly, Category = "Room State")
    bool bIsDoorLocked;
    
    // ===== CLEANING TRACKING =====
    
    UPROPERTY(BlueprintReadOnly, Category = "Room State")
    TSet<ECleaningTask> CompletedTasks; // Tasks done this cleaning session
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Room Settings")
    TArray<ECleaningTask> RequiredCleaningTasks; // What needs to be done to mark room clean
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Room Settings")
    float CleanlinessPerTask = 33.33f; // How much each task increases cleanliness
    
    // ===== FUNCTIONS =====
    
    // Assign a guest to this room (using AActor for now)
    UFUNCTION(BlueprintCallable, Server, Reliable, Category = "Room Management")
    void Server_AssignGuest(AActor* Guest);
    
    // Guest checks out, room becomes available
    UFUNCTION(BlueprintCallable, Server, Reliable, Category = "Room Management")
    void Server_CheckOutGuest();
    
    // Complete a cleaning task
    UFUNCTION(BlueprintCallable, Server, Reliable, Category = "Room Management")
    void Server_CompleteCleaningTask(ECleaningTask Task);
    
    // Add an issue to the room
    UFUNCTION(BlueprintCallable, Server, Reliable, Category = "Room Management")
    void Server_AddIssue(ERoomIssue Issue);
    
    // Resolve an issue in the room
    UFUNCTION(BlueprintCallable, Server, Reliable, Category = "Room Management")
    void Server_ResolveIssue(ERoomIssue Issue);
    
    // Force set room state (for testing/admin)
    UFUNCTION(BlueprintCallable, Server, Reliable, Category = "Room Management")
    void Server_SetRoomState(ERoomState NewState);
    
    // ===== DOOR CONTROL =====
    
    UFUNCTION(BlueprintCallable, Category = "Room Management")
    void LockDoor();
    
    UFUNCTION(BlueprintCallable, Category = "Room Management")
    void UnlockDoor();
    
    // ===== QUERIES =====
    
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Room Management")
    bool IsOccupied() const { return CurrentState == ERoomState::Occupied; }
    
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Room Management")
    bool IsClean() const { return CurrentState == ERoomState::Vacant_Clean; }
    
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Room Management")
    bool NeedsCleaning() const { return CurrentState == ERoomState::Vacant_Dirty; }
    
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Room Management")
    bool HasIssue(ERoomIssue Issue) const { return ActiveIssues.Contains(Issue); }
    
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Room Management")
    float GetCleanlinessPercent() const { return CleanlinessLevel; }
    
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Room Management")
    int32 GetRemainingCleaningTasks() const;
    
    // ===== REPLICATION CALLBACKS =====
    
    UFUNCTION()
    void OnRep_RoomState();
    
    UFUNCTION()
    void OnRep_Cleanliness();
    
    // ===== BLUEPRINT EVENTS =====
    
    UPROPERTY(BlueprintAssignable, Category = "Room Events")
    FOnRoomStateChanged OnRoomStateChanged;
    
    UPROPERTY(BlueprintAssignable, Category = "Room Events")
    FOnGuestAssigned OnGuestAssigned;
    
private:
    // Helper to check if room is fully cleaned
    bool IsFullyCleaned() const;
    
    // Helper to update room state based on cleanliness
    void UpdateRoomStateFromCleanliness();
};