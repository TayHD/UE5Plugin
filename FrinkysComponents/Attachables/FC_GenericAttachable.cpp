// Copyright Frinky 2022


#include "FC_GenericAttachable.h"

#include "FC_GenericAttachablePoint.h"
#include "FrinkysComponents/General/FC_ItemStateManager.h"
#include "Net/UnrealNetwork.h"

// Sets default values for this component's properties
UFC_GenericAttachable::UFC_GenericAttachable()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
	Name = "Attachable";
	// ...
}


// Called when the game starts
void UFC_GenericAttachable::BeginPlay()
{
	Super::BeginPlay();

	// ...

	ItemStateManager = GetOwner()->FindComponentByClass<UFC_ItemStateManager>();
	if (!IsValid(ItemStateManager))
	{
		//On screen message telling the user to add the component
		GEngine->AddOnScreenDebugMessage(-1, 100000, FColor::Red, "NO ITEM STATE MANAGER FOUND! PLEASE ADD FC ItemStateManager TO ITEM ACTOR! INVENTORY WILL BE BROKEN");
	}

	
	
}


void UFC_GenericAttachable::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{

	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UFC_GenericAttachable, Name);
	DOREPLIFETIME(UFC_GenericAttachable, OwningSlot);
	
}	


// Called every frame
void UFC_GenericAttachable::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UFC_GenericAttachable::Detach()
{
	if(OwningSlot)
	{
		OwningSlot->DropAttached();
	}
}

void UFC_GenericAttachable::UseItem(FString Tag)
{
	UseItemClient(Tag);
	UseItemServer(Tag);
	
}

void UFC_GenericAttachable::UseItemServer_Implementation(const FString& Tag)
{
	UseItemMulti(Tag);
	ServerUsed.Broadcast(Tag);
}

void UFC_GenericAttachable::UseItemMulti_Implementation(const FString& Tag)
{
	MultiUsed.Broadcast(Tag);
}

void UFC_GenericAttachable::UseItemClient_Implementation(const FString& Tag)
{
	ClientUsed.Broadcast(Tag);
}

