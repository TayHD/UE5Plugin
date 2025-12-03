// Copyright Frinky 2022

#pragma once

#include "CoreMinimal.h"
#include "FC_AnimInstance.h"
#include "Components/ActorComponent.h"
#include "FC_AnimComponent.generated.h"


UCLASS(ClassGroup=(FRINKYCORE), meta=(BlueprintSpawnableComponent, ToolTip="This is a component that helps replicate certain values on the anim instance, if it doesn't exist, we'll fall back on worse methods"))
class FRINKYSCOMPONENTS_API UFC_AnimComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UFC_AnimComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	virtual void BeginPlay() override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(Server, Reliable, Category = "Anim")
	void Server_UpdateAimOffset(FRotator InAimOffset, FRotator InRelativeAimOffset);

	UFUNCTION(NetMulticast, Unreliable, Category = "Anim")
	void MultiUpdateAimOffset(FRotator InControlRotation, FRotator InRelativeControlRotation);

	UPROPERTY(BlueprintReadOnly, Category = "Anim")
	FRotator ControlRotation;

	UPROPERTY(BlueprintReadOnly, Category = "Anim")
	FRotator RelativeControlRotation;

	UPROPERTY(BlueprintReadOnly, Category = "Anim")
	FRotator AimDifference;

	UPROPERTY(BlueprintReadOnly, Category = "Anim")
	APawn* OwningPawn;
	

	
};