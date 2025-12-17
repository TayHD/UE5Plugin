// RoomActor.cpp
#include "RoomActor.h"
#include "DoorActor.h"
#include "GuestPawn.h"
#include "Components/BoxComponent.h"
#include "Net/UnrealNetwork.h"

ARoomActor::ARoomActor()
{
    PrimaryActorTick.bCanEverTick = false;
    bReplicates = true;
    
    // Create root
    RoomRoot = CreateDefaultSubobject<USceneComponent>(TEXT("RoomRoot"));
    RootComponent = RoomRoot;
    
    // Create room bounds
    RoomBounds = CreateDefaultSubobject<UBoxComponent>(TEXT("RoomBounds"));
    RoomBounds->SetupAttachment(RoomRoot);
    RoomBounds->SetBoxExtent(FVector(300.0f, 300.0f, 150.0f)); // Default room size
    RoomBounds->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    RoomBounds->SetCollisionResponseToAllChannels(ECR_Overlap);
    
    // Initialize state
    CurrentState = ERoomState::Clean;
    CurrentGuest = nullptr;
    bIsOccupied = false;
    bIsClean = true;
}

void ARoomActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    
    DOREPLIFETIME(ARoomActor, CurrentState);
    DOREPLIFETIME(ARoomActor, CurrentGuest);
    DOREPLIFETIME(ARoomActor, bIsOccupied);
    DOREPLIFETIME(ARoomActor, bIsClean);
}

void ARoomActor::BeginPlay()
{
    Super::BeginPlay();
    
    // Only set initial door state on server
    if (HasAuthority() && DoorActor)
    {
        DoorActor->SetLocked(false); // Start unlocked
        UE_LOG(LogTemp, Log, TEXT("Room %d: Door unlocked"), RoomNumber);
    }
}

void ARoomActor::Server_AssignGuest(AGuestPawn* Guest)
{
    // Should only be called on server
    if (!HasAuthority())
    {
        UE_LOG(LogTemp, Error, TEXT("Room: Server_AssignGuest called on client!"));
        return;
    }
    
    if (!Guest)
    {
        UE_LOG(LogTemp, Warning, TEXT("Room %d: Tried to assign null guest"), RoomNumber);
        return;
    }
    
    // Assign guest
    CurrentGuest = Guest;
    bIsOccupied = true;
    CurrentState = ERoomState::Occupied;
    
    // Lock door
    if (DoorActor)
    {
        DoorActor->SetLocked(true);
        UE_LOG(LogTemp, Log, TEXT("Room %d: Door locked"), RoomNumber);
    }
    
    UE_LOG(LogTemp, Log, TEXT("Room %d: Guest assigned (%s)"), RoomNumber, *Guest->GetName());
}

void ARoomActor::Server_CheckOutGuest()
{
    // Should only be called on server
    if (!HasAuthority())
    {
        UE_LOG(LogTemp, Error, TEXT("Room: Server_CheckOutGuest called on client!"));
        return;
    }
    
    if (!CurrentGuest)
    {
        UE_LOG(LogTemp, Warning, TEXT("Room %d: No guest to check out"), RoomNumber);
        return;
    }
    
    UE_LOG(LogTemp, Log, TEXT("Room %d: Guest %s checked out"), RoomNumber, *CurrentGuest->GetName());
    
    // Clear guest
    CurrentGuest = nullptr;
    bIsOccupied = false;
    CurrentState = ERoomState::Dirty;
    
    // Unlock door
    if (DoorActor)
    {
        DoorActor->SetLocked(false);
        UE_LOG(LogTemp, Log, TEXT("Room %d: Door unlocked"), RoomNumber);
    }
}

void ARoomActor::Server_CleanRoom()
{
    // Should only be called on server
    if (!HasAuthority())
    {
        UE_LOG(LogTemp, Error, TEXT("Room: Server_CleanRoom called on client!"));
        return;
    }
    
    if (bIsOccupied)
    {
        UE_LOG(LogTemp, Warning, TEXT("Room %d: Cannot clean - room is occupied"), RoomNumber);
        return;
    }
    
    bIsClean = true;
    CurrentState = ERoomState::Clean;
    
    UE_LOG(LogTemp, Log, TEXT("Room %d: Cleaned"), RoomNumber);
}

void ARoomActor::Server_DirtyRoom()
{
    // Should only be called on server
    if (!HasAuthority())
    {
        UE_LOG(LogTemp, Error, TEXT("Room: Server_DirtyRoom called on client!"));
        return;
    }
    
    bIsClean = false;
    CurrentState = ERoomState::Dirty;
    
    UE_LOG(LogTemp, Log, TEXT("Room %d: Dirtied"), RoomNumber);
}

void ARoomActor::OnRep_RoomState()
{
    // Update visuals on clients based on replicated state
    UE_LOG(LogTemp, Log, TEXT("Room %d: State replicated - %d"), RoomNumber, (int32)CurrentState);
}