// Copyright Frinky 2022

#include "FrinkysComponents/General/PhysicsGrabbable.h"
#include "FrinkysComponents/Movement/MultiCharacter.h"
#include "Net/UnrealNetwork.h"
#include "Components/StaticMeshComponent.h"

// Constructor - Sets up the actor with a mesh and interaction point
APhysicsGrabbable::APhysicsGrabbable()
{
	// Enable ticking if needed (currently disabled for performance)
	PrimaryActorTick.bCanEverTick = false;

	// Enable replication - this actor needs to sync across network
	bReplicates = true;

	// Create the static mesh component (the visual object)
	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	RootComponent = MeshComponent;

	// Setup physics properties
	MeshComponent->SetSimulatePhysics(true); // Object has physics when not held
	MeshComponent->SetEnableGravity(true); // Object falls when not held
	MeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics); // Can be hit and blocks

	// Create the interaction point (allows players to interact with this)
	InteractionPoint = CreateDefaultSubobject<UFC_InteractionPoint>(TEXT("InteractionPoint"));
	InteractionPoint->SetupAttachment(RootComponent);
	InteractionPoint->InteractionName = "Grab Object";
	InteractionPoint->InteractionTime = 0.0f; // Instant interaction (no hold time needed)

	// Create optional hold location point (where player's hand attaches)
	// Can be repositioned in Blueprint to customize grip location
	HoldLocationPoint = CreateDefaultSubobject<USceneComponent>(TEXT("HoldLocationPoint"));
	HoldLocationPoint->SetupAttachment(RootComponent);

	// Set default collision to block all but allow overlap for interaction
	MeshComponent->SetCollisionResponseToAllChannels(ECR_Block);
}

// Called when the game starts or when spawned
void APhysicsGrabbable::BeginPlay()
{
	Super::BeginPlay();
	
	// Bind to the interaction point's OnInteract event
	// When a player presses interact on this object, OnInteracted will be called
	if (InteractionPoint)
	{
		InteractionPoint->OnInteract.AddDynamic(this, &APhysicsGrabbable::OnInteracted);
	}
}

// Setup which properties replicate over network
void APhysicsGrabbable::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// Replicate held state to all clients
	DOREPLIFETIME(APhysicsGrabbable, bIsBeingHeld);
	// Replicate who is holding this object
	DOREPLIFETIME(APhysicsGrabbable, CurrentHolder);
}

// Called when a player interacts with this object
void APhysicsGrabbable::OnInteracted(APawn* Interactor)
{
	// Check if object can be grabbed
	if (!bCanBeGrabbed)
	{
		return;
	}

	// Check if already being held
	if (bIsBeingHeld)
	{
		return;
	}

	// Check if interactor is valid
	if (!IsValid(Interactor))
	{
		return;
	}

	// Cast to MultiCharacter and call Server_GrabObject
	AMultiCharacter* Character = Cast<AMultiCharacter>(Interactor);
	if (Character)
	{
		Character->Server_GrabObject(this);
	}
}

// Called when the held state changes (via replication)
void APhysicsGrabbable::OnRep_IsBeingHeld()
{
	if (bIsBeingHeld)
	{
		// Object is now being held
		// Disable physics simulation so it doesn't fall
		if (MeshComponent)
		{
			MeshComponent->SetSimulatePhysics(false);
			MeshComponent->SetEnableGravity(false);
			// Optionally disable collision while held to prevent blocking
			// MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		}

		// Broadcast event for Blueprint customization
		OnGrabbed.Broadcast(CurrentHolder);
	}
	else
	{
		// Object is no longer being held
		// Re-enable physics simulation
		if (MeshComponent)
		{
			MeshComponent->SetSimulatePhysics(true);
			MeshComponent->SetEnableGravity(true);
			MeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		}

		// Broadcast event for Blueprint customization
		OnObjectReleased.Broadcast(CurrentHolder);
	}
}

// Set whether this object is being held (called by character)
void APhysicsGrabbable::SetHeldState(bool bHeld, APawn* Holder)
{
	// Only the server should set this state
	if (!HasAuthority())
	{
		return;
	}

	bIsBeingHeld = bHeld;
	CurrentHolder = Holder;

	// Manually call the replication callback on server
	// (OnRep functions don't fire on server, only on clients)
	OnRep_IsBeingHeld();
}