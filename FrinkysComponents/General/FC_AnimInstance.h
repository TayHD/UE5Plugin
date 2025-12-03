// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "FC_HealthComponent.h"
#include "Animation/AnimInstance.h"
#include "FC_AnimInstance.generated.h"

/**
 * 
 */
UCLASS()
class FRINKYSCOMPONENTS_API UFC_AnimInstance : public UAnimInstance
{
	
public:
	// Sets default values for this component's properties
	UFC_AnimInstance();
protected:
	
	GENERATED_BODY()

	virtual void NativeBeginPlay() override;

	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:
	UPROPERTY(BlueprintReadOnly, Category="FCore|Velocity")
	float ForwardVelocity;

	UPROPERTY(BlueprintReadOnly, Category="FCore|Velocity")
	float Speed;

	UPROPERTY(BlueprintReadOnly, Category="FCore|Velocity")
	float RightVelocity;

	UPROPERTY(BlueprintReadOnly, Category="FCore|Info")
	bool bIsFalling;

	UPROPERTY(BlueprintReadOnly, Category="FCore|Info")
	bool isSwimming;
	
	UPROPERTY(BlueprintReadOnly, Category="FCore|Info")
    bool bIsCrouching;

	UPROPERTY(BlueprintReadOnly, Category="FCore|Info")
	bool bIsLocallyControlled = false;
	
	UPROPERTY(BlueprintReadOnly, Category="FCore|Info")
	bool bIsAlive = true;

	UPROPERTY(BlueprintReadOnly, Category="FCore|Info")
	bool bIsMontagePlaying = false;

	UPROPERTY(BlueprintReadOnly, Category="FCore|Info")
	float Health = 100;

	UPROPERTY(BlueprintReadOnly, Category="FCore|Info")
	float HealthPercent = 1;

	UPROPERTY(BlueprintReadOnly, Category="FCore|Look")
	FRotator ControlRotation;

	UPROPERTY(BlueprintReadOnly, Category="FCore|Look")
	FRotator RelativeControlRotation;
	

	UPROPERTY(BlueprintReadOnly, Category="FCore|Sway")
	FVector2D LookDelta;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="FCore|Sway")
	float LookDeltaInterp;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="FCore|Sway")
	float LookDeltaHorizontalMultiplier;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="FCore|Sway")
	float LookDeltaVerticalMultiplier;
	
	UPROPERTY(BlueprintReadOnly, Category="FCore|Velocity")
	float InterpedSpeedRatio;

	UPROPERTY(BlueprintReadOnly, Category="FCore|Sway")
	float AimingNoise = 0.0f;
	
	
	UPROPERTY(BlueprintReadOnly, Category="FCore|Velocity")
	FVector Velocity;

	UPROPERTY(BlueprintReadOnly, Category="FCore|References")
	APawn* Pawn;

	UPROPERTY(BlueprintReadOnly, Category="FCore|References")
	ACharacter* Character;

	UPROPERTY(BlueprintReadOnly, Category="FCore|References")
	UFC_HealthComponent* HealthComponent;

	// Functions

	UFUNCTION(Server, Reliable)
	void ServerSetControlRotation(FRotator Rotation);

	UFUNCTION(NetMulticast, Reliable)
	void MultiSetControlRotation(FRotator Rotation);

	UFUNCTION(BlueprintCallable)
	void CalculateBlocking(float DeltaTime, float Length, float Width, FTransform CurrentBlockingTransform, bool& DisableAimLowered, FTransform& OutBlockingTransform);
	
	UFUNCTION(BlueprintCallable)
	void ProceduralADSBreathing(float DeltaTime, float TimeSeconds, float NoiseInterpSpeed, float MovementMultiplier, float IdleValue, FVector Location, FRotator Rotation, FVector& OutLocation, FRotator& OutRotation);
};
