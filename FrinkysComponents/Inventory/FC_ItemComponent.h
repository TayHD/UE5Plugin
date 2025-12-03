// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "FrinkysComponents/General/FC_ItemStateManager.h"
#include "FC_ItemComponent.generated.h"

class UFC_InventoryComponent;
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FItemEnabledSigniture, bool, Enabled);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FItemSigniture);

UCLASS(Blueprintable, ClassGroup=(FRINKYCORE), meta=(BlueprintSpawnableComponent))
class FRINKYSCOMPONENTS_API UFC_ItemComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UFC_ItemComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:	

	//Property's
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category="Settings|Item")
	float Weight;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Settings|Item")
	FString ItemName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(ToolTip="Optional, if you want icons in your inventory implementation"), Category="Settings|Item")
	UTexture2D* Icon;

	UPROPERTY(BlueprintReadOnly, Replicated)
	UFC_ItemStateManager* ItemStateManager;

	UPROPERTY(Replicated, BlueprintReadOnly)
	UFC_InventoryComponent* OwningInventory;

	UPROPERTY(Replicated, EditAnywhere, BlueprintReadOnly, Category="Settings|Item|Use")
	int Uses;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Settings|Item|Use")
	bool CanBeUsed;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Settings|Item|Use")
	bool DestroyOnUse;
	
	//Functions

	UFUNCTION(Server, Reliable, BlueprintCallable)
	void UseItem();

	UFUNCTION(Server, Reliable, BlueprintCallable)
	void Drop();

	//Delegate
	UPROPERTY(BlueprintAssignable)
	FItemEnabledSigniture OnItemEnabledChanged;

	UPROPERTY(BlueprintAssignable)
	FItemSigniture ItemDropped;

	UPROPERTY(BlueprintAssignable)
	FItemSigniture ItemPickedUp;
	
	UPROPERTY(BlueprintAssignable)
	FItemSigniture OnUseItem;

	
	
};
