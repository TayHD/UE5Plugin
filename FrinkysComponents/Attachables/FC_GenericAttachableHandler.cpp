// Copyright Frinky 2022


#include "FC_GenericAttachableHandler.h"
#include "FC_GenericAttachable.h"
#include "FC_GenericAttachablePoint.h"
#include "FrinkysComponents/Inventory/FC_InventoryComponent.h"
#include "FrinkysComponents/Inventory/FC_ItemComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"

// Sets default values for this component's properties
UFC_GenericAttachableHandler::UFC_GenericAttachableHandler()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
	Slots = TArray<UFC_GenericAttachablePoint*>();
	AutoGetSlots = true;

	// ...
}


// Called when the game starts
void UFC_GenericAttachableHandler::BeginPlay()
{
	Super::BeginPlay();
	if(AutoGetSlots && GetOwnerRole() == ROLE_Authority)
	{
		AutoRefreshSlots();

	}

}

void UFC_GenericAttachableHandler::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	for (auto Slot : Slots)
	{
		if(Slot->GetAttachedActor() != nullptr)
		{
			Slot->GetAttachedActor()->Destroy();
		}
	}
}

void UFC_GenericAttachableHandler::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UFC_GenericAttachableHandler, Slots);
}

// Called every frame
void UFC_GenericAttachableHandler::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UFC_GenericAttachableHandler::OnRep_Slots()
{
}

//Finds an empty and compatible slot
UFC_GenericAttachablePoint* UFC_GenericAttachableHandler::FindEmptySlot(UFC_GenericAttachable* Attachable)
{
	for (UFC_GenericAttachablePoint* Slot : Slots)
	{
		if(DoesFitSlot(Attachable, Slot) && !IsValid(Slot->Data.Attached))
		{
			return Slot;
		}
	}

	return nullptr;
}

UFC_GenericAttachablePoint* UFC_GenericAttachableHandler::FindSlot(UFC_GenericAttachable* Attachable)
{
	for (UFC_GenericAttachablePoint* Slot : Slots)
	{
		if(DoesFitSlot(Attachable, Slot))
		{
			return Slot;
		}
	}

	return nullptr;
}

UFC_GenericAttachablePoint* UFC_GenericAttachableHandler::FindSlotByTag(FGameplayTag Tag)
{
	for (UFC_GenericAttachablePoint* Slot : Slots)
	{
		if(Slot->Data.Tag.MatchesTag(Tag))
		{
			return Slot;
		}
	}

	return nullptr;
}

bool UFC_GenericAttachableHandler::DoesFitSlot(UFC_GenericAttachable* Attachable, UFC_GenericAttachablePoint* SlotToCheck)
{
	return IsValid(Attachable) && IsValid(SlotToCheck) && Attachable->SlotTag.MatchesTag(SlotToCheck->Data.Tag);
}

void UFC_GenericAttachableHandler::EquipSlot_Implementation(UFC_GenericAttachable* Attachable, UFC_GenericAttachablePoint* SlotToAttach)
{
	if(DoesFitSlot(Attachable, SlotToAttach) )
	{
		if(IsValid(SlotToAttach->GetAttached()))
		{
			SlotToAttach->GetAttached()->Detach();
		}
		Attach(Attachable, SlotToAttach);
	}
	
}

void UFC_GenericAttachableHandler::DropAll()
{
	for (UFC_GenericAttachablePoint* Slot : Slots)
	{
		DropSlot(Slot);
	}
	
}

void UFC_GenericAttachableHandler::CallOnAttachablesUpdateClient_Implementation()
{
	OnAttachablesUpdateClient.Broadcast();
}

void UFC_GenericAttachableHandler::SpawnAndAttach_Implementation(UClass* Class)
{
	if(GetOwner()->HasAuthority() && IsValid(Class))
	{
		FActorSpawnParameters SpawnParams{};
		AActor* ActorSpawned = GetWorld()->SpawnActor(Class, 0, 0, SpawnParams);
		UFC_GenericAttachable* GenericAttachable = ActorSpawned->FindComponentByClass<UFC_GenericAttachable>();
		if (GenericAttachable && FindEmptySlot(GenericAttachable))
		{
			EquipSlot(GenericAttachable, FindEmptySlot(GenericAttachable));
			return;
		}
		else if(!GenericAttachable)
		{
			UE_LOG(LogTemp, Error, TEXT("Tried to spawn and attach actor that doesn not have an attachable component! (%s)"), *ActorSpawned->GetName())
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("No slot found for actor, aborting! (%s)"), *ActorSpawned->GetName())
		}
		GetWorld()->DestroyActor(ActorSpawned);
	}
	
}

void UFC_GenericAttachableHandler::AutoRefreshSlots_Implementation()
{
	TArray<USceneComponent*> ComponentsFound;
	TArray<UFC_GenericAttachablePoint*> SlotsFound;

	//Get all descendent child components, recursively
	GetOwner()->GetRootComponent()->GetChildrenComponents( true, ComponentsFound);

	//loop through them
	for (USceneComponent* Component : ComponentsFound)
	{
		//Attempt to cast to the scene component using dynamic cast (nullptr if not the right type)
		UFC_GenericAttachablePoint* a;
		a = dynamic_cast<UFC_GenericAttachablePoint*>(Component);
		if(IsValid(a))
		{
			//if our cast succeeds, add to the array
			SlotsFound.AddUnique(a);
			a->OwningHandler = this;
		}
	}
	
	//Set slots to the new value
	Slots = SlotsFound;
	OnRep_Slots();
	
}

void UFC_GenericAttachableHandler::Attach_Implementation(UFC_GenericAttachable* Attachable, UFC_GenericAttachablePoint* SlotToAttach)
{
	if(IsValid(Attachable) && IsValid(SlotToAttach) && IsValid(Attachable->ItemStateManager))
	{
		//UE_LOG(LogTemp, Display, TEXT("Attaching %s to slot"), *Attachable->GetOwner()->GetName());
		Attachable->GetOwner()->SetOwner(GetOwner());
		Attachable->ItemStateManager->Server_ChangeActorState(EActorState::Equipped);
		SlotToAttach->Data.Attached = Attachable;
		SlotToAttach->OnRep_Data();
		Attachable->OwningSlot = SlotToAttach;
	}
}
void UFC_GenericAttachableHandler::DropSlot_Implementation(UFC_GenericAttachablePoint* SlotToDetach)
{
	if(IsValid(SlotToDetach) && IsValid(SlotToDetach->GetAttached()) && IsValid(SlotToDetach->GetAttached()->ItemStateManager))
	{
		SlotToDetach->GetAttachedActor()->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform); // Detach us
		SlotToDetach->GetAttached()->ItemStateManager->Server_ChangeActorState(EActorState::Dropped); // Change state
		SlotToDetach->GetAttached()->OnDetached.Broadcast(); // Call event for BPS
		SlotToDetach->GetAttached()->OwningSlot = nullptr;  // Remove reference to slot
		SlotToDetach->GetAttachedActor()->SetOwner(nullptr);
		SlotToDetach->Data.Attached = nullptr; // Remove reference to actor

		 // Remove owner
		SlotToDetach->OnRep_Data();	// Replicate changes

	}
}

