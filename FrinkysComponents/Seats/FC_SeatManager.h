// Copyright Frinky 2022

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "FC_SeatManager.generated.h"


UCLASS(ClassGroup = (FRINKYCORE), meta = (BlueprintSpawnableComponent))
class FRINKYSCOMPONENTS_API UFC_SeatManager : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UFC_SeatManager();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
};