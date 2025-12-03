// Copyright Frinky 2022

#pragma once

#include "CoreMinimal.h"
#include "FC_InventoryComponent.h"
#include "FC_LootDataAsset.h"
#include "Components/ActorComponent.h"
#include "FC_LootGenerator.generated.h"


UCLASS(ClassGroup=(FRINKYCORE), meta=(BlueprintSpawnableComponent))
class FRINKYSCOMPONENTS_API UFC_LootGenerator : public UActorComponent
{
	GENERATED_BODY()

public:
	UFC_LootGenerator();

protected:
	virtual void BeginPlay() override;
public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(ToolTip="The inventory component to add the items to"))
	UFC_InventoryComponent* InventoryComponent;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(ToolTip="The loot table to use"))
	UFC_LootDataAsset* LootDataAsset;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(ToolTip="Max items to spawn"))
	int MaxItemsToSpawn = 10;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(ToolTip="Min items to spawn"))
	int MinItemsToSpawn = 1;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(ToolTip="The loot table to use"))
	bool bSpawnOnBeginPlay = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(ToolTip="Should we auto find the inventory component in the owner?"))
	bool bAutoFindInventoryComponent = true;
	
	UFUNCTION(BlueprintCallable, Server, Reliable, meta=(ToolTip="Returns a random item from the loot table"))
	void GenerateLoot(bool bClearPreviousLoot, int Min, int Max);

};
