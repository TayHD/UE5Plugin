// Copyright Frinky 2022

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FrinkysComponents/General/FC_InteractionPoint.h"
#include "PhysicsGrabbable.generated.h"

/**
 * PhysicsGrabbable - An actor that can be grabbed, carried, and placed by players
 * Similar to Half-Life 2 physics object interaction
 * 
 * How it works:
 * 1. Has a mesh that simulates physics when not held
 * 2. Has an interaction point that players can interact with
 * 3. When grabbed, disables physics and attaches to player
 * 4. When released, re-enables physics and can be thrown
 */
UCLASS()
class FRINKYSCOMPONENTS_API APhysicsGrabbable : public AActor
{
	GENERATED_BODY()
	
public:	
	// Constructor - sets up default values
	APhysicsGrabbable();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Called to setup replication
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:	
	// The visual mesh component that has physics
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* MeshComponent;

	// The interaction point that players can interact with
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UFC_InteractionPoint* InteractionPoint;

	// Optional: Scene component defining where the player's hand/hold point should attach
	// If set, object will be held at this point instead of root. Leave null to use root/center.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USceneComponent* HoldLocationPoint;

	// Settings

	// Can this object be grabbed?
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grabbable Settings")
	bool bCanBeGrabbed = true;

	// Maximum distance from which this object can be grabbed
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grabbable Settings")
	float MaxGrabDistance = 500.0f;

	// Should the object maintain its rotation when grabbed, or follow camera orientation?
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grabbable Settings")
	bool bMaintainRotationWhenHeld = false;

	// Override the character's hold distance for this specific object (0 = use character's default)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grabbable Settings")
	float HoldDistanceOverride = 0.0f;

	// Current State

	// Is this object currently being held by someone?
	UPROPERTY(ReplicatedUsing = OnRep_IsBeingHeld, BlueprintReadOnly, Category = "Grabbable State")
	bool bIsBeingHeld = false;

	// Who is currently holding this object?
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Grabbable State")
	APawn* CurrentHolder = nullptr;

	// Functions

	// Called when a player interacts with this object (from FC_InteractionPoint)
	UFUNCTION()
	void OnInteracted(APawn* Interactor);

	// Called when the held state changes (replication callback)
	UFUNCTION()
	void OnRep_IsBeingHeld();

	// Get whether this object is currently being held
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Grabbable")
	bool IsBeingHeld() const { return bIsBeingHeld; }

	// Get who is currently holding this object
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Grabbable")
	APawn* GetCurrentHolder() const { return CurrentHolder; }

	// Get the hold location point component (null if using root/center)
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Grabbable")
	USceneComponent* GetHoldLocationPoint() const { return HoldLocationPoint; }

	// Get the custom hold distance override (0 = use character default)
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Grabbable")
	float GetHoldDistanceOverride() const { return HoldDistanceOverride; }

	// Set the held state (called by character when grabbing/releasing)
	UFUNCTION(BlueprintCallable, Category = "Grabbable")
	void SetHeldState(bool bHeld, APawn* Holder);

	// Blueprint events for customization
	UPROPERTY(BlueprintAssignable, Category = "Grabbable Events")
	FInteractSignature OnGrabbed;

	UPROPERTY(BlueprintAssignable, Category = "Grabbable Events")
	FInteractSignature OnObjectReleased;
};