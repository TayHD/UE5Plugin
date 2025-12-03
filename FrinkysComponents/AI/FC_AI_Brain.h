// Copyright Frinky 2022

#pragma once

#include "CoreMinimal.h"
#include "FC_AI_Task.h"
#include "Components/ActorComponent.h"
#include "FC_AI_Brain.generated.h"


UENUM(BlueprintType)
enum class ESortingMode : uint8 { HighestScore, LowestScore, Random };

UCLASS(ClassGroup=(FRINKYCORE), meta=(BlueprintSpawnableComponent))
class FRINKYSCOMPONENTS_API UFC_AI_Brain : public UActorComponent
{
	GENERATED_BODY()
protected:
	virtual void BeginPlay() override;

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
	// Sets default values for this component's properties
	UFC_AI_Brain();

	//Default tasks
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="FC_AI")
	TArray<TSubclassOf<AFC_AI_Task>> DefaultTasks;
	
	//Current Task
	UPROPERTY(BlueprintReadOnly)
	AFC_AI_Task* CurrentTask;

	//List of tasks
	UPROPERTY(BlueprintReadOnly)
	TArray<AFC_AI_Task*> Tasks;

	UFUNCTION(BlueprintCallable)
	void ExecuteTask(AFC_AI_Task* Task);

	UFUNCTION(BlueprintCallable, Category="FC_AI")
	void DecideTask(ESortingMode SortingMode = ESortingMode::HighestScore);

	UFUNCTION(BlueprintCallable, Category="FC_AI")
	void AddTask(TSubclassOf<AFC_AI_Task> NewTask);

	UFUNCTION(BlueprintCallable, Category="FC_AI")
	void AddTasks(TArray<TSubclassOf<AFC_AI_Task>> NewTasks);

	//abort current task function
	UFUNCTION(BlueprintCallable, Category="FC_AI")
	void AbortCurrentTask();


	
};
