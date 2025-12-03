// Copyright 2021 Glitch Games.
// Redistributed under the MIT license
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "MultiCharacter.generated.h"

class UMyCharacterMovementComponent; 
UCLASS()
class FRINKYSCOMPONENTS_API AMultiCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AMultiCharacter(const class FObjectInitializer& ObjectInitializer);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Called to setup replication
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UFUNCTION(BlueprintNativeEvent)
		void PhysNetCustom(float DeltaTime, int32 Iterations);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Smooth Network Movement")
		UMyCharacterMovementComponent* GetMultiplayerMovementComponent() const;

	UFUNCTION(BlueprintNativeEvent)
		void OnMovementUpdatedCustom(float DeltaSeconds, const FVector& OldLocation,const FVector& OldVelocity); 

	// Physics Grabbing System

	// Component that determines where held objects are positioned (in front of camera)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Physics Grabbing")
	USceneComponent* HoldPoint;

	// The object currently being held by this character (replicated to all clients)
	UPROPERTY(ReplicatedUsing = OnRep_HeldObject, BlueprintReadOnly, Category = "Physics Grabbing")
	AActor* HeldObject;

	// Settings for grabbing behavior
	
	// Distance in front of camera where objects are held
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Grabbing")
	float HoldDistance = 150.0f;

	// How fast the hold point follows camera rotation (smoothing)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Grabbing")
	float HoldPointRotationSpeed = 10.0f;

	// Multiplier for throw velocity when releasing with throw
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Grabbing")
	float ThrowStrength = 1000.0f;

	// Functions

	// Server RPC: Grab an object (called from client, executed on server)
	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "Physics Grabbing")
	void Server_GrabObject(AActor* ObjectToGrab);

	// Server RPC: Release the held object (called from client, executed on server)
	// bThrow: if true, applies throw velocity; if false, just drops it
	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "Physics Grabbing")
	void Server_ReleaseObject(bool bThrow);

	// Get the currently held object (null if not holding anything)
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Physics Grabbing")
	AActor* GetHeldObject() const { return HeldObject; }

	// Check if currently holding an object
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Physics Grabbing")
	bool IsHoldingObject() const { return HeldObject != nullptr; }

	// Calculate throw velocity based on camera direction and character velocity
	UFUNCTION(BlueprintCallable, Category = "Physics Grabbing")
	FVector CalculateThrowVelocity() const;

	// Multicast RPC: Synchronize object placement across all clients
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_PlaceObject(AActor* PlacedObject, FVector Location, FRotator Rotation, FVector Velocity);

protected:
	// Called when HeldObject replicates to clients
	UFUNCTION()
	void OnRep_HeldObject();

	// Update the hold point position each frame to follow camera
	void UpdateHoldPoint(float DeltaTime);

};