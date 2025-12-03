// Copyright Frinky 2022


#include "FC_AIController.h"

#include "Navigation/CrowdFollowingComponent.h"
#include "Navigation/CrowdManager.h"

AFC_AIController::AFC_AIController(const FObjectInitializer& ObjectInitializer)
: Super(ObjectInitializer.SetDefaultSubobjectClass<UCrowdFollowingComponent>(TEXT("PathFollowingComponent")))
{
	// subclass constructor code
}

void AFC_AIController::UpdateControlRotation(float DeltaTime, bool bUpdatePawn) 
{
	APawn* const MyPawn = GetPawn();
	if(!MyPawn || MyPawn == nullptr) 
	{
		return;
	}

	FRotator NewControlRotation = MyPawn->GetControlRotation();

	// Look toward focus
	const FVector FocalPoint = GetFocalPoint();
	if(FAISystem::IsValidLocation(FocalPoint))
	{
		NewControlRotation = FMath::RInterpTo(MyPawn->GetControlRotation(), (FocalPoint - MyPawn->GetPawnViewLocation()).Rotation(), DeltaTime, 5.0f); 
	}
	else if(bSetControlRotationFromPawnOrientation)
	{
		NewControlRotation = MyPawn->GetActorRotation();
	}

	SetControlRotation(NewControlRotation);

	if(bUpdatePawn)
	{
		const FRotator CurrentPawnRotation = MyPawn->GetActorRotation();
		if(CurrentPawnRotation.Equals(NewControlRotation, 1e-3f) == false) 
		{
			MyPawn->FaceRotation(NewControlRotation, DeltaTime);
		}
	}
}

void AFC_AIController::RemoveFromCrowd()
{
	// get the crowd manager
	UCrowdManager * StaticCrowdManager = UCrowdManager::GetCurrent(this);
	if (StaticCrowdManager)
	{
		// we need to get the crowd component
		UCrowdFollowingComponent* FollowComponent = this->FindComponentByClass<UCrowdFollowingComponent>();
		// now, we need to get the ICrowdAgentInterface
		StaticCrowdManager->UnregisterAgent(FollowComponent);

	}
}
