// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "FC_InteractionPoint.h"
#include "Components/ActorComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "FC_InteractionManager.generated.h"


UCLASS(ClassGroup=(FRINKYCORE), meta=(BlueprintSpawnableComponent), Blueprintable)
class FRINKYSCOMPONENTS_API UFC_InteractionManager : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UFC_InteractionManager();
	void TraceForInteractionPoints();

protected:

public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	virtual void BeginPlay() override;

	UPROPERTY(BlueprintReadOnly)
	UFC_InteractionPoint* LookingAtInteractionPoint;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Interaction|Trace")
	bool bHandleTrace = true;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Interaction|Trace")
	bool bEnabled = true;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Interaction|Trace")
	float TraceDistance = 100;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Interaction|Trace")
	TEnumAsByte<ETraceTypeQuery>  TraceChannel;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Interaction|Trace")
	TEnumAsByte<EDrawDebugTrace::Type> TraceDebug;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Interaction|Trace")
	bool bTraceComplex = false;
	
	UPROPERTY(BlueprintReadOnly, Category = "Interaction")
	float InteractionTimer = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Interaction")
	float InteractionTime = 3;

	UPROPERTY(BlueprintReadOnly, Category = "Interaction")
	bool bIsInteractionTimerActive = false;
	
	UPROPERTY(BlueprintReadOnly, Category = "Interaction")
	UFC_InteractionPoint* InteractionPoint;

	UPROPERTY(BlueprintReadOnly, Category = "Interaction")
	APawn* OwningPawn;

	UFUNCTION(BlueprintCallable, Category = "Interaction")
		void SetInteractionPoint(UFC_InteractionPoint* NewInteractionPoint, bool bStartInteracting = true);

	// Server function for calling interact
	UFUNCTION(Server, Reliable, Category = "Interaction")
		void ServerCallInteract(APawn* Interactor, UFC_InteractionPoint* InInteractionPoint);

	UFUNCTION(Server, Reliable, Category = "Interaction")
		void ServerCallStopInteract(APawn* Interactor, UFC_InteractionPoint* InInteractionPoint);
	
	UFUNCTION(BlueprintCallable, Category = "Interaction")
		void StartInteracting();

	UFUNCTION(BlueprintCallable, Category = "Interaction")
		void StopInteracting();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Interaction")
		float GetInteractionProgress();

	UFUNCTION(BlueprintCallable, BlueprintPure, BlueprintNativeEvent)
		void GetTraceDirection(FVector& TraceDirection, FVector& TraceStart);

	
	
};
