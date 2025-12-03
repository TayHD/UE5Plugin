// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "FC_ReplicatedRagdoll.generated.h"


UCLASS(ClassGroup=(FRINKYCORE), meta=(BlueprintSpawnableComponent))
class FRINKYSCOMPONENTS_API UFC_ReplicatedRagdoll : public UActorComponent
{
	GENERATED_BODY()

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:
	// Sets default values for this component's properties
	UFC_ReplicatedRagdoll();
	
	virtual void BeginPlay() override;
	
	void DrawDebug();

	////////// VARIABLES //////////

	UPROPERTY(BlueprintReadOnly, Category = "Ragdoll")
	USkeletalMeshComponent* MeshToSync;



	UPROPERTY(BlueprintReadOnly, Category = "Ragdoll", Replicated)
	FTransform RagdollPositionServer;

	UPROPERTY(BlueprintReadOnly, Category = "Ragdoll")
	APawn* OwningPawn;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Ragdoll")
	bool bDrawDebug = false;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Ragdoll")
	FName BoneToSync = "pelvis";

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Ragdoll")
	bool bRotationSync = true;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Ragdoll")
	float Force = 10000;
	


	///////// FUNCTIONS //////////
	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintCallable, Category = "Ragdoll")
	void InitializeRagdoll(USkeletalMeshComponent* Mesh);
	

	UFUNCTION(Category = "Ragdoll")
	FTransform GetQuantizedRagdollTransform();

	UFUNCTION(Category = "Ragdoll")
	FTransform GetRagdollTransform();

    UFUNCTION(BlueprintPure, Category = "Ragdoll")
    bool isSimulating();

	UFUNCTION(Category = "Ragdoll")
	void ApplyRagdollTransform();
	

	// RPCS

	UFUNCTION(Server, Unreliable, Category = "Ragdoll")
	void ServerSetRagdollTransform(FTransform Transform);

   


protected:

	
	
};
