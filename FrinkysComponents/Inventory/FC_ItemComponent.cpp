// Fill out your copyright notice in the Description page of Project Settings.


#include "FC_ItemComponent.h"

#include "FC_InventoryComponent.h"
#include "FrinkysComponents/Attachables/FC_GenericAttachable.h"
#include "FrinkysComponents/General/FC_ItemStateManager.h"

#include "Net/UnrealNetwork.h"

// Sets default values for this component's properties
UFC_ItemComponent::UFC_ItemComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);

	Weight = 1.0;
	ItemName = "Item";
	Uses = 1;
	CanBeUsed = false;

}


// Called when the game starts
void UFC_ItemComponent::BeginPlay()
{
	Super::BeginPlay();
	// ...
	ItemStateManager = GetOwner()->FindComponentByClass<UFC_ItemStateManager>();
	if (!IsValid(ItemStateManager))
	{
		//On screen message telling the user to add the component
		GEngine->AddOnScreenDebugMessage(-1, 100000, FColor::Red, "NO ITEM STATE MANAGER FOUND! PLEASE ADD FC ItemStateManager TO EQUIPPABLE ACTOR! INVENTORY WILL BE BROKEN");
	}
}

void UFC_ItemComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UFC_ItemComponent, Weight);
	DOREPLIFETIME(UFC_ItemComponent, OwningInventory)
	DOREPLIFETIME(UFC_ItemComponent, Uses);
}

void UFC_ItemComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	if(OwningInventory)
	{
		OwningInventory->ValidateItems();
	}
}




void UFC_ItemComponent::Drop_Implementation()
{
	if(OwningInventory)
	{
		OwningInventory->DropItem(this);
	}
}

void UFC_ItemComponent::UseItem_Implementation()
{
	if(CanBeUsed && Uses > 0)
	{
		Uses--;
		OnUseItem.Broadcast();
		if(Uses == 0 && DestroyOnUse)
		{
			GetOwner()->Destroy();
			OwningInventory->ValidateItems();
		}
	}
}


