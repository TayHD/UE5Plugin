// Copyright Frinky 2022

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Components/BoxComponent.h"
#include "FC_InteractionPoint.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FInteractSignature, APawn*, Interactor);


UCLASS(Blueprintable, ClassGroup=(FRINKYCORE), meta=(BlueprintSpawnableComponent))
class FRINKYSCOMPONENTS_API UFC_InteractionPoint : public UBoxComponent
{
	GENERATED_BODY()

public:
	UFC_InteractionPoint();

protected:

	virtual void BeginPlay() override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;

	//UPropertys
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Replicated, Category="Settings|Interaction")
	FString InteractionName;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Replicated, Category="Settings|Interaction")
	bool isEnabled = true;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Replicated, Category="Settings|Interaction")
	bool StopInteractionOnInteractComplete = false;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Replicated, Category="Settings|Interaction", meta=(ToolTip="Time in seconds to interact with this point (if using the interaction manager)"))
	float InteractionTime = 0.0;


	UFUNCTION(BlueprintCallable)
	void Interact(APawn* Interactor);

	UFUNCTION(BlueprintCallable, Server, Reliable)
	void ServerInteract(APawn* Interactor);

	UFUNCTION(BlueprintCallable)
	void StopInteract(APawn* Interactor);

	UFUNCTION(BlueprintCallable, Server, Reliable)
	void ServerStopInteract(APawn* Interactor);
	
	UFUNCTION(BlueprintCallable, NetMulticast, Reliable)
	void SetEnabled(bool NewEnabled);
	
	UFUNCTION(BlueprintPure)
	void GetInteractionInfo(bool& enabled, FString& name);
	
	UPROPERTY(BlueprintAssignable, BlueprintCallable) 
	FInteractSignature OnInteract;

	UPROPERTY(BlueprintAssignable, BlueprintCallable) 
	FInteractSignature OnInteractServer;


	UPROPERTY(BlueprintAssignable, BlueprintCallable, meta=(ToolTip="Called when the interaction is stopped (only if using interaction manager)"))
	FInteractSignature OnStopInteract;

	UPROPERTY(BlueprintAssignable, BlueprintCallable, meta=(ToolTip="Called when the interaction (server) is stopped (only if using interaction manager)")) 
	FInteractSignature OnStopInteractServer;
	


	

	
};
