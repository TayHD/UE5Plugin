// RoomActor.h
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RoomActor.generated.h"

// Forward declarations
class ADoorActor;
class AGuestPawn;

UENUM(BlueprintType)
enum class ERoomState : uint8
{
    Dirty       UMETA(DisplayName = "Dirty"),
    Clean       UMETA(DisplayName = "Clean"),
    Occupied    UMETA(DisplayName = "Occupied")
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
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* RoomRoot;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UBoxComponent* RoomBounds; // Defines room volume
    
    // ===== ROOM PROPERTIES =====
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Room")
    int32 RoomNumber = 1;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Room")
    float RoomRate = 100.0f; // Cost per night
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Room")
    ADoorActor* DoorActor; // Reference to this room's door
    
    // ===== STATE (Replicated) =====
    
    UPROPERTY(ReplicatedUsing=OnRep_RoomState, BlueprintReadOnly, Category = "Room State")
    ERoomState CurrentState;
    
    UPROPERTY(Replicated, BlueprintReadOnly, Category = "Room State")
    AGuestPawn* CurrentGuest; // Who is staying in this room
    
    UPROPERTY(Replicated, BlueprintReadOnly, Category = "Room State")
    bool bIsOccupied;
    
    UPROPERTY(Replicated, BlueprintReadOnly, Category = "Room State")
    bool bIsClean;
    
    // ===== FUNCTIONS =====
    
    // Room management (called by server)
    UFUNCTION(BlueprintCallable, Category = "Room")
    void Server_AssignGuest(AGuestPawn* Guest);
    
    UFUNCTION(BlueprintCallable, Category = "Room")
    void Server_CheckOutGuest();
    
    UFUNCTION(BlueprintCallable, Category = "Room")
    void Server_CleanRoom();
    
    UFUNCTION(BlueprintCallable, Category = "Room")
    void Server_DirtyRoom();
    
    // Query functions
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Room")
    bool IsClean() const { return bIsClean; }
    
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Room")
    bool IsOccupied() const { return bIsOccupied; }
    
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Room")
    AGuestPawn* GetCurrentGuest() const { return CurrentGuest; }
    
    // ===== REPLICATION CALLBACKS =====
    
    UFUNCTION()
    void OnRep_RoomState();
};