// Copyright Frinky 2022

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Components/ActorComponent.h"
#include "FC_GenericAttachableHandler.generated.h"

class UFC_GenericAttachablePoint;
class UFC_GenericAttachable;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FGenericHandlerSigniture);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FGenericHandlerAttachedSigniture, UFC_GenericAttachable*, Attached);


UCLASS(Blueprintable, ClassGroup=(FRINKYCORE), meta=(BlueprintSpawnableComponent))
class FRINKYSCOMPONENTS_API UFC_GenericAttachableHandler : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UFC_GenericAttachableHandler();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;


private:


	
public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;


		//Property's
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(ToolTip="Automatically updates the list of slots recursively on begin play"))
	bool AutoGetSlots;

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing=OnRep_Slots, meta=(ToolTip="Current slots"))
	TArray<UFC_GenericAttachablePoint*> Slots;
	

	
		//Functions
	UFUNCTION()
	virtual void OnRep_Slots();

	UFUNCTION(BlueprintCallable, BlueprintPure, meta=(ToolTip="Finds an empty and compatible slot"))
	virtual UFC_GenericAttachablePoint* FindEmptySlot(UFC_GenericAttachable* Attachable);

	UFUNCTION(BlueprintCallable, BlueprintPure, meta=(ToolTip="Finds any slot, regardless of if theres something attached"))
	virtual UFC_GenericAttachablePoint* FindSlot(UFC_GenericAttachable* Attachable);

	UFUNCTION(BlueprintCallable, BlueprintPure, meta=(ToolTip="Finds any slot, regardless of if theres something attached"))
	virtual UFC_GenericAttachablePoint* FindSlotByTag(FGameplayTag Tag);

	UFUNCTION(BlueprintCallable, BlueprintPure, meta=(ToolTip="Does this attachment fit in the attachment slot"))
	virtual bool DoesFitSlot(UFC_GenericAttachable* Attachable, UFC_GenericAttachablePoint* SlotToCheck);

	UFUNCTION(BlueprintCallable, Server, Reliable, meta=(ToolTip="Refreshes list of slots by automatically recursively finding them, if auto-get slots is ticked this is called automatiicaly on begin play"))
	virtual void AutoRefreshSlots();

	UFUNCTION(Server, Reliable)
	virtual void Attach(UFC_GenericAttachable* Attachable, UFC_GenericAttachablePoint* SlotToAttach);

	UFUNCTION(BlueprintCallable, Server, Reliable, meta=(ToolTip="Attaches the attachable to the slot, replicated"))
	virtual void EquipSlot(UFC_GenericAttachable* Attachable, UFC_GenericAttachablePoint* SlotToAttach);


	UFUNCTION(BlueprintCallable, Server, Reliable, meta=(ToolTip="detaches anything connected to the slot provided"))
	virtual void DropSlot(UFC_GenericAttachablePoint* SlotToDetach);
	
	UFUNCTION(BlueprintCallable, meta=(ToolTip="detaches every slots item"))
	virtual void DropAll();

	UFUNCTION(BlueprintCallable, Server, Reliable, meta=(ToolTip="spawns item, finds relevant slot, attaches"))
	virtual void SpawnAndAttach(UClass* Class);

	UFUNCTION(Client, Reliable)
	void CallOnAttachablesUpdateClient();
	

		//Dispatchers
	UPROPERTY(BlueprintAssignable, meta=(ToolTip="When the slots array changes"))
	FGenericHandlerSigniture OnSlotsUpdated;

	UPROPERTY(BlueprintAssignable, meta=(ToolTip="When something detaches from any slot"))
	FGenericHandlerSigniture OnDetach;

	UPROPERTY(BlueprintAssignable, meta=(ToolTip="When something attaches to any slot"))
	FGenericHandlerAttachedSigniture OnAttach;

	UPROPERTY(BlueprintAssignable, meta=(ToolTip="Called when anything attaches or detaches at all"))
	FGenericHandlerSigniture OnAttachablesUpdate;

	UPROPERTY(BlueprintAssignable, meta=(ToolTip="Called when anything attaches or detaches at all"))
	FGenericHandlerSigniture OnAttachablesUpdateClient;
	
		
};
