// Copyright Frinky 2022

#pragma once

#include "CoreMinimal.h"
#include "Core.h"
#include "CoreUObject.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "FC_SurfaceDataAsset.generated.h"


USTRUCT(BlueprintType)
struct FSurfaceData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FString SurfaceName;


	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UPhysicalMaterial* Material;


	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	USoundBase* Sound;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UParticleSystem* ParticleSystem;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UNiagaraSystem* NiagaraEmitter;
	
};

/**
 * 
 */
UCLASS(Blueprintable, BlueprintType)
class FRINKYSCOMPONENTS_API UFC_SurfaceDataAsset : public UDataAsset
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere)
	TArray<FSurfaceData> Surfaces;

	UFUNCTION(BlueprintCallable, BlueprintPure, meta=(ToolTip="Checks through the data base for the physical material, if we cant find it we fall back to the FIRST ARRAY ELEMENT"))
	FSurfaceData GetDataFromPhysicalMaterial(UPhysicalMaterial* HitMaterial);

	UFUNCTION(BlueprintCallable, BlueprintPure, meta=(ToolTip="Checks through the data base for the hit data, if we cant find it we fall back to the FIRST ARRAY ELEMENT"))
	FSurfaceData GetDataFromHitResult(FHitResult HitResult);
	
};
