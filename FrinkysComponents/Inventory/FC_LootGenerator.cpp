// Copyright Frinky 2022


#include "FC_LootGenerator.h"

#include "Kismet/KismetSystemLibrary.h"


UFC_LootGenerator::UFC_LootGenerator()
{
	PrimaryComponentTick.bCanEverTick = false;
}


void UFC_LootGenerator::BeginPlay()
{
	Super::BeginPlay();
	
	if(bAutoFindInventoryComponent)
	{
		InventoryComponent = GetOwner()->FindComponentByClass<UFC_InventoryComponent>();
		if(InventoryComponent == nullptr)
		{
			GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, "No inventory component found on owner!");
			bSpawnOnBeginPlay = false;
		}
	}
	if(bSpawnOnBeginPlay)
	{
		GenerateLoot(true, MinItemsToSpawn, MaxItemsToSpawn);
	}

}

void UFC_LootGenerator::GenerateLoot_Implementation(bool bClearPreviousLoot, int Min, int Max)
{
	// Only perform this on the server
	if(!UKismetSystemLibrary::IsServer(GetWorld()))
	{
		return;
	}
	if(InventoryComponent == nullptr)
	{
		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, "No inventory component found on owner when trying to generate loot");
		return; 
	}

	// Clear the inventory if we need to
	if(bClearPreviousLoot)
	{
		InventoryComponent->ClearInventory();
	}

	// Get the number of items to spawn
	int ItemsToSpawn = FMath::RandRange(Min, Max);
	// Loop through and spawn the items
	for (int i = 0; i < ItemsToSpawn; ++i)
	{
		InventoryComponent->SpawnAndPickup( LootDataAsset->GetRandomItem() );
	}
}
