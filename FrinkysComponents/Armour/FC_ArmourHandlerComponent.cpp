// Copyright Frinky 2022


#include "FC_ArmourHandlerComponent.h"

#include "FC_ArmourComponent.h"
#include "FC_ArmourSlotComponent.h"


// Sets default values for this component's properties
UFC_ArmourHandlerComponent::UFC_ArmourHandlerComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UFC_ArmourHandlerComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void UFC_ArmourHandlerComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                               FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UFC_ArmourHandlerComponent::AutoRefreshSlots()
{
	TArray<USceneComponent*> ComponentsFound;
	TArray<UFC_GenericAttachablePoint*> SlotsFound;

	//Get all descendent child components, recursively
	GetOwner()->GetRootComponent()->GetChildrenComponents(true, ComponentsFound);

	//loop through them
	for (USceneComponent* Component : ComponentsFound)
	{
		//Attempt to cast to the scene component using dynamic cast (nullptr if not the right type)
		UFC_GenericAttachablePoint* a;
		a = dynamic_cast<UFC_ArmourSlotComponent*>(Component);
		if(IsValid(a))
		{
			//if our cast succeeds, add to the array
			SlotsFound.AddUnique(a);
			a->OwningHandler = this;

		}
	}
	//Set slots to the new value
	Slots = SlotsFound;
	OnRep_Slots();
}

float UFC_ArmourHandlerComponent::DamageArmour(FHitResult HitResult, float Damage)
{
	const FName BoneHit = HitResult.BoneName;
	for (auto Slot : Slots)
	{
		UFC_ArmourSlotComponent* ArmourSlot = dynamic_cast<UFC_ArmourSlotComponent*>(Slot);
		UFC_ArmourComponent* Armour = dynamic_cast<UFC_ArmourComponent*>(ArmourSlot->Data.Attached);
		if(Armour)
		{	
			if(Armour->BonesBlocked.Contains(BoneHit))
			{
				float FinalDamageOut;
				float DamageReduction = Armour->BaseDamageReductionPercent * Armour->ArmourDamageReductionCurve->GetFloatValue(Armour->GetHealthPercent());
				FinalDamageOut = Damage * DamageReduction;
				Armour->ServerApplyDamage(Damage);
				return FinalDamageOut;
			}
		}
	}
	return Damage;
}

