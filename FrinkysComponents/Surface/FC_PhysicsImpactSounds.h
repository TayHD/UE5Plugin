// Copyright Frinky 2022

#pragma once

#include "CoreMinimal.h"
#include "FC_SurfaceDataAsset.h"
#include "Components/ActorComponent.h"
#include "FC_PhysicsImpactSounds.generated.h"


UCLASS(ClassGroup=(FRINKYCORE), meta=(BlueprintSpawnableComponent))
class FRINKYSCOMPONENTS_API UFC_PhysicsImpactSounds : public UActorComponent
{
	GENERATED_BODY()

public:
	UFC_PhysicsImpactSounds();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Physics Impact Sounds", meta=(ToolTip="if true, we'll use the root component to call the on hit event"))
	bool bUseRoot = true;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Physics Impact Sounds", meta=(ToolTip="the minimum velocity required to play a sound"))
	float ImpactVelocityThreshold = 250;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Physics Impact Sounds", meta=(ToolTip="the amount we'll divide the impact velocity by to get the impact size"))
	float ObjectImpactSize = 5000;

	UPROPERTY(SaveGame, BlueprintReadOnly, EditAnywhere, Category="Physics Impact Sounds")
	USoundConcurrency* Concurrency;
	
	UPROPERTY(SaveGame, BlueprintReadWrite, EditAnywhere, Category="Physics Impact Sounds") 
	USoundAttenuation* Attenuation;

	UPROPERTY(BlueprintReadOnly, Category="Physics Impact Sounds", meta=(ToolTip="the component to bind the on hit event to"))
	UMeshComponent* CollisionComponent;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Physics Impact Sounds", meta=(ToolTip="this is the data asset FC_SurfaceDataAsset that contains the sound data for the surface"))
	UFC_SurfaceDataAsset* ImpactDataAsset;

	UFUNCTION(BlueprintCallable, Category="Physics Impact Sounds")
	void OnChildHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	UFUNCTION(BlueprintCallable, Category="Physics Impact Sounds")
	void SetCollisionComponent(UMeshComponent* NewCollisionComponent);

	

};
