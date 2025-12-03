// Copyright Frinky 2022


#include "FC_Action.h"

#include "FC_ActionManager.h"

void AFC_Action::OnAddedToQueue_Implementation()
{
	
}

AFC_Action::AFC_Action()
{
	PrimaryActorTick.bCanEverTick = false;
	SetReplicates(false);
	
}

void AFC_Action::BeginPlay()
{
	Super::BeginPlay();
}

void AFC_Action::OnExecute_Implementation(const EActionExecutionState ExecutionState, UObject* Payload)
{
}

void AFC_Action::EndAction()
{
	if(!ActionManager)
	{
		return;
	}
	ActionManager->EndAction(this);
}

APawn* AFC_Action::GetOwningPawn() const
{
	if(!ActionManager)
	{
		return nullptr;
		
	}
	return ActionManager->OwningPawn;
	
}

UObject* AFC_Action::GetActionPayload() const
{
	return ActionPayload;
}


