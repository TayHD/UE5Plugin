// Copyright Frinky 2022

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "FC_ItemStateManager.generated.h"


//UENUM with all the different states for an item

UENUM(BlueprintType)
enum class EActorState : uint8
{
	Equipped,
	Inventory,
	Dropped
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FActorStateSigniture, EActorState, NewState);




UCLASS(ClassGroup=(FRINKYCORE), meta=(BlueprintSpawnableComponent))
class FRINKYSCOMPONENTS_API UFC_ItemStateManager : public UActorComponent
{
	GENERATED_BODY()

	//get replicated lifetime props
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:
	// Sets default values for this component's properties
	UFC_ItemStateManager();

protected:
	// Called when the game starts
	//virtual void BeginPlay() override;

public:
	
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing=OnRep_ActorStateUpdated)
	EActorState ActorState;

	UFUNCTION()
	void OnRep_ActorStateUpdated();

	//server function for changing actor state
	UFUNCTION(Server, Reliable)
	void Server_ChangeActorState(EActorState NewState);



	//Dispatchers
	UPROPERTY(BlueprintAssignable)
	FActorStateSigniture OnActorStateUpdated;
	
};
