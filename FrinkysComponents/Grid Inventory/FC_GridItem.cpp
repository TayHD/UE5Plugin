// Copyright Frinky 2022


#include "FC_GridItem.h"

#include "FrinkysComponents/Inventory/FC_InventoryComponent.h"
#include "Net/UnrealNetwork.h"


UFC_GridItem::UFC_GridItem()
{

	PrimaryComponentTick.bCanEverTick = false;
}

void UFC_GridItem::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(UFC_GridItem, GridLocation)
	DOREPLIFETIME(UFC_GridItem, isRotated)
	DOREPLIFETIME(UFC_GridItem, GridSize)
	
}
