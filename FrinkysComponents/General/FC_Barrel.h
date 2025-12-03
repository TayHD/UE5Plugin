// Copyright Frinky 2022

#pragma once

#include "CoreMinimal.h"
#include "NiagaraSystem.h"
#include "Components/ActorComponent.h"
#include "Components/ArrowComponent.h"
#include "FC_Barrel.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE(FBarrelDelgate);

UCLASS(ClassGroup=(FRINKYCORE), meta=(BlueprintSpawnableComponent), Blueprintable)
class FRINKYSCOMPONENTS_API UFC_Barrel : public UArrowComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UFC_Barrel();

protected:
	


public:
	

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Barrel Info")
	bool bIsSuppressor = false;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Barrel Info")
	UParticleSystem* ParticleSystem;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Barrel Info")
	UNiagaraSystem* NiagaraEmitter;

	//FUNCTIONS
	
	UFUNCTION()
	void Shoot();

	UPROPERTY(BlueprintAssignable)
	FBarrelDelgate OnShoot;

	

};
