// Copyright Frinky 2022

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Components/SceneComponent.h"
#include "FC_GenericAttachablePoint.generated.h"

class UFC_GenericAttachableHandler;
class UFC_GenericAttachable;
USTRUCT(BlueprintType)


struct FAttachableData
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Settings")
	FName SlotName;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Settings")
	UFC_GenericAttachable* Attached;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Settings")
	FGameplayTag Tag;

	FAttachableData()
	{
		SlotName=FName("Slot");
		Attached = nullptr;
		Tag = FGameplayTag::EmptyTag;
		
	}
	
};

UCLASS(Blueprintable, ClassGroup=(FRINKYCORE), meta=(BlueprintSpawnableComponent))
class FRINKYSCOMPONENTS_API UFC_GenericAttachablePoint : public USceneComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UFC_GenericAttachablePoint();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
	//Property's

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing=OnRep_Data, EditAnywhere)
	FAttachableData Data;

	UPROPERTY(BlueprintReadOnly, Replicated)
	UFC_GenericAttachableHandler* OwningHandler;

	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(ToolTip="Optional, if you want icons in your inventory implementation"), Category="Settings")
	UTexture2D* Icon;

	//Functions
	UFUNCTION()
	virtual void OnRep_Data();

	UFUNCTION(BlueprintCallable)
	void DropAttached();
	
	
	UFUNCTION(BlueprintPure, BlueprintCallable)
	UFC_GenericAttachable* GetAttached();
	
	UFUNCTION(BlueprintPure, BlueprintCallable)
	AActor* GetAttachedActor();

	UFUNCTION(BlueprintPure, BlueprintCallable)
	const bool HasAttachment();

	
	
};
