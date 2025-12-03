// Copyright Frinky 2022


#include "FC_InventoryComponent.h"
#include "FC_ItemComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Net/UnrealNetwork.h"
#include "Serialization/ObjectAndNameAsStringProxyArchive.h"


UFC_InventoryComponent::UFC_InventoryComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
	MaxWeight = 24;
	InventoryName = "Inventory Name";
	IgnoreWeight = false;
}

// Called when the game starts
void UFC_InventoryComponent::BeginPlay()
{
	Super::BeginPlay();

	
}

void UFC_InventoryComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UFC_InventoryComponent, Items);
	DOREPLIFETIME(UFC_InventoryComponent, MaxWeight);
	DOREPLIFETIME(UFC_InventoryComponent, CurrentWeight);
	DOREPLIFETIME(UFC_InventoryComponent, InventoryName);
}

void UFC_InventoryComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	ClearInventory();
	
	Super::EndPlay(EndPlayReason);
}

// Called every frame
void UFC_InventoryComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}


	

bool UFC_InventoryComponent::CanPickup(UFC_ItemComponent* Item)
{
	if(CurrentWeight + Item->Weight <= MaxWeight || IgnoreWeight)
	{
		return true;
	}
	else
	{
		return false;
	}

}

AActor* UFC_InventoryComponent::HasItemOfClass(TSubclassOf<AActor> ItemClass)
{
	 UClass* Class = ItemClass.Get();
	for (UFC_ItemComponent* Item : Items)
	{
		AActor* ItemActor = Item->GetOwner(); 
		if(ItemActor->GetClass()->IsChildOf(Class))
		{
			return ItemActor;
		}
	}

	return nullptr;
	
}

void UFC_InventoryComponent::DropItemMulti_Implementation(UFC_ItemComponent* Item)
{
	//Move the actor to the correct location, this has to be done on everyone, to make the sync nicer
	
	Item->GetOwner()->SetActorLocation(GetOwner()->GetActorLocation() + (GetOwner()->GetActorForwardVector()*150), false, nullptr, ETeleportType::ResetPhysics);
	Item->GetOwner()->SetActorRotation(GetOwner()->GetActorRotation());
	
}

void UFC_InventoryComponent::PickupItemMulti_Implementation(UFC_ItemComponent* Item)
{

	//Item->GetOwner()->SetActorEnableCollision(false);
	//Item->OnItemEnabledChanged.Broadcast(false);
}

void UFC_InventoryComponent::DropItem_Implementation(UFC_ItemComponent* Item)
{
	if(Items.Contains(Item) && IsValid(Item) && IsValid(Item->ItemStateManager))
	{
	
		Item->OwningInventory = nullptr; // we are no longer the owning inventory;
		
		Items.Remove(Item); // remove it from our array of items
		OnRep_ItemsUpdated();

		//Item->GetOwner()->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);

		Item->GetOwner()->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);

		Item->ItemStateManager->Server_ChangeActorState(EActorState::Dropped); //Set the state to the default state (dropped)

		
		
		DropItemMulti(Item); // drop the item for everyone (detach, etc) | this can be multi because by default items are dropped therefore nothing needs to be replicated to new players
		
		Item->ItemDropped.Broadcast(); // broadcast that we dropped the item
		Item->GetOwner()->SetOwner(nullptr); //we should no longer own this item
	}
}

void UFC_InventoryComponent::ForcePickup_Implementation(UFC_ItemComponent* Item)
{
	if(IsValid(Item) && IsValid(Item->ItemStateManager))
	{
		Item->GetOwner()->SetOwner(GetOwner()); //Set the owner to our owner (the container/player)

		if(IsValid(Item->OwningInventory))//Remove from other inventory
		{
			Item->OwningInventory->Items.Remove(Item);
			Item->OwningInventory->OnRep_ItemsUpdated();
		}
		
		Item->ItemStateManager->Server_ChangeActorState(EActorState::Inventory); //Set the state to the default state

		Item->GetOwner()->AttachToActor(GetOwner(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, NAME_None); //attach to us
		
		Item->OwningInventory = this;
		Items.Add(Item);
		Items = Items; //So that the rep_notify runs
		OnRep_ItemsUpdated();
		Item->ItemPickedUp.Broadcast();
		
	}
}

void UFC_InventoryComponent::AttemptPickup(UFC_ItemComponent* Item)
{
	if(CanPickup(Item))
	{
		ForcePickup(Item);
	}
	
}


void UFC_InventoryComponent::SpawnAndPickup_Implementation(TSubclassOf<AActor> ItemClass)
{
	AActor* SpawnedItem = GetWorld()->SpawnActor(ItemClass, &GetOwner()->GetActorTransform());
	UFC_ItemComponent* Item = SpawnedItem->FindComponentByClass<UFC_ItemComponent>();
	if(Item == nullptr)
	{
		GEngine->AddOnScreenDebugMessage(-1, 15, FColor::Red, "Tried to spawn and pickup item that does not have a UFC_ItemComponent");
		SpawnedItem->Destroy();
		return;
	}
	if(!CanPickup(Item))
	{
		GEngine->AddOnScreenDebugMessage(-1, 15, FColor::Red, "Tried to spawn and pickup item that we cannot pickup");
		SpawnedItem->Destroy();
		return;
	}
	
	AttemptPickup(Item);
	
}

float UFC_InventoryComponent::CalculateWeight()
{
	float TempWeight = 0.0;
	for (UFC_ItemComponent* Item : Items)
	{
		if(IsValid(Item))
		{
			TempWeight += Item->Weight;
		}
		
	}
	CurrentWeight = TempWeight;
	return TempWeight;
}

void UFC_InventoryComponent::DropAll_Implementation()
{
	ValidateItems();
	
	for (UFC_ItemComponent* Item : Items)
	{
		DropItem(Item);
	}
}

void UFC_InventoryComponent::ValidateItems_Implementation()
{
	TArray<UFC_ItemComponent*> TempItems;
	
	for (UFC_ItemComponent* Item : Items)
	{
		if(IsValid(Item))
		{
			TempItems.Add(Item);
		}
	}
	
	Items = TempItems;
	OnRep_ItemsUpdated();
	
}



void UFC_InventoryComponent::OnRep_ItemsUpdated()
{
	CalculateWeight();
	InventoryUpdated.Broadcast();
}

//fix this for me
void UFC_InventoryComponent::ClearInventory_Implementation()
{
	TArray<UFC_InventoryComponent*> ItemsSave;
	for (auto Item : ItemsSave)
	{
		if (Item == nullptr || Item->GetOwner() == nullptr || IsValid(Item->GetOwner()))
		{
			continue;
		}
		Item->GetOwner()->Destroy();
	}
	ValidateItems();
}

