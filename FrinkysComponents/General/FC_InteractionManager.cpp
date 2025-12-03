// Fill out your copyright notice in the Description page of Project Settings.


#include "FC_InteractionManager.h"

#include "GameFramework/PlayerController.h"
#include "Kismet/KismetSystemLibrary.h"


// Sets default values for this component's properties
UFC_InteractionManager::UFC_InteractionManager()
{
	PrimaryComponentTick.bCanEverTick = true;
}


void UFC_InteractionManager::TraceForInteractionPoints()
{
	//trace for an interaction point using the GetTraceDirection function
	FVector TraceDirection;
	FVector TraceLocation;
	GetTraceDirection(TraceDirection, TraceLocation);

	FHitResult HitOut;
	bool bHit = UKismetSystemLibrary::LineTraceSingle(GetWorld(), TraceLocation,
	                                                  TraceDirection * TraceDistance + TraceLocation, TraceChannel,
	                                                  bTraceComplex,
	                                                  TArray<AActor*>() = {OwningPawn, GetOwner()},
	                                                  TraceDebug, HitOut, true, FLinearColor::Blue,
	                                                  FLinearColor::Green);

	//if we hit something, check if it is an interaction point
	if (bHit)
	{
		UFC_InteractionPoint* InteractionPointHit = dynamic_cast<UFC_InteractionPoint*>(
			HitOut.GetComponent());
		LookingAtInteractionPoint = InteractionPointHit;
		if (!IsValid(LookingAtInteractionPoint))
		{
			StopInteracting();
		}
	}
	else
	{
		LookingAtInteractionPoint = nullptr;
		StopInteracting();
	}
}

// Called every frame
void UFC_InteractionManager::TickComponent(float DeltaTime, ELevelTick TickType,
                                           FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	if(!OwningPawn->IsLocallyControlled())
	{
		return;
	}

	if(!OwningPawn->IsPlayerControlled())
	{
		PrimaryComponentTick.bCanEverTick = false;
		return;
	}


	if(!bEnabled)
	{
		InteractionTimer = 0;
		InteractionPoint = nullptr;
		bIsInteractionTimerActive = false;
		return;
	}
	
	if (bHandleTrace)
	{
		TraceForInteractionPoints();
	}

	

	if (bIsInteractionTimerActive && IsValid(InteractionPoint) && OwningPawn->IsLocallyControlled())
	{
		InteractionTimer += DeltaTime;
		InteractionTimer = FMath::Clamp(InteractionTimer, 0.0f, InteractionTime);
		if (InteractionTimer >= InteractionTime)
		{
			UFC_InteractionPoint *InteractionPointToInteract = InteractionPoint;
			if(InteractionPoint->StopInteractionOnInteractComplete == true)
			{
				InteractionPoint = nullptr;
			}
			ServerCallInteract(dynamic_cast<APawn*>(GetOwner()), InteractionPointToInteract);
			InteractionPointToInteract->Interact(dynamic_cast<APawn*>(GetOwner()));

			bIsInteractionTimerActive = false;
		}
	}
}

void UFC_InteractionManager::BeginPlay()
{
	Super::BeginPlay();
	if (IsValid(dynamic_cast<APawn*>(GetOwner())))
	{
		SetComponentTickEnabled(true);
		OwningPawn = dynamic_cast<APawn*>(GetOwner());
	}
	else
	{
		SetComponentTickEnabled(false);
		OwningPawn = nullptr;
	}
}


void UFC_InteractionManager::SetInteractionPoint(UFC_InteractionPoint* NewInteractionPoint, bool bStartInteracting)
{
	if (IsValid(NewInteractionPoint) && NewInteractionPoint->isEnabled)
	{
		InteractionPoint = NewInteractionPoint;
		if (bStartInteracting)
		{
			StartInteracting();
		}
	}
}

void UFC_InteractionManager::ServerCallInteract_Implementation(APawn* Interactor,
                                                               UFC_InteractionPoint* InInteractionPoint)
{
	if (IsValid(InInteractionPoint))
	{
		InInteractionPoint->ServerInteract(Interactor); // Call the interact function on the server
	}
}

void UFC_InteractionManager::ServerCallStopInteract_Implementation(APawn* Interactor,
                                                                   UFC_InteractionPoint* InInteractionPoint)
{
	if (IsValid(InInteractionPoint))
	{
		InInteractionPoint->ServerStopInteract(Interactor);
	}
}

void UFC_InteractionManager::StartInteracting()
{
	if (IsValid(InteractionPoint))
	{
		bIsInteractionTimerActive = true;
		InteractionTimer = 0;
		InteractionTime = InteractionPoint->InteractionTime;
	}
	else
	{
		if (IsValid(LookingAtInteractionPoint))
		{
			InteractionPoint = LookingAtInteractionPoint;
			bIsInteractionTimerActive = true;
			InteractionTimer = 0;
			InteractionTime = InteractionPoint->InteractionTime;
		}
	}
}

void UFC_InteractionManager::StopInteracting()
{
	if (InteractionPoint && InteractionTimer >= InteractionTime)
	{
		InteractionPoint->StopInteract(dynamic_cast<APawn*>(GetOwner()));
		ServerCallStopInteract(dynamic_cast<APawn*>(GetOwner()), InteractionPoint);
	}
	bIsInteractionTimerActive = false;
	InteractionTimer = 0;
	InteractionTime = 0;
	InteractionPoint = nullptr;
}

float UFC_InteractionManager::GetInteractionProgress()
{
	return InteractionTimer / InteractionTime;
}

void UFC_InteractionManager::GetTraceDirection_Implementation(FVector& TraceDirection, FVector& TraceStart)
{
	// get direction from control rotation
	TraceDirection = OwningPawn->GetControlRotation().Vector();
	// get start location from camera
	APlayerController* Test = dynamic_cast<APlayerController*>(OwningPawn->GetController());
	if (IsValid(Test))
	{
		TraceStart = Test->PlayerCameraManager->GetCameraLocation();
	}
	else
	{
		TraceStart = OwningPawn->GetPawnViewLocation();
	}
	return;
}
