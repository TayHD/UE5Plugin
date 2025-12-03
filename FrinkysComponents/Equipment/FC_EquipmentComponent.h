// Copyright Frinky 2022

#pragma once

#include "CoreMinimal.h"
#include "FrinkysComponents/Attachables/FC_GenericAttachable.h"
#include "FC_EquipmentComponent.generated.h"

/**
 * 
 */

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnHeld, AActor*, ActorHolding);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FUnheld);

UCLASS(Blueprintable, ClassGroup=(FRINKYCORE), meta=(BlueprintSpawnableComponent))
class FRINKYSCOMPONENTS_API UFC_EquipmentComponent : public UFC_GenericAttachable
{
	GENERATED_BODY()

	UFC_EquipmentComponent();

public:
	UPROPERTY(BlueprintAssignable, meta=(ToolTip="Called when held"))
	FOnHeld OnHeld;
	UPROPERTY(BlueprintAssignable, meta=(ToolTip="called when unheld"))
	FUnheld OnUnheld;
	
};
