// Copyright Frinky 2022

#pragma once

#include "CoreMinimal.h"
#include "FC_EquipmentComponent.h"
#include "FC_EquipmentPointComponent.h"
#include "FrinkysComponents/Attachables/FC_GenericAttachableHandler.h"
#include "FC_EquipmentHandlerComponent.generated.h"

/**
 * 
 */

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FHeldItemSigniture);

UCLASS(Blueprintable, ClassGroup=(FRINKYCORE), meta=(BlueprintSpawnableComponent))
class FRINKYSCOMPONENTS_API UFC_EquipmentHandlerComponent : public UFC_GenericAttachableHandler
{
	GENERATED_BODY()

public:

	UFC_EquipmentHandlerComponent();
	
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual void AutoRefreshSlots() override;

	virtual void EquipSlot(UFC_GenericAttachable* Attachable, UFC_GenericAttachablePoint* SlotToAttach) override;

	virtual void BeginPlay() override;

	UPROPERTY(ReplicatedUsing=OnRep_Held, BlueprintReadOnly)
	UFC_EquipmentPointComponent* Held;

	UPROPERTY(Replicated, BlueprintReadOnly)
	UFC_EquipmentPointComponent* LastHeld;



	UFUNCTION(Server, Reliable, BlueprintCallable)
	void HoldSlot(UFC_EquipmentPointComponent* Slot);

	UFUNCTION(BlueprintCallable, BlueprintPure)
	AActor* GetHeldActor();

	UFUNCTION(BlueprintCallable, BlueprintPure)
	UFC_EquipmentComponent* GetHeld();

	UFUNCTION()
	void UpdateHeld();
	

	UFUNCTION()
	void OnRep_Held();
	
	//Delegate
	UPROPERTY(BlueprintAssignable)
	FHeldItemSigniture OnHeldItemChanged;
	
};