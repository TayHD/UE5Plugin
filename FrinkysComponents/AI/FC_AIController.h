// Copyright Frinky 2022

#pragma once

#include "CoreMinimal.h"
#include "DetourCrowdAIController.h"
#include "FC_AIController.generated.h"

/**
 * 
 */
UCLASS()
class FRINKYSCOMPONENTS_API AFC_AIController : public AAIController
{
	GENERATED_BODY()

	
	
public:
	virtual void UpdateControlRotation(float DeltaTime, bool bUpdatePawn = true) override;

	UFUNCTION(BlueprintCallable, Category = "AI")
	void RemoveFromCrowd();

	AFC_AIController(const FObjectInitializer& ObjectInitializer);

	
};
