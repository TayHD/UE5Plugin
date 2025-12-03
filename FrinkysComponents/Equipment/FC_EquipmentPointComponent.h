// Copyright Frinky 2022

#pragma once

#include "CoreMinimal.h"
#include "FrinkysComponents/Attachables/FC_GenericAttachablePoint.h"
#include "FC_EquipmentPointComponent.generated.h"

/**
 * 
 */
UCLASS(Blueprintable, ClassGroup=(FRINKYCORE), meta=(BlueprintSpawnableComponent))
class FRINKYSCOMPONENTS_API UFC_EquipmentPointComponent : public UFC_GenericAttachablePoint
{
	GENERATED_BODY()

	UFC_EquipmentPointComponent();

	virtual void BeginPlay() override;
	
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	

public:

	virtual void OnRep_Data() override;
	
	
};
