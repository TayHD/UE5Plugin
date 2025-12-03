// Copyright Frinky 2022


#include "FC_AI_Brain.h"

#include "Kismet/KismetSystemLibrary.h"


void UFC_AI_Brain::BeginPlay()
{
	Super::BeginPlay();
	AddTasks(DefaultTasks);
}

void UFC_AI_Brain::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	if(IsValid(CurrentTask))
	{
		GEngine->AddOnScreenDebugMessage(17, 100, FColor::Green, "Current Task:" + CurrentTask->GetClass()->GetName() );
		CurrentTask->TickTask(DeltaTime);
	}
}

UFC_AI_Brain::UFC_AI_Brain()
{

	PrimaryComponentTick.bCanEverTick = true;

}

void UFC_AI_Brain::ExecuteTask(AFC_AI_Task* Task)
{
	//on screen message telling what task is about to be executed
	
	if(!IsValid(CurrentTask))
	{
		GEngine->AddOnScreenDebugMessage(16, 100, FColor::Red, "No Current Task > Executing Task: " + Task->GetClass()->GetName() );
		CurrentTask = Task;
		CurrentTask->BrainRunningTask = this;
		CurrentTask->ExecuteTask(this);
	}
	else if(IsValid(CurrentTask) && Task->OverrideOtherTasks && !(CurrentTask == Task) && CurrentTask->Score < Task->Score)
	{
		AbortCurrentTask();
		GEngine->AddOnScreenDebugMessage(16, 100, FColor::Red, "Other Task Overriding > Executing Task: " + Task->GetClass()->GetName() );
		CurrentTask = Task;
		CurrentTask->BrainRunningTask = this;
		CurrentTask->ExecuteTask(this);
	} 
	else if(IsValid(CurrentTask) && CurrentTask->OverridenByOtherTasks && !(CurrentTask == Task) && CurrentTask->Score < Task->Score)
	{
		GEngine->AddOnScreenDebugMessage(16, 100, FColor::Red, "This Task is Allowed to be Overriden By Other Task > Executing Task: " + Task->GetClass()->GetName() );
		CurrentTask = Task;
		CurrentTask->BrainRunningTask = this;
		CurrentTask->ExecuteTask(this);
	}
	else
	{
		//GEngine->AddOnScreenDebugMessage(-1, 2, FColor::Red, "Task: " + Task->GetClass()->GetName() + " was not executed");
	}

}

void UFC_AI_Brain::DecideTask(ESortingMode SortingMode)
{
	AFC_AI_Task* SelectedTask = nullptr;
	if(Tasks.IsValidIndex(0))
	{
		for (AFC_AI_Task* Task : Tasks)
		{
			Task->Score = Task->ScoreTask();
		}

	
		if(SortingMode == ESortingMode::HighestScore)
		{
			//sort by highest score
			Tasks.Sort([](const AFC_AI_Task& A, const AFC_AI_Task& B) { return A.Score >= B.Score; });
			SelectedTask = Tasks[0];
		}
		else if(SortingMode == ESortingMode::LowestScore)
		{
			//sort by lowest score
			Tasks.Sort([](const AFC_AI_Task& A, const AFC_AI_Task& B) { return A.Score <= B.Score; });
			SelectedTask = Tasks[0];
		}
		else
		{
			//pick a random one from the array
			SelectedTask = Tasks[FMath::RandRange(0, Tasks.Num() - 1)];
		}

		GEngine->AddOnScreenDebugMessage(-1, 0.1, FColor::Blue, "---- ");
		int i = 0;
		for (auto Task : Tasks)
		{
			GEngine->AddOnScreenDebugMessage(i, 1, FColor::Blue, "Task: " + Task->GetClass()->GetName() + " Score: " + FString::SanitizeFloat(Task->Score) + (SelectedTask == Task ? " (Selected)" : ""));
			i++;

		}
	}
	if(IsValid(SelectedTask))
	{
		ExecuteTask(SelectedTask);
	}
}

void UFC_AI_Brain::AddTask(TSubclassOf<AFC_AI_Task> NewTask)
{
	for (auto Task : Tasks)
	{
		if(Task->GetClass() == NewTask)
		{
			return;
		}
	}
	// spawn task actor
	AFC_AI_Task* Task = GetWorld()->SpawnActor<AFC_AI_Task>(NewTask);
	Task->SetOwner(GetOwner());
	
	// add task to array
	Tasks.Add(Task);
	
}

void UFC_AI_Brain::AddTasks(TArray<TSubclassOf<AFC_AI_Task>> NewTasks)
{
	for (auto Task : NewTasks)
	{
		AddTask(Task);
	}
}

void UFC_AI_Brain::AbortCurrentTask()
{
	if(IsValid(CurrentTask))
	{
		//save the current tasks class
		TSubclassOf<AFC_AI_Task> CurrentTaskClass = CurrentTask->GetClass();
		//remove the current task from the array
		Tasks.Remove(CurrentTask);
		//destroy the current task
		CurrentTask->ConditionalBeginDestroy();
		//add the current task back to the array
		AddTask(CurrentTaskClass);
		//set the current task to null
		CurrentTask = nullptr;
		
	}
}

