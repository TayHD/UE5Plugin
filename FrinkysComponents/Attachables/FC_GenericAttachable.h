// Copyright Frinky 2022

#pragma once

#include "CoreMinimal.h"
#include "FC_GenericAttachableHandler.h"
#include "GameplayTagContainer.h"
#include "Components/ActorComponent.h"
#include "FC_GenericAttachable.generated.h"

class UFC_ItemStateManager;
class UFC_GenericAttachablePoint;
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FGenericAttachableSigniture);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FAttachableItemEnableSigniture, bool, Enabled);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FAttachableUse, FString, Tag);


UCLASS(Blueprintable, ClassGroup=(FRINKYCORE), meta=(BlueprintSpawnableComponent))
class FRINKYSCOMPONENTS_API UFC_GenericAttachable : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UFC_GenericAttachable();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	

public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	//Property's
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category="Settings")
	FGameplayTag SlotTag;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Replicated, Category="Settings")
	FName Name;
	UPROPERTY(BlueprintReadOnly, Replicated)
	UFC_GenericAttachablePoint* OwningSlot;
	
	UPROPERTY(BlueprintReadOnly, Replicated)
	UFC_ItemStateManager* ItemStateManager;


	//Functions
	
	UFUNCTION(BlueprintCallable, Category="FC_Attachable")
	void Detach();

	UFUNCTION(BlueprintCallable, Category="FC_Attachable")
	void UseItem(FString Tag);
	
	UFUNCTION(Server, Reliable)
	void UseItemServer(const FString& Tag);

	UFUNCTION(NetMulticast, Reliable)
	void UseItemMulti(const FString& Tag);

	UFUNCTION(Client, Reliable)
	void UseItemClient(const FString& Tag);
	
	//Dispatchers
	UPROPERTY(BlueprintAssignable)
	FGenericAttachableSigniture OnAttached;

	UPROPERTY(BlueprintAssignable)
	FGenericAttachableSigniture OnDetached;

	UPROPERTY(BlueprintAssignable)
	FAttachableUse ServerUsed;

	UPROPERTY(BlueprintAssignable)
	FAttachableUse MultiUsed;

	UPROPERTY(BlueprintAssignable)
	FAttachableUse ClientUsed;

	
};
