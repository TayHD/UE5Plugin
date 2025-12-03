// Copyright Frinky 2022

#pragma once

#include "CoreMinimal.h"
#include "FC_AI_Task.generated.h"

class UFC_AI_Brain;
/**
 * 
 */
UCLASS(Blueprintable, ClassGroup=(FRINKYCORE), meta=(ShowWorldContextPin))
class FRINKYSCOMPONENTS_API AFC_AI_Task : public AInfo
{
	GENERATED_BODY()

	AFC_AI_Task();

public:
	UPROPERTY(BlueprintReadOnly)
	UFC_AI_Brain* BrainRunningTask;
	
	UPROPERTY(BlueprintReadOnly)
	float Score;

	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	bool OverrideOtherTasks = false;

	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	bool OverridenByOtherTasks = false;

	UFUNCTION(BlueprintPure, BlueprintNativeEvent, Category="FC_AI")
	float ScoreTask();

	UFUNCTION(BlueprintPure, Category="FC_AI", meta=(ToolTip="Gets the outer, if its an actor"))
	AActor* GetOuterActor();

	UFUNCTION(BlueprintNativeEvent, Category="FC_AI")
	void ExecuteTask(UFC_AI_Brain* BrainCalling);

	UFUNCTION(BlueprintNativeEvent, Category="FC_AI")
	void TickTask(float DeltaTime);

	UFUNCTION(BlueprintCallable, Category="FC_AI")
	void FinishExecuteTask(bool success);
	
};
