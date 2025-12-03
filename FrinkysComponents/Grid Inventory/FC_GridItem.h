// Copyright Frinky 2022

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "FrinkysComponents/Inventory/FC_ItemComponent.h"
#include "FC_GridItem.generated.h"


UCLASS(ClassGroup=(FRINKYCORE), meta=(BlueprintSpawnableComponent))
class FRINKYSCOMPONENTS_API UFC_GridItem : public UFC_ItemComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UFC_GridItem();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item", Replicated)
	FIntPoint GridLocation = FIntPoint(0, 0);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item", Replicated)
	FIntPoint GridSize = FIntPoint(1, 1);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item", Replicated)
	bool isRotated = false;


public:

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;


	
};
