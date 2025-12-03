// Copyright Frinky 2022

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "FC_LootDataAsset.generated.h"

USTRUCT(BlueprintType)
struct FLootItemStruct
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float SpawnProbability = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSubclassOf<class AActor> ItemClass;
	
};


UCLASS()
class FRINKYSCOMPONENTS_API UFC_LootDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere)
	TArray<FLootItemStruct> LootItems;

	UFUNCTION(BlueprintCallable, BlueprintPure, meta=(ToolTip="Returns a random item from the loot table"))
	TSubclassOf<class AActor> GetRandomItem();

	
};
