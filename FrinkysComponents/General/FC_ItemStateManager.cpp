// Copyright Frinky 2022

#include "FC_ItemStateManager.h"

#include "Net/UnrealNetwork.h"

void UFC_ItemStateManager::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UFC_ItemStateManager, ActorState);
	
}

UFC_ItemStateManager::UFC_ItemStateManager()
{

	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
	ActorState = EActorState::Dropped;

}

void UFC_ItemStateManager::OnRep_ActorStateUpdated()
{

	if(ActorState == EActorState::Inventory)
	{
		GetOwner()->GetRootComponent()->SetVisibility(false, true);
		GetOwner()->SetActorEnableCollision(false);
		GetOwner()->SetReplicateMovement(false);
	}
	else if(ActorState == EActorState::Dropped)
	{
		GetOwner()->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
		GetOwner()->GetRootComponent()->SetVisibility(true, true);
		GetOwner()->SetActorEnableCollision(true);
		GetOwner()->SetReplicateMovement(true);
	}
	else if(ActorState == EActorState::Equipped)
	{
		GetOwner()->GetRootComponent()->SetVisibility(true, true);
		GetOwner()->SetActorEnableCollision(false);
		GetOwner()->SetReplicateMovement(true);
	}
	else
	{
		GetOwner()->GetRootComponent()->SetVisibility(true, true);
		GetOwner()->SetActorEnableCollision(true);
		GetOwner()->SetReplicateMovement(true);
	}

	//Broadcast
	OnActorStateUpdated.Broadcast(ActorState);
}



void UFC_ItemStateManager::Server_ChangeActorState_Implementation(EActorState NewState)
{
	ActorState = NewState;
	OnRep_ActorStateUpdated();
}


