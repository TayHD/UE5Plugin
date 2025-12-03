// Copyright Frinky 2022

#pragma once

#include "CoreMinimal.h"
#include "FC_GridItem.h"
#include "Components/ActorComponent.h"
#include "FrinkysComponents/Inventory/FC_InventoryComponent.h"
#include "FC_GridInventory.generated.h"


UCLASS(ClassGroup=(FRINKYCORE), meta=(BlueprintSpawnableComponent))
class FRINKYSCOMPONENTS_API UFC_GridInventory : public UFC_InventoryComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UFC_GridInventory();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory", Replicated)
	FIntPoint InventorySize = FIntPoint(5, 5);
	

	// Functions

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool WillItemFit(UFC_GridItem* Item);

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool WillFit(FIntPoint Location, FIntPoint Size, bool isRotated);
	
protected:

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual void OnRep_ItemsUpdated() override;

};
