// Copyright Frinky 2022


#include "FC_PhysicsImpactSounds.h"

#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"


// Sets default values for this component's properties
UFC_PhysicsImpactSounds::UFC_PhysicsImpactSounds()
{
	PrimaryComponentTick.bCanEverTick = false;

	ImpactVelocityThreshold = 250;
	ObjectImpactSize = 5000;
	bUseRoot = true;
	
}


// Called when the game starts
void UFC_PhysicsImpactSounds::BeginPlay()
{
	Super::BeginPlay();
	if(bUseRoot)
	{
		SetCollisionComponent(dynamic_cast<UMeshComponent*>(GetOwner()->GetRootComponent()));
	}
	
	
	
	
}

void UFC_PhysicsImpactSounds::OnChildHit(UPrimitiveComponent* HitComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	float ImpactVelocity = NormalImpulse.Size();
	if(ImpactVelocity >= ImpactVelocityThreshold && IsValid(ImpactDataAsset))
	{
		float ImpactSize = ImpactVelocity / 5000;
		//clamp the impact size to 2
		ImpactSize = FMath::Clamp(ImpactSize, 0.0f, 2.0f);
		//get the surface type
		FSurfaceData SurfaceData = ImpactDataAsset->GetDataFromHitResult(Hit);

		//play the sound
		UGameplayStatics::SpawnSoundAtLocation(GetWorld(), SurfaceData.Sound, Hit.ImpactPoint, FRotator::ZeroRotator, ImpactSize, 1, 0, Attenuation, Concurrency);

	}
}

void UFC_PhysicsImpactSounds::SetCollisionComponent(UMeshComponent* NewCollisionComponent)
{
	
	//bind on hit event to our function
	if (IsValid(NewCollisionComponent))
	{
		NewCollisionComponent->OnComponentHit.AddDynamic(this, &UFC_PhysicsImpactSounds::OnChildHit);
		CollisionComponent = NewCollisionComponent;
	}
}


