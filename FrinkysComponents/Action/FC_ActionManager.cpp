// Copyright Frinky 2022


#include "FC_ActionManager.h"

#include "FC_Action.h"
#include "Net/UnrealNetwork.h"


UFC_ActionManager::UFC_ActionManager()
{
	PrimaryComponentTick.bCanEverTick = true;
	SetIsReplicatedByDefault(true);
	
}

void UFC_ActionManager::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

}

void UFC_ActionManager::BeginPlay()
{
	Super::BeginPlay();
	// set owning pawn
	OwningPawn = Cast<APawn>(GetOwner());

	// on screen message if not owning pawn
	if (!OwningPawn)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5, FColor::Red, TEXT("Action Manager not attached to Pawn"));
	}

	// start timer if locally controlled
	if (OwningPawn)
	{
		GetWorld()->GetTimerManager().SetTimer(ActionEvaluationTimer, this, &UFC_ActionManager::EvaluateActions, ActionEvaluationInterval, true);
	}
	
}

void UFC_ActionManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	
	
	if(bDebug && OwningPawn->GetNetMode() == NM_Client)
	{
		// print out actions
		// If actions is empty, print out "No Actions"

		GEngine->AddOnScreenDebugMessage(-1, 0, FColor::White, "---");

		if(Actions.Num() == 0)
		{
			GEngine->AddOnScreenDebugMessage(-1, 0, FColor::White, "No Actions");
		}
		else
		{
			
			
			for (FActionStruct Action : Actions)
			{
				if(!IsValid(Action.Action))
				{
					continue;
				}
				GEngine->AddOnScreenDebugMessage(-1, 0, FColor::White, Action.Action.GetDefaultObject()->GetName());
			}
			GEngine->AddOnScreenDebugMessage(-1, 0, FColor::White, "Actions:");
			
			
		}

		// print out current action
		if(IsValid(CurrentAction.Action))
		{
			GEngine->AddOnScreenDebugMessage(-1, 0, FColor::Orange, "Current Action: " + CurrentAction.Action->GetName());
		}
		else
		{
			GEngine->AddOnScreenDebugMessage(-1, 0, FColor::Orange, "No Current Action");
		}

		GEngine->AddOnScreenDebugMessage(-1, 0, FColor::White, "---");

		
	}
	
}

bool UFC_ActionManager::AddAction(TSubclassOf<AFC_Action> ActionToAdd, UObject* Payload)
{
	if(!CanAddAction(ActionToAdd.GetDefaultObject())) { return false; }
	if(OwningPawn && OwningPawn->IsLocallyControlled()) // 
	{
		return SpawnAndAddAction(ActionToAdd, Payload);
	}
	return false;
	
}



bool UFC_ActionManager::CanAddAction(AFC_Action* ActionToAdd)
{
	return true;
}




bool UFC_ActionManager::SpawnAndAddAction(TSubclassOf<AFC_Action> ActionToAdd, UObject* Payload)
{
	FActionStruct NewAction = FActionStruct();
	NewAction.Action = ActionToAdd;
	NewAction.Payload = Payload;
	if(ActionToAdd.GetDefaultObject()->bOnlyOneCanExist)
	{
		// if we only want one of this action to exist, we check if we already have one
		for (FActionStruct Action : Actions)
		{
			if(Action.Action == ActionToAdd)
			{
				GEngine->AddOnScreenDebugMessage(-1, 5, FColor::Red, TEXT("Action already exists in queue"));
				return false;
			}
		}
		// if we don't have one, we check if we are currently performing one
		if(IsValid(CurrentAction.Action))
		{
			GEngine->AddOnScreenDebugMessage(-1, 5, FColor::Red, "Action A: " + CurrentAction.Action->GetDefaultObject()->GetName() + " Action B: " + ActionToAdd->GetDefaultObject()->GetName());
			if(CurrentAction.Action->GetDefaultObject()->GetName() == ActionToAdd.GetDefaultObject()->GetName())
			{
				GEngine->AddOnScreenDebugMessage(-1, 5, FColor::Red, TEXT("Action already exists in as current action"));
				return false;
			}
		}
	} // One exist check
	
	if (!CanAddAction(ActionToAdd.GetDefaultObject())) { return false; } // can add check

	
	if (ActionToAdd.GetDefaultObject()->bClearsActions) // if this action clears actions, we clear the array
	{
		GEngine->AddOnScreenDebugMessage(-1, 5, FColor::Green, TEXT("Clearing actions"));
		Actions.Empty();
	}

	// add action
	if(ActionToAdd.GetDefaultObject()->bTakesPriority)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5, FColor::Green, TEXT("Adding priority action"));
		Actions.Insert(NewAction, 0);
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(-1, 5, FColor::Green, TEXT("Adding normal action"));
		Actions.Add(NewAction);
	}
	ActionToAdd.GetDefaultObject()->ActionPayload = Payload;
	OnActionAdded.Broadcast(ActionToAdd);
	return true;

}

