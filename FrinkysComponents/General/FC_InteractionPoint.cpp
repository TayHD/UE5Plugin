// Copyright Frinky 2022


#include "FC_InteractionPoint.h"

#include "Misc/MessageDialog.h"
#include "Net/UnrealNetwork.h"


// Sets default values for this component's properties
UFC_InteractionPoint::UFC_InteractionPoint()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;
	SetCollisionProfileName(FName("UI"));
	SetIsReplicatedByDefault(true);
	// ...
}


// Called when the game starts
void UFC_InteractionPoint::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}

void UFC_InteractionPoint::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UFC_InteractionPoint, InteractionName);
	DOREPLIFETIME(UFC_InteractionPoint, isEnabled);
}


// Called every frame
void UFC_InteractionPoint::TickComponent(float DeltaTime, ELevelTick TickType,
                                         FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UFC_InteractionPoint::Interact(APawn* Interactor)
{
	if (isEnabled)
	{
		OnInteract.Broadcast(Interactor);
	}
}

void UFC_InteractionPoint::StopInteract(APawn* Interactor)
{
	OnStopInteract.Broadcast(Interactor);
	
}

void UFC_InteractionPoint::ServerStopInteract_Implementation(APawn* Interactor)
{
	OnStopInteractServer.Broadcast(Interactor);
}


void UFC_InteractionPoint::ServerInteract_Implementation(APawn* Interactor)
{
	if (isEnabled)
	{
		OnInteractServer.Broadcast(Interactor);
	}
}


void UFC_InteractionPoint::GetInteractionInfo(bool& enabled, FString& name)
{
	enabled = isEnabled;
	name = InteractionName;
}

void UFC_InteractionPoint::SetEnabled_Implementation(bool NewEnabled)
{
	isEnabled = NewEnabled;
}


