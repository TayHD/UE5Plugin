// Copyright Frinky 2022


#include "FC_GridInventory.h"

#include "Net/UnrealNetwork.h"

UFC_GridInventory::UFC_GridInventory()
{

	PrimaryComponentTick.bCanEverTick = false;
	

}

bool UFC_GridInventory::WillItemFit(UFC_GridItem* Item)
{
	return WillFit(Item->GridLocation, Item->GridSize, Item->isRotated);
}

bool UFC_GridInventory::WillFit(FIntPoint Location, FIntPoint Size, bool isRotated)
{
	FIntPoint FinalSize = Size;
	
	if (isRotated)
	{
		FinalSize = FIntPoint(Size.Y, Size.X);
	}

	// Check if the item will fit in the inventory
	if (Location.X + FinalSize.X > InventorySize.X || Location.Y + FinalSize.Y > InventorySize.Y)
	{
		return false;
	}

	return true;
}

void UFC_GridInventory::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(UFC_GridInventory, InventorySize, COND_InitialOnly);
}

void UFC_GridInventory::OnRep_ItemsUpdated()
{
	Super::OnRep_ItemsUpdated(); // Call the base class version of this function first

	
	
}
