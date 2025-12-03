// Copyright 2021 Glitch Games.


#include "MultiCharacter.h"
#include "MyCharacterMovementComponent.h"
#include "FrinkysComponents/General/PhysicsGrabbable.h"
#include "Net/UnrealNetwork.h"
#include "Components/SceneComponent.h"
#include "GameFramework/PlayerController.h"
#include "Camera/PlayerCameraManager.h"

// Sets default values
AMultiCharacter::AMultiCharacter(const class FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer.SetDefaultSubobjectClass<UMyCharacterMovementComponent>(ACharacter::CharacterMovementComponentName))
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Create the HoldPoint component
	// This is an invisible point in front of the camera where objects attach when grabbed
	HoldPoint = CreateDefaultSubobject<USceneComponent>(TEXT("HoldPoint"));
	HoldPoint->SetupAttachment(RootComponent); // Attach to character root
	HoldPoint->SetRelativeLocation(FVector(150.0f, 0.0f, 0.0f)); // 150 units in front

	// Initialize held object to null
	HeldObject = nullptr;
}

// Called when the game starts or when spawned
void AMultiCharacter::BeginPlay()
{
	Super::BeginPlay();
	
}

// Setup which variables replicate over network
void AMultiCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// Replicate HeldObject to all clients
	DOREPLIFETIME(AMultiCharacter, HeldObject);
}

// Called every frame
void AMultiCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Update hold point position if we're holding an object
	if (IsHoldingObject() && IsLocallyControlled())
	{
		UpdateHoldPoint(DeltaTime);
	}
}

// Called to bind functionality to input
void AMultiCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

void AMultiCharacter::PhysNetCustom_Implementation(float DeltaTime, int32 Iterations)
{
	//This is made to be overridable in a c++ class. 
}

UMyCharacterMovementComponent* AMultiCharacter::GetMultiplayerMovementComponent() const
{
	UMyCharacterMovementComponent* MyCharacterMovementComponent = static_cast<UMyCharacterMovementComponent*>(GetCharacterMovement());
	return MyCharacterMovementComponent; 
}

void AMultiCharacter::OnMovementUpdatedCustom_Implementation(float DeltaSeconds, const FVector& OldLocation,
	const FVector& OldVelocity)
{
	//This is made to be overridable in a c++ class.
}

// Physics Grabbing Implementation

// Server RPC: Grab an object
void AMultiCharacter::Server_GrabObject_Implementation(AActor* ObjectToGrab)
{
	// Validate input
	if (!IsValid(ObjectToGrab))
	{
		return;
	}

	// Check if already holding something
	if (IsHoldingObject())
	{
		return;
	}

	// Get the root component
	USceneComponent* ObjectRoot = ObjectToGrab->GetRootComponent();
	if (!ObjectRoot)
	{
		return;
	}

	// Set held object reference
	HeldObject = ObjectToGrab;

	// Disable physics on the object
	UPrimitiveComponent* PrimComp = Cast<UPrimitiveComponent>(ObjectRoot);
	if (PrimComp)
	{
		PrimComp->SetSimulatePhysics(false);
		PrimComp->SetEnableGravity(false);
	}

	// If it's a PhysicsGrabbable, notify it
	APhysicsGrabbable* Grabbable = Cast<APhysicsGrabbable>(ObjectToGrab);
	if (Grabbable)
	{
		Grabbable->SetHeldState(true, this);
	}

	// Attach object to hold point
	// The HoldLocationPoint in the object defines the grip point offset
	ObjectToGrab->AttachToComponent(HoldPoint, FAttachmentTransformRules::KeepWorldTransform);

	// If there's a HoldLocationPoint, adjust the attachment so that point aligns with HoldPoint
	if (Grabbable && Grabbable->GetHoldLocationPoint())
	{
		USceneComponent* HoldLoc = Grabbable->GetHoldLocationPoint();
		
		// Calculate offset from object root to hold location point
		FVector Offset = HoldLoc->GetComponentLocation() - ObjectToGrab->GetActorLocation();
		
		// Move the object so the HoldLocationPoint aligns with HoldPoint
		FVector NewLocation = HoldPoint->GetComponentLocation() - Offset;
		ObjectToGrab->SetActorLocation(NewLocation);
	}

	// Call replication callback manually on server
	OnRep_HeldObject();
}

// Server RPC: Release the held object
void AMultiCharacter::Server_ReleaseObject_Implementation(bool bThrow)
{
	// Check if holding something
	if (!IsHoldingObject())
	{
		return;
	}

	AActor* ObjectToRelease = HeldObject;

	// If it's a PhysicsGrabbable, notify it before releasing
	APhysicsGrabbable* Grabbable = Cast<APhysicsGrabbable>(ObjectToRelease);
	if (Grabbable)
	{
		Grabbable->SetHeldState(false, nullptr);
	}

	// Get current transform before detaching
	FVector ReleaseLocation = ObjectToRelease->GetActorLocation();
	FRotator ReleaseRotation = ObjectToRelease->GetActorRotation();
	
	// Calculate velocity for throw or drop
	FVector ReleaseVelocity = bThrow ? CalculateThrowVelocity() : FVector::ZeroVector;

	// Clear held object reference
	HeldObject = nullptr;

	// Detach from hold point
	ObjectToRelease->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);

	// Use multicast to ensure all clients place object at same location
	Multicast_PlaceObject(ObjectToRelease, ReleaseLocation, ReleaseRotation, ReleaseVelocity);

	// Call replication callback manually on server
	OnRep_HeldObject();
}

