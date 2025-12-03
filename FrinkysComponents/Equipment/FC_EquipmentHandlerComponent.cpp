// Copyright Frinky 2022


#include "FrinkysComponents/Equipment/FC_EquipmentHandlerComponent.h"

#include "GameFramework/GameStateBase.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Net/UnrealNetwork.h"

UFC_EquipmentHandlerComponent::UFC_EquipmentHandlerComponent()
{
	Held = nullptr;
}

void UFC_EquipmentHandlerComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UFC_EquipmentHandlerComponent, Held);
	DOREPLIFETIME_CONDITION(UFC_EquipmentHandlerComponent, LastHeld, COND_OwnerOnly );
}

void UFC_EquipmentHandlerComponent::AutoRefreshSlots()
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
		a = dynamic_cast<UFC_EquipmentPointComponent*>(Component);
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

void UFC_EquipmentHandlerComponent::EquipSlot(UFC_GenericAttachable* Attachable, UFC_GenericAttachablePoint* SlotToAttach)
{
	Super::EquipSlot(Attachable, SlotToAttach);
	//HoldSlot(Held);
}

void UFC_EquipmentHandlerComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UFC_EquipmentHandlerComponent::HoldSlot_Implementation(UFC_EquipmentPointComponent* Slot) // Call on input (1 = Primary slot, etc)
{
	
	if(Slot != Held || Held) // if the slot we're trying to hold is not what we're already holding, or if we're not holding an actor
	{
		if(IsValid(Held) && IsValid(Held->GetAttachedActor()))
		{
			Held->GetAttachedActor()->AttachToComponent(Held, FAttachmentTransformRules::SnapToTargetIncludingScale);
			//Attach(CurrentHeld, CurrentHeldSlot);
			if(Held)
			{
				dynamic_cast<UFC_EquipmentComponent*>(Held->GetAttached())->OnUnheld.Broadcast();
				LastHeld = Held;
			}
		}

		
		Held = Slot;
		OnRep_Held();
	}
}

AActor* UFC_EquipmentHandlerComponent::GetHeldActor()
{
	if(IsValid(Held))
	{
		return Held->GetAttachedActor();
	}
	else
	{
		return nullptr;
	}
}

UFC_EquipmentComponent* UFC_EquipmentHandlerComponent::GetHeld()
{
	if(IsValid(Held))
	{
		return dynamic_cast<UFC_EquipmentComponent*>(Held->Data.Attached);;
	}
	else
	{
		return nullptr;
	}
}

void UFC_EquipmentHandlerComponent::UpdateHeld()
{
	OnHeldItemChanged.Broadcast();
	if(IsValid(Held) && IsValid(Held->GetAttached()))
	{
	
		dynamic_cast<UFC_EquipmentComponent*>(Held->GetAttached())->OnHeld.Broadcast(GetOwner());	
	}
}


void UFC_EquipmentHandlerComponent::OnRep_Held()
{
	// Debug
	UpdateHeld();
}

