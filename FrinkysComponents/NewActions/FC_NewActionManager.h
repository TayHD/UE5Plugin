// Copyright Frinky 2022

#pragma once

#include "CoreMinimal.h"
#include "FC_NewAction.h"
#include "Components/ActorComponent.h"
#include "FC_NewActionManager.generated.h"

USTRUCT(BlueprintType)
struct FNewActionStruct
{
	GENERATED_BODY()

public:

	UPROPERTY(BlueprintReadOnly, Category="Action")
	AFC_NewAction* Action;

	UPROPERTY(BlueprintReadOnly, Category="Action")
	TArray<UObject*> Payloads;


};


UCLASS(ClassGroup=(FRINKYCORE), meta=(BlueprintSpawnableComponent))
class FRINKYSCOMPONENTS_API UFC_NewActionManager : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UFC_NewActionManager();

	

protected:


public:

};
