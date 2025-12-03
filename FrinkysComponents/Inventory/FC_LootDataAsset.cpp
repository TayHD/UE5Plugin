// Copyright Frinky 2022


#include "FC_LootDataAsset.h"

TSubclassOf<AActor> UFC_LootDataAsset::GetRandomItem()
{
	if(LootItems.Num() == 0)
	{
		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, "No items in loot table, add some!");
		return nullptr;
	}
	
	// Get the total probability
	float TotalProbability = 0.0f;
	for (FLootItemStruct Item : LootItems)
	{
		TotalProbability += Item.SpawnProbability;
	}
	// Get a random number between 0 and the total probability
	float RandomNumber = FMath::FRandRange(0.0f, TotalProbability);
	// Loop through the items and subtract the spawn probability from the random number
	// If the random number is less than or equal to 0, return the item
	for (FLootItemStruct Item : LootItems)
	{
		RandomNumber -= Item.SpawnProbability;
		if (RandomNumber <= 0)
		{
			return Item.ItemClass;
		}
	}
	// If we get here, something is really broken
	GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, "Something went wrong with the loot table");
	return nullptr;
}
