// Copyright Frinky 2022

#pragma once

#include "CoreMinimal.h"
#include "FC_Action.h"
#include "Components/ActorComponent.h"
#include "FC_ActionManager.generated.h"

USTRUCT(BlueprintType)
struct FActionStruct
{
	GENERATED_BODY()

public:
	
	UPROPERTY(BlueprintReadOnly, Category = "Action")
	TSubclassOf<AFC_Action> Action;

	UPROPERTY(BlueprintReadOnly, Category = "Action")
	UObject* Payload;

	inline bool operator==(const FActionStruct& Other) const
	{
		return Action == Other.Action;
	}
	
};



DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnActionAdded, TSubclassOf<AFC_Action>, Action);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnActionStarted, TSubclassOf<AFC_Action>, Action);

class AFC_Action;
UCLASS(ClassGroup=(FRINKYCORE), meta=(BlueprintSpawnableComponent))
class FRINKYSCOMPONENTS_API UFC_ActionManager : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UFC_ActionManager();

protected:

	// dorep
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual void BeginPlay() override;

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	

public:

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Action")
	TArray<FActionStruct> Actions; // array of queued actions

	UPROPERTY(BlueprintReadOnly, Category = "Action")
	FActionStruct CurrentAction; // current action

	UPROPERTY(BlueprintReadOnly, Category = "Action")
	APawn* OwningPawn;

	// time between evaluating actions
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Action")
	float ActionEvaluationInterval = 0.01f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Action")
	bool bDebug = false;

	UPROPERTY()
	FTimerHandle ActionEvaluationTimer;


	UPROPERTY(BlueprintAssignable)
	FOnActionAdded OnActionAdded;
	
	UPROPERTY(BlueprintAssignable)
	FOnActionStarted OnActionStarted;
	
	
	UFUNCTION(BlueprintCallable, Category = "Action")
	bool AddAction(TSubclassOf<AFC_Action> ActionToAdd, UObject* Payload);

	UFUNCTION(BlueprintCallable, Category = "Action")
	bool CanAddAction(AFC_Action* ActionToAdd);


	UFUNCTION()
	bool SpawnAndAddAction(TSubclassOf<AFC_Action> ActionToAdd, UObject* Payload);

	UFUNCTION(BlueprintPure, Category = "Action")
	bool IsPerformingAction();
	

	UFUNCTION(BlueprintCallable, Category = "Action")
	void EvaluateActions();

	

	UFUNCTION()
	void ExecuteAction(TSubclassOf<AFC_Action>& ActionToExecute, EActionExecutionState ExecutionState, UObject* Payload);

	UFUNCTION(Server, Reliable)
	void ServerExecuteAction(TSubclassOf<AFC_Action> ActionToExecute, UObject* Payload);

	UFUNCTION(NetMulticast, Reliable)
	void MultiExecuteAction(TSubclassOf<AFC_Action> ActionToExecute, UObject* Payload);
	
	AFC_Action* SpawnAction(const TSubclassOf<AFC_Action>& ActionToExecute);

	UFUNCTION()
	void EndAction(AFC_Action* AFC_Action);
};

