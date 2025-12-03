// Copyright Frinky 2022

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "FrinkysComponents/Attachables/FC_GenericAttachablePoint.h"
#include "FC_ArmourSlotComponent.generated.h"


UCLASS(ClassGroup=(FRINKYCORE), meta=(BlueprintSpawnableComponent))
class FRINKYSCOMPONENTS_API UFC_ArmourSlotComponent : public UFC_GenericAttachablePoint
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UFC_ArmourSlotComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;
};
