// Copyright Frinky 2022


#include "FC_ArmourComponent.h"

#include "Kismet/KismetMathLibrary.h"
#include "Net/UnrealNetwork.h"


// Sets default values for this component's properties
UFC_ArmourComponent::UFC_ArmourComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	FSoftObjectPath MyAssetPath("CurveFloat'/FrinkysComponents/Curves/CT_ArmourResistanceBase.CT_ArmourResistanceBase'");
	ArmourDamageReductionCurve = dynamic_cast<UCurveFloat*>(MyAssetPath.TryLoad());

	// ...
}


// Called when the game starts
void UFC_ArmourComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}

void UFC_ArmourComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UFC_ArmourComponent, Health);
}


// Called every frame
void UFC_ArmourComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                        FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

float UFC_ArmourComponent::GetHealthPercent()
{
	return Health/MaxHealth;
}

void UFC_ArmourComponent::ServerApplyDamage_Implementation(float Damage)
{
	Health = UKismetMathLibrary::Clamp(Health-Damage, 0, MaxHealth);
	if(Health <= 0 && DestroyOnBreak)
	{
		GetOwner()->Destroy();
	}
}
