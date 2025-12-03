// Copyright Frinky 2022


#include "FC_AI_Task.h"

#include "FC_AI_Brain.h"

float AFC_AI_Task::ScoreTask_Implementation()
{
	return 0.0f;
}

AFC_AI_Task::AFC_AI_Task()
{
}

AActor* AFC_AI_Task::GetOuterActor()
{
	return GetOwner();
}

void AFC_AI_Task::ExecuteTask_Implementation(UFC_AI_Brain* BrainCalling)
{
}

void AFC_AI_Task::TickTask_Implementation(float DeltaTime)
{
}

void AFC_AI_Task::FinishExecuteTask(bool success)
{
	BrainRunningTask->CurrentTask = nullptr;
	BrainRunningTask->AddTask(this->GetClass());
	Destroy();
	
}


