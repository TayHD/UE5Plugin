// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "FC_DynamicIconComponent.h"
#include "FC_ItemComponent.h"
#include "Components/DirectionalLightComponent.h"
#include "FC_DynamicIconSubsystem.generated.h"



// Struct for holding task data
USTRUCT()
struct FGenerateDynamicIconTask
{
	GENERATED_BODY()
	
	UFC_DynamicIconComponent* DynamicIconComponent;
	
	bool bForceUpdate;
	
	
};

/**
 * 
 */
UCLASS()
class FRINKYSCOMPONENTS_API UFC_DynamicIconSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	// Begin USubsystem
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	UFUNCTION(BlueprintCallable, Category = "FrinkysComponents|DynamicIcon", meta=(ToolTip="Request a dynamic icon to be generated, this will be added to a queue and executed when it's the next task in the queue, the bForceUpdate parameter will force the icon to be regenerated even if it already has been generated"))
	void RequestGenerateDynamicIcon(UFC_DynamicIconComponent* DynamicIconComponent, bool bForceUpdate = false, bool bSkipQueue = false);

private:
	// All my variables
	UPROPERTY()
	TArray<FGenerateDynamicIconTask> TaskQueue;

	UPROPERTY()
	USceneCaptureComponent2D* SceneCaptureComponent;

	UPROPERTY()
	FGenerateDynamicIconTask &CurrentTask = *(new FGenerateDynamicIconTask());
	
	void ExecuteTasks();
	void ExecuteNextTask();
	void InitSceneCapture(UFC_DynamicIconComponent* DynamicIconComponent);
	void ClearTempLighting(UFC_DynamicIconComponent* DynamicIconComponent, UDirectionalLightComponent* DirectionalLight1, UDirectionalLightComponent
	                       * DirectionalLight2, TMap<UPrimitiveComponent*, FLightingChannels> LightingChannelsMap);
	void GenerateDynamicIcon(FGenerateDynamicIconTask& Task);

	float TaskDelay;

	FTimerHandle TaskTimerHandle;
	

	
};
