// Copyright Frinky 2022

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "FC_InventorySaver.generated.h"


UCLASS(ClassGroup=(FRINKYCORE), meta=(BlueprintSpawnableComponent))
class FRINKYSCOMPONENTS_API UFC_InventorySaver : public UActorComponent
{
	GENERATED_BODY()

public:
	UFC_InventorySaver();

protected:
	virtual void BeginPlay() override;


};
