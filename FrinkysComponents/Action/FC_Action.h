// Copyright Frinky 2022

#pragma once

#include "CoreMinimal.h"
#include "FC_Action.generated.h"

// forward declare
class UFC_ActionManager;

UENUM(BlueprintType)
enum EActionExecutionState
{
	ExecutingLocal,
	ExecutingRemote,
	ExecutingOnServer
	
};

/**
 * 
 */
UCLASS(Blueprintable, ClassGroup=(FRINKYCORE), meta=(ShowWorldContextPin))
class FRINKYSCOMPONENTS_API AFC_Action : public AInfo
{
	GENERATED_BODY()

public:

	AFC_Action();

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Action|Queue")
	bool bTakesPriority = false;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Action|Queue")
	bool bClearsActions = false;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Action|Queue")
	bool bInterrupts = false;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Action|Queue")
	bool bOnlyOneCanExist = false;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Action")
	bool bClientPredicts = true;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Action")
	UFC_ActionManager* ActionManager;

	UPROPERTY()
	UObject* ActionPayload;

	virtual void BeginPlay() override;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Action|Queue")
	void OnAddedToQueue();
	
	// on execute
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Action")
	void OnExecute(const EActionExecutionState ExecutionState, UObject* Payload = nullptr);

	UFUNCTION(BlueprintCallable, Category = "Action")
	void EndAction();


	// get owning pawn
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Action")
	APawn* GetOwningPawn() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Action")
	UObject* GetActionPayload() const;

	
};
