// Copyright Frinky 2022

#pragma once

#include "CoreMinimal.h"
#include "FC_ItemComponent.h"

#include "Components/ActorComponent.h"
#include "FC_InventoryComponent.generated.h"



class UFC_ItemComponent;
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FInventorySignature);

UCLASS(Blueprintable, ClassGroup=(FRINKYCORE), meta=(BlueprintSpawnableComponent))
class FRINKYSCOMPONENTS_API UFC_InventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UFC_InventoryComponent();
	

protected:
	UFUNCTION()
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;



public:
	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	//List of items
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing=OnRep_ItemsUpdated)
	TArray<UFC_ItemComponent*> Items;

	//Max weight the inventory can hold
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Replicated, Category="Settings")
	float MaxWeight;

	UPROPERTY(BlueprintReadOnly, Replicated, Category="Settings")
	float CurrentWeight;

	UPROPERTY(BlueprintReadWrite, Replicated, EditAnywhere, Category="Settings")
	FString InventoryName;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category="Settings")
	bool IgnoreWeight;
	

	//Functions
	UFUNCTION()
	virtual void OnRep_ItemsUpdated();
	
	

	UFUNCTION(BlueprintCallable, Server, Reliable)
	void ClearInventory();
	

	UFUNCTION(BlueprintCallable, BlueprintPure, meta=(ToolTip="Checks for weight limit, and any other restrictions"))
	bool CanPickup(UFC_ItemComponent* Item);

	UFUNCTION(BlueprintCallable, BlueprintPure, meta=(ToolTip="Checks for weight limit, and any other restrictions"))
	AActor* HasItemOfClass(TSubclassOf<AActor> ItemClass);

	UFUNCTION(BlueprintCallable, Server, Reliable, meta=(ToolTip="Probably dont use this, it bypasses the weight limit, but if you'd like to force the player to pick something"))
	void ForcePickup(UFC_ItemComponent* Item);

	UFUNCTION(BlueprintCallable, meta=(ToolTip="Use this function to pickup items"))
	void AttemptPickup(UFC_ItemComponent* Item);

	UFUNCTION(BlueprintCallable, Server, Reliable, meta=(ToolTip="Use this function to drop an item, if you own it"))
	void DropItem(UFC_ItemComponent* Item);

	//drop item for everyone
	UFUNCTION(Reliable, NetMulticast)
	void DropItemMulti(UFC_ItemComponent* Item);

	//pickup item for everyone
	UFUNCTION(Reliable, NetMulticast)
	void PickupItemMulti(UFC_ItemComponent* Item);

	UFUNCTION(Server, Reliable, BlueprintCallable, meta=(ToolTip="Spawns an item and picks it up"))
	void SpawnAndPickup(TSubclassOf<AActor> ItemClass);
	
	

	UFUNCTION(BlueprintCallable, BlueprintPure)
	float CalculateWeight();

	UFUNCTION(BlueprintCallable, Server, Reliable, meta=(ToolTip="validates item list, removes any none valid items"))
	void ValidateItems();

	UFUNCTION(BlueprintCallable, Server, Reliable, meta=(ToolTip="Take a wild guess, it drops every item in the inventory"))
	void DropAll();

	//Delegate
	UPROPERTY(BlueprintAssignable, BlueprintCallable)
	FInventorySignature InventoryUpdated;

	//Client Inventory Updated, to allow for an inventory to locally update something (ie an inventory)
	UPROPERTY(BlueprintAssignable, BlueprintCallable)
	FInventorySignature ClientInventoryUpdated;

	
		
};
