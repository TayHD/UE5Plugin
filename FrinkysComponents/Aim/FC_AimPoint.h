// Copyright Frinky 2022

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "FC_AimPoint.generated.h"


UCLASS(Blueprintable, ClassGroup=(FRINKYCORE), meta=(BlueprintSpawnableComponent))
class FRINKYSCOMPONENTS_API UFC_AimPoint : public USceneComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UFC_AimPoint();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;


	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float AimDistance = 12;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float FOVChange = 0;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	bool Enabled = true;
};
