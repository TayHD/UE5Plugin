// Copyright Frinky 2022

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "FrinkysComponents/Attachables/FC_GenericAttachable.h"
#include "FC_ArmourComponent.generated.h"


UCLASS(ClassGroup=(FRINKYCORE), meta=(BlueprintSpawnableComponent))
class FRINKYSCOMPONENTS_API UFC_ArmourComponent : public UFC_GenericAttachable
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UFC_ArmourComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Settings|Armour")
	TArray<FName> BonesBlocked;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Settings|Armour")
	bool DestroyOnBreak = false;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Settings|Armour")
	UCurveFloat* ArmourDamageReductionCurve;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Settings|Armour")
	float BaseDamageReductionPercent = 0.75;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Settings|Armour")
	float MaxHealth = 50;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Settings|Armour", Replicated)
	float Health = 50;

	UFUNCTION(Server, Reliable, BlueprintCallable)
	void ServerApplyDamage(float Damage);

	UFUNCTION(BlueprintCallable, BlueprintPure)
	float GetHealthPercent();
};