// Multicast RPC: Synchronize object placement across all clients
void AMultiCharacter::Multicast_PlaceObject_Implementation(AActor* PlacedObject, FVector Location, FRotator Rotation, FVector Velocity)
{
	if (!IsValid(PlacedObject))
	{
		return;
	}

	// Set exact position and rotation on all machines
	PlacedObject->SetActorLocationAndRotation(Location, Rotation, false, nullptr, ETeleportType::TeleportPhysics);

	// Re-enable physics
	UPrimitiveComponent* PrimComp = Cast<UPrimitiveComponent>(PlacedObject->GetRootComponent());
	if (PrimComp)
	{
		PrimComp->SetSimulatePhysics(true);
		PrimComp->SetEnableGravity(true);

		// Set velocity (works for both throw and drop)
		PrimComp->SetPhysicsLinearVelocity(Velocity);
		
		// Clear angular velocity to prevent unwanted spinning
		PrimComp->SetPhysicsAngularVelocityInDegrees(FVector::ZeroVector);
	}
}

// Calculate throw velocity based on camera direction and character velocity
FVector AMultiCharacter::CalculateThrowVelocity() const
{
	FVector ThrowVelocity;

	// Get camera forward direction
	APlayerController* PC = Cast<APlayerController>(GetController());
	if (PC && PC->PlayerCameraManager)
	{
		FVector CameraForward = PC->PlayerCameraManager->GetActorForwardVector();
		ThrowVelocity = CameraForward * ThrowStrength;

		// Add character velocity to throw velocity (so throwing while running adds momentum)
		ThrowVelocity += GetVelocity();
	}
	else
	{
		// Fallback: use control rotation
		ThrowVelocity = GetControlRotation().Vector() * ThrowStrength;
		ThrowVelocity += GetVelocity();
	}

	return ThrowVelocity;
}

// Called when HeldObject replicates to clients
void AMultiCharacter::OnRep_HeldObject()
{
	// If we're now holding an object
	if (IsHoldingObject())
	{
		// Get the object's root component
		USceneComponent* ObjectRoot = HeldObject->GetRootComponent();
		
		// Ensure it's attached (may already be attached on server)
		if (ObjectRoot && ObjectRoot->GetAttachParent() != HoldPoint)
		{
			HeldObject->AttachToComponent(HoldPoint, FAttachmentTransformRules::KeepWorldTransform);
			
			// If there's a HoldLocationPoint, adjust position
			APhysicsGrabbable* Grabbable = Cast<APhysicsGrabbable>(HeldObject);
			if (Grabbable && Grabbable->GetHoldLocationPoint())
			{
				USceneComponent* HoldLoc = Grabbable->GetHoldLocationPoint();
				FVector Offset = HoldLoc->GetComponentLocation() - HeldObject->GetActorLocation();
				FVector NewLocation = HoldPoint->GetComponentLocation() - Offset;
				HeldObject->SetActorLocation(NewLocation);
			}
		}

		// Disable physics on client
		UPrimitiveComponent* PrimComp = Cast<UPrimitiveComponent>(HeldObject->GetRootComponent());
		if (PrimComp)
		{
			PrimComp->SetSimulatePhysics(false);
			PrimComp->SetEnableGravity(false);
		}
	}
	// If we released an object, physics is already re-enabled by server
}

// Update hold point to follow camera rotation
void AMultiCharacter::UpdateHoldPoint(float DeltaTime)
{
	// Get player controller and camera
	APlayerController* PC = Cast<APlayerController>(GetController());
	if (!PC || !PC->PlayerCameraManager)
	{
		return;
	}

	// Get camera location and rotation
	FVector CameraLocation = PC->PlayerCameraManager->GetCameraLocation();
	FRotator CameraRotation = PC->PlayerCameraManager->GetCameraRotation();

	// Determine hold distance (use object's override if set, otherwise use character's default)
	float EffectiveHoldDistance = HoldDistance;
	
	APhysicsGrabbable* Grabbable = Cast<APhysicsGrabbable>(HeldObject);
	if (Grabbable && Grabbable->GetHoldDistanceOverride() > 0.0f)
	{
		EffectiveHoldDistance = Grabbable->GetHoldDistanceOverride();
	}

	// Calculate desired position (in front of camera at effective hold distance)
	FVector DesiredLocation = CameraLocation + (CameraRotation.Vector() * EffectiveHoldDistance);

	// Smoothly interpolate hold point to desired location
	FVector NewLocation = FMath::VInterpTo(HoldPoint->GetComponentLocation(), DesiredLocation, DeltaTime, HoldPointRotationSpeed);
	HoldPoint->SetWorldLocation(NewLocation);

	// Optionally rotate hold point to face camera direction (for non-maintained rotation objects)
	// HoldPoint->SetWorldRotation(CameraRotation);
}