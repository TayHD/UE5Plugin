// CheckInDesk.cpp
#include "CheckInDesk.h"
#include "GuestPawn.h"
#include "RoomActor.h"
#include "HotelGameState.h"
#include "Net/UnrealNetwork.h"
#include "Kismet/GameplayStatics.h"

ACheckInDesk::ACheckInDesk()
{
    PrimaryActorTick.bCanEverTick = false;
    bReplicates = true;
    
    // Create root
    DeskRoot = CreateDefaultSubobject<USceneComponent>(TEXT("DeskRoot"));
    RootComponent = DeskRoot;
    
    // Create desk mesh
    DeskMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DeskMesh"));
    DeskMesh->SetupAttachment(DeskRoot);
    DeskMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    DeskMesh->SetCollisionResponseToAllChannels(ECR_Block);
    
    // Create interaction point
    DeskInteraction = CreateDefaultSubobject<UFC_InteractionPoint>(TEXT("DeskInteraction"));
    DeskInteraction->SetupAttachment(DeskRoot);
    DeskInteraction->InteractionName = "Check In Guest";
    DeskInteraction->InteractionTime = 0.0f; // Instant
    
    // Create guest wait point
    GuestWaitPoint = CreateDefaultSubobject<USceneComponent>(TEXT("GuestWaitPoint"));
    GuestWaitPoint->SetupAttachment(DeskRoot);
    GuestWaitPoint->SetRelativeLocation(FVector(150.0f, 0.0f, 0.0f)); // In front of desk
    
    // Initialize state
    CurrentGuest = nullptr;
    bIsOccupied = false;
    CurrentClerk = nullptr;
}

void ACheckInDesk::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    
    DOREPLIFETIME(ACheckInDesk, CurrentGuest);
    DOREPLIFETIME(ACheckInDesk, GuestQueue);
    DOREPLIFETIME(ACheckInDesk, bIsOccupied);
    DOREPLIFETIME(ACheckInDesk, CurrentClerk);
}

void ACheckInDesk::BeginPlay()
{
    Super::BeginPlay();
    
    // Bind interaction
    if (DeskInteraction)
    {
        DeskInteraction->OnInteract.AddDynamic(this, &ACheckInDesk::OnDeskInteracted);
    }
}

void ACheckInDesk::AddGuestToQueue(AGuestPawn* Guest)
{
    // This should only be called on server (by AI)
    if (!HasAuthority())
    {
        UE_LOG(LogTemp, Error, TEXT("CheckInDesk: AddGuestToQueue called on client!"));
        return;
    }
    
    if (!Guest)
    {
        UE_LOG(LogTemp, Warning, TEXT("CheckInDesk: Tried to add null guest to queue"));
        return;
    }
    
    // Add to queue
    GuestQueue.Add(Guest);
    
    // Update guest state
    Guest->Server_SetBehaviorState(EGuestBehaviorState::WaitingAtDesk);
    
    // Update queue positions
    UpdateQueuePositions();
    
    // If no one is being served, serve this guest immediately
    if (!CurrentGuest && !bIsOccupied)
    {
        ServeNextGuest();
    }
    
    UE_LOG(LogTemp, Log, TEXT("CheckInDesk: Guest %s added to queue (Position: %d)"), 
           *Guest->GetGuestData().GuestName, GuestQueue.Num());
}

void ACheckInDesk::OpenCheckInUI(APawn* Clerk)
{
    // This runs locally on the client that interacted
    // Just opens the UI, doesn't change server state
    
    if (!CurrentGuest)
    {
        UE_LOG(LogTemp, Warning, TEXT("CheckInDesk: No guest to serve"));
        return;
    }
    
    // Broadcast event (Blueprint will open UI)
    OnCheckInStarted.Broadcast(CurrentGuest, Clerk);
    
    UE_LOG(LogTemp, Log, TEXT("CheckInDesk: Check-in started for %s"), 
           *CurrentGuest->GetGuestData().GuestName);
}

