// Copyright Frinky 2022


#include "FC_GenericAttachablePoint.h"


#include "FC_GenericAttachable.h"
#include "Net/UnrealNetwork.h"

// Sets default values for this component's properties
UFC_GenericAttachablePoint::UFC_GenericAttachablePoint()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
	// ...
}


// Called when the game starts
void UFC_GenericAttachablePoint::BeginPlay()
{
	Super::BeginPlay();

	// ...
}


// Called every frame
void UFC_GenericAttachablePoint::TickComponent(float DeltaTime, ELevelTick TickType,
                                               FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UFC_GenericAttachablePoint::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UFC_GenericAttachablePoint, Data);
	DOREPLIFETIME(UFC_GenericAttachablePoint, OwningHandler);
}

void UFC_GenericAttachablePoint::OnRep_Data()
{
	if (Data.Attached && GetAttachedActor())
	{
		Data.Attached->OnAttached.Broadcast();
		GetAttachedActor()->AttachToComponent(this, FAttachmentTransformRules::SnapToTargetIncludingScale);
	}
	else
	{
		TArray<USceneComponent*> SceneComponents = GetAttachChildren();

		for (USceneComponent* Child : SceneComponents)
		{
			if (IsValid(Child->GetOwner()->FindComponentByClass<UFC_GenericAttachable>()) && Child->IsRegistered() &&  this->GetAttachChildren().Contains(Child))
			{
				Child->GetOwner()->FindComponentByClass<UFC_GenericAttachable>()->OnDetached.Broadcast();
				Child->GetOwner()->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
			}
		}

		
	}

	if(OwningHandler)
	{
		OwningHandler->CallOnAttachablesUpdateClient();
		if(Data.Attached)
		{
			OwningHandler->OnAttach.Broadcast(Data.Attached);
		}
		else
		{
			OwningHandler->OnDetach.Broadcast();
		}
		OwningHandler->OnAttachablesUpdate.Broadcast();	
	}
}

void UFC_GenericAttachablePoint::DropAttached()
{
	if(OwningHandler)
	{
		OwningHandler->DropSlot(this);
	}
}


UFC_GenericAttachable* UFC_GenericAttachablePoint::GetAttached()
{
	if (IsValid(Data.Attached))
	{
		return Data.Attached;
	}
	return nullptr;
}

AActor* UFC_GenericAttachablePoint::GetAttachedActor()
{
	if (IsValid(Data.Attached) && IsValid(Data.Attached->GetOwner()))
	{
		return Data.Attached->GetOwner();
	}
	return nullptr;
}

const bool UFC_GenericAttachablePoint::HasAttachment()
{
	if(Data.Attached != nullptr && IsValid(Data.Attached->GetOwner()))
	{
		return true;
	}

	return false;
	
}
