// Copyright Frinky 2022

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "FrinkysComponents/Attachables/FC_GenericAttachableHandler.h"
#include "FC_ArmourHandlerComponent.generated.h"


UCLASS(ClassGroup=(FRINKYCORE), meta=(BlueprintSpawnableComponent))
class FRINKYSCOMPONENTS_API UFC_ArmourHandlerComponent : public UFC_GenericAttachableHandler
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UFC_ArmourHandlerComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;

	virtual void AutoRefreshSlots() override;

	UFUNCTION(BlueprintCallable, Category="Armour")
	float DamageArmour(FHitResult HitResult, float Damage);
};