void ACheckInDesk::ProcessCheckIn(APawn* Clerk, FGuestCheckInResponse Response)
{
    // This is called by PlayerController's Server RPC
    // Should ONLY run on server
    if (!HasAuthority())
    {
        UE_LOG(LogTemp, Error, TEXT("CheckInDesk: ProcessCheckIn called on client! This should never happen."));
        return;
    }
    
    if (!CurrentGuest)
    {
        UE_LOG(LogTemp, Warning, TEXT("CheckInDesk: No guest to process"));
        return;
    }
    
    if (Response.bApproved)
    {
        // Find room by number
        ARoomActor* AssignedRoom = nullptr;
        TArray<AActor*> FoundRooms;
        UGameplayStatics::GetAllActorsOfClass(GetWorld(), ARoomActor::StaticClass(), FoundRooms);
        
        for (AActor* Actor : FoundRooms)
        {
            ARoomActor* Room = Cast<ARoomActor>(Actor);
            if (Room && Room->RoomNumber == Response.AssignedRoomNumber)
            {
                AssignedRoom = Room;
                break;
            }
        }
        
        if (!AssignedRoom)
        {
            UE_LOG(LogTemp, Error, TEXT("CheckInDesk: Room %d not found!"), Response.AssignedRoomNumber);
            return;
        }
        
        // Assign room to guest
        CurrentGuest->Server_AssignRoom(AssignedRoom);
        CurrentGuest->Server_MarkCheckedIn();
        
        // Add income to hotel
        AHotelGameState* GameState = GetWorld()->GetGameState<AHotelGameState>();
        if (GameState)
        {
            GameState->Server_AddIncome(CurrentGuest->GetGuestData().RoomRate, EIncomeSource::RoomRental);
        }
        
        // Guest goes to room
        CurrentGuest->Server_SetBehaviorState(EGuestBehaviorState::GoingToRoom);
        
        UE_LOG(LogTemp, Log, TEXT("CheckInDesk: Guest %s approved, assigned Room %d"), 
               *CurrentGuest->GetGuestData().GuestName, Response.AssignedRoomNumber);
    }
    else
    {
        // Guest denied, leaves hotel
        CurrentGuest->Server_SetBehaviorState(EGuestBehaviorState::Leaving);
        
        UE_LOG(LogTemp, Log, TEXT("CheckInDesk: Guest %s denied (%s)"), 
               *CurrentGuest->GetGuestData().GuestName, *Response.RejectionReason);
    }
    
    // Broadcast completion
    OnCheckInCompleted.Broadcast(CurrentGuest, Response.bApproved, Response.AssignedRoomNumber);
    
    // Clear current guest
    CurrentGuest = nullptr;
    bIsOccupied = false;
    CurrentClerk = nullptr;
    
    // Serve next guest in queue
    ServeNextGuest();
}

void ACheckInDesk::ServeNextGuest()
{
    // Server only
    if (!HasAuthority())
    {
        return;
    }
    
    // Check if there are guests waiting
    if (GuestQueue.Num() == 0)
    {
        CurrentGuest = nullptr;
        return;
    }
    
    // Get first guest in queue
    CurrentGuest = GuestQueue[0];
    GuestQueue.RemoveAt(0);
    
    // Update queue positions
    UpdateQueuePositions();
    
    // Update interaction prompt
    if (DeskInteraction)
    {
        DeskInteraction->InteractionName = FString::Printf(TEXT("Check In %s"), 
                                                           *CurrentGuest->GetGuestData().GuestName);
    }
    
    UE_LOG(LogTemp, Log, TEXT("CheckInDesk: Now serving %s"), 
           *CurrentGuest->GetGuestData().GuestName);
}

ARoomActor* ACheckInDesk::FindAvailableRoom()
{
    TArray<AActor*> FoundRooms;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ARoomActor::StaticClass(), FoundRooms);
    
    for (AActor* Actor : FoundRooms)
    {
        ARoomActor* Room = Cast<ARoomActor>(Actor);
        if (Room && Room->IsClean() && !Room->IsOccupied())
        {
            return Room;
        }
    }
    
    return nullptr; // No rooms available
}

void ACheckInDesk::OnDeskInteracted(APawn* Interactor)
{
    // Called locally when player presses E
    // Just opens UI locally, doesn't call server
    OpenCheckInUI(Interactor);
}

void ACheckInDesk::UpdateQueuePositions()
{
    // Position guests in a line
    for (int32 i = 0; i < GuestQueue.Num(); i++)
    {
        if (GuestQueue[i])
        {
            FVector QueuePosition = GuestWaitPoint->GetComponentLocation();
            QueuePosition += GetActorForwardVector() * (i * QueueSpacing);
            
            // TODO: Tell guest AI to move to this position
            // (Will implement when we add AI movement)
        }
    }
}