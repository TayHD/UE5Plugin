// Copyright Frinky 2022

#pragma once

#include "CoreMinimal.h"
#include "FC_AimPoint.h"
#include "Components/ActorComponent.h"
#include "FC_AimHandler.generated.h"


UCLASS(Blueprintable, ClassGroup=(FRINKYCORE), meta=(BlueprintSpawnableComponent))
class FRINKYSCOMPONENTS_API UFC_AimHandler : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UFC_AimHandler();
	FTransform CalculateAim();
	void GetAim();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:

	UFUNCTION(BlueprintPure)
	float GetFOVChange();
	

	UFUNCTION(BlueprintCallable)
	void CycleSights();

	UFUNCTION(BlueprintCallable)
	FTransform GetAim(bool& success);

	UFUNCTION(BlueprintCallable)
	void RefreshPoints();

	UFUNCTION(BlueprintCallable)
	void ValidateCurrentAimPoint();

	UFUNCTION(BlueprintCallable)
	TArray<UFC_AimPoint*> FindDecendentAimPoints(USceneComponent* Target);
	


	


	// Propertys
	UPROPERTY(BlueprintReadOnly)
	UFC_AimPoint* CurrentAimPoint;

	UPROPERTY(BlueprintReadOnly)
	TArray<UFC_AimPoint*> AimPointList;

	UPROPERTY(BlueprintReadOnly)
	FTransform AimTransform;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float AimInterp = 15;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Fixed Aim Distance")
	float FixedAimDistance = 30;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Fixed Aim Distance", meta=(ToolTip="fixed aim distance means that the parent determines the aim distance, not the individual aim point"))
	bool UseFixedAimDistance = false;
	

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	bool xForward = false;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	bool InvertRotation = false;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	bool AutoRefreshPoints = true;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FRotator RotateVector;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FVector EyeOffset;


	

	
};