bool UFC_ActionManager::IsPerformingAction()
{
	return CurrentAction.Action != nullptr;
}


void UFC_ActionManager::EvaluateActions()
{
	if(!OwningPawn || !OwningPawn->IsLocallyControlled() )
	{
		SetComponentTickEnabled(false);
		return; // if we are not locally controlled, we don't evaluate actions
	}
	if(Actions.Num() == 0)
	{
		
		return; // if we have no actions, we don't evaluate actions
	}

	if(IsValid(CurrentAction.Action))
	{
		return; // if we are currently performing an action, we don't evaluate actions
	}
	
	// make a copy of the first action in the array
	FActionStruct ActionToEvaluate = Actions[0];
	
	
	if(IsValid(ActionToEvaluate.Action) && OwningPawn)
	{
		CurrentAction = ActionToEvaluate;
		Actions.RemoveSingle(ActionToEvaluate);
		if(!OwningPawn->HasAuthority())
		{
			ExecuteAction(ActionToEvaluate.Action, EActionExecutionState::ExecutingLocal, ActionToEvaluate.Payload);

		}
		ServerExecuteAction(ActionToEvaluate.Action, ActionToEvaluate.Payload);
		
	}
}

void UFC_ActionManager::ExecuteAction(TSubclassOf<AFC_Action>& ActionToExecute, EActionExecutionState ExecutionState, UObject* Payload)
{
	if(!IsValid(ActionToExecute)) { return; }
	
	AFC_Action* SpawnedAction; // create a pointer to the action
	SpawnedAction = SpawnAction(ActionToExecute); // spawn the action
	SpawnedAction->ActionPayload = Payload; // set the payload
	SpawnedAction->OnExecute(ExecutionState, Payload); // execute the action
	OnActionStarted.Broadcast(ActionToExecute); // broadcast that the action has started

	
}

void UFC_ActionManager::ServerExecuteAction_Implementation(TSubclassOf<AFC_Action> ActionToExecute, UObject* Payload)
{
	ExecuteAction(ActionToExecute, EActionExecutionState::ExecutingOnServer, Payload);
	MultiExecuteAction(ActionToExecute, Payload);
}

void UFC_ActionManager::MultiExecuteAction_Implementation(TSubclassOf<AFC_Action> ActionToExecute, UObject* Payload)
{
	if(OwningPawn && !OwningPawn->IsLocallyControlled())
	{
		ExecuteAction(ActionToExecute, EActionExecutionState::ExecutingRemote, Payload);
	}
}

AFC_Action* UFC_ActionManager::SpawnAction(const TSubclassOf<AFC_Action>& ActionToExecute)
{
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	AFC_Action* Action = GetWorld()->SpawnActor<AFC_Action>(ActionToExecute, SpawnParams);
	// set the action manager
	Action->ActionManager = this;
	// execute the action
	return Action;
}

void UFC_ActionManager::EndAction(AFC_Action* AFC_Action)
{
	CurrentAction.Action = nullptr;
	CurrentAction.Payload = nullptr;
	AFC_Action->Destroy();
}
