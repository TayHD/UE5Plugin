// Copyright Frinky 2022


#include "FC_AimHandler.h"

#include "Kismet/KismetMathLibrary.h"


// Sets default values for this component's properties
UFC_AimHandler::UFC_AimHandler()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	// ...
}


FTransform UFC_AimHandler::CalculateAim()
{
	if(IsValid(CurrentAimPoint))
	{
		float CurrentAimDistance;
		if(FixedAimDistance)
		{
			CurrentAimDistance = FixedAimDistance;
		}
		else
		{
			CurrentAimDistance = CurrentAimPoint->AimDistance;
		}
		FTransform RelativeAimTransform = UKismetMathLibrary::MakeRelativeTransform(CurrentAimPoint->GetComponentTransform(),
			GetOwner()->GetRootComponent()->GetComponentTransform());
		RelativeAimTransform.SetLocation((RelativeAimTransform.GetLocation()*-1) + FVector(xForward ? CurrentAimDistance : 0, xForward ? 0 : CurrentAimPoint->AimDistance, 0 ));
		RelativeAimTransform.SetRotation(RelativeAimTransform.GetRotation()*-1);
		RelativeAimTransform.SetLocation( UKismetMathLibrary::Quat_RotateVector(RelativeAimTransform.GetRotation(), RelativeAimTransform.GetLocation()));
		RelativeAimTransform.SetLocation( UKismetMathLibrary::Quat_RotateVector(RotateVector.Quaternion(), RelativeAimTransform.GetLocation()));
		return RelativeAimTransform;
	}

	return FTransform();
	
}

// Called when the game starts
void UFC_AimHandler::BeginPlay()
{
	Super::BeginPlay();
	if(AutoRefreshPoints)
	{
		RefreshPoints();
		AimTransform = CalculateAim();
	}
	
}

float UFC_AimHandler::GetFOVChange()
{
	if(CurrentAimPoint)
	{
		return CurrentAimPoint->FOVChange;
	}
	
	return 0;
}

void UFC_AimHandler::CycleSights()
{
	int Index = AimPointList.Find(CurrentAimPoint);
	if(Index == AimPointList.Num()-1)
	{
		CurrentAimPoint = AimPointList[0];
	}
	else
	{
		CurrentAimPoint = AimPointList[Index+1];
	}
	if(CurrentAimPoint->Enabled)
	{
		return;
	}
	ValidateCurrentAimPoint();
	
}



FTransform UFC_AimHandler::GetAim(bool& success)
{
	FTransform Aim = FTransform::Identity;

	if(CurrentAimPoint && IsValid(CurrentAimPoint))
	{
		Aim = UKismetMathLibrary::TInterpTo(AimTransform, CalculateAim(), GetWorld()->GetDeltaSeconds(), AimInterp);
		AimTransform = Aim;
		success = true;
		return Aim;
	}
	success = false;
	return Aim;
}

void UFC_AimHandler::RefreshPoints()
{
	if(GetOwner())
	{
		AimPointList.Empty();
		AimPointList = FindDecendentAimPoints(GetOwner()->GetRootComponent());
		
		ValidateCurrentAimPoint();
	}
	
}

void UFC_AimHandler::ValidateCurrentAimPoint()
{
	bool success = false;
	int iterator = 0;
	if(CurrentAimPoint && AimPointList.Contains(CurrentAimPoint) && CurrentAimPoint->Enabled) { return; }

	if(AimPointList.Num() == 0)
	{
		CurrentAimPoint = nullptr;
		return;
	}
	
	if(!CurrentAimPoint || !AimPointList.Contains(CurrentAimPoint) || !CurrentAimPoint->Enabled)
	{
		while(!success && iterator <= AimPointList.Num()*4)
		{
			int Index = AimPointList.Find(CurrentAimPoint);
			if(Index == AimPointList.Num()-1)
			{
				CurrentAimPoint = AimPointList[0];
			}
			else
			{
				CurrentAimPoint = AimPointList[Index+1];
			}
			if(CurrentAimPoint->Enabled)
			{
				return;	
			}

			
			iterator++;
		}
	}
	
	CurrentAimPoint = nullptr;
	
}

TArray<UFC_AimPoint*> UFC_AimHandler::FindDecendentAimPoints(USceneComponent* Target)
{
	
	TArray<UFC_AimPoint*> AimPoints;
	TArray<USceneComponent*> ScenePoints;
	Target->GetChildrenComponents(true, ScenePoints);

	for (auto SceneComponent : ScenePoints)
	{
		UFC_AimPoint* AimPoint = dynamic_cast<UFC_AimPoint*>(SceneComponent);
		if(AimPoint && IsValid(AimPoint))
		{
			AimPoints.Add(AimPoint);
		}
	}

	return AimPoints;
}

