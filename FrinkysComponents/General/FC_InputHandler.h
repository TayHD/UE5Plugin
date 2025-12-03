// Copyright Frinky 2022

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "FC_InputHandler.generated.h"


UCLASS(Blueprintable, ClassGroup=(FRINKYCORE), meta=(BlueprintSpawnableComponent))
class FRINKYSCOMPONENTS_API UFC_InputHandler : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UFC_InputHandler();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

private:
	float ForwardInput;
	float RightInput;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	
	//Functions
	UFUNCTION(BlueprintCallable, Category="Input")
	FVector2D HandleMouseInput(FVector MouseInput);

	UFUNCTION(BlueprintCallable, Category="Input")
	void HandleRightInput(float Input);

	UFUNCTION(BlueprintCallable, Category="Input")
	void HandleForwardInput(float Input);
	
	//Property's
	UPROPERTY(BlueprintReadOnly)
	APawn* Pawn;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Settings")
	float Sensitivity;
	
	UPROPERTY(BlueprintReadOnly)
	FVector2D MouseDirection;
	
	
	
	

		
};
