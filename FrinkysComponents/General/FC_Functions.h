// Copyright Frinky 2022

#pragma once

#include "CoreMinimal.h"
#include "FC_GunComponent.h"
#include "FC_HealthComponent.h"
#include "FC_InteractionManager.h"
#include "FrinkysComponents/Aim/FC_AimHandler.h"
#include "FrinkysComponents/Armour/FC_ArmourComponent.h"
#include "FrinkysComponents/Armour/FC_ArmourHandlerComponent.h"
#include "FrinkysComponents/Attachables/FC_GenericAttachable.h"
#include "FrinkysComponents/Equipment/FC_EquipmentComponent.h"
#include "FrinkysComponents/Equipment/FC_EquipmentHandlerComponent.h"
#include "FrinkysComponents/Seats/FC_Seat.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "FC_Functions.generated.h"

class UFC_InventoryComponent;
class UFC_ItemComponent;
/**
 * 
 */
UCLASS()
class FRINKYSCOMPONENTS_API UFC_Functions : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	

public:
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="FRINKYCORE")
		static UFC_HealthComponent* GetHealthComponent(AActor* Actor);
	
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="FRINKYCORE")
		static UFC_ItemComponent* GetItemComponent(AActor* Actor);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category="FRINKYCORE")
		static UFC_InventoryComponent* GetInventoryComponent(AActor* Actor);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category="FRINKYCORE")
		static UFC_GenericAttachable* GetGenericAttachable(AActor* Actor);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category="FRINKYCORE")
		static UFC_GenericAttachableHandler* GetGenericAttachableHandler(AActor* Actor);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category="FRINKYCORE")
		static UFC_EquipmentComponent* GetEquipmentComponent(AActor* Actor);
	
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="FRINKYCORE")
		static UFC_EquipmentHandlerComponent* GetEquipmentHandler(AActor* Actor);
	
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="FRINKYCORE")
		static UFC_GunComponent* GetGunComponent(AActor* Actor);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category="FRINKYCORE")
		static UFC_ArmourComponent* GetArmourComponent(AActor* Actor);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category="FRINKYCORE")
		static UFC_ArmourHandlerComponent* GetArmourHandlerComponent(AActor* Actor);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category="FRINKYCORE")
		static UFC_InteractionManager* GetInteractionManagerComponent(AActor* Actor);


	UFUNCTION(BlueprintCallable, BlueprintPure, Category="FRINKYCORE")
		static UFC_Seat* GetActorSeat(AActor* Actor);
	
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="FRINKYCORE")
		static UFC_EquipmentComponent* GetHeld(AActor* Actor);
	
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="FRINKYCORE")
		static AActor* GetHeldActor(AActor* Actor);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category="FRINKYCORE")
		static UFC_GunComponent* GetHeldGunComponent(AActor* Actor);
	

	UFUNCTION(BlueprintCallable, BlueprintPure, Category="FRINKYCORE")
		static UFC_AimHandler* GetAimHandler(AActor* Actor);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category="FRINKYCORE")
		static UFC_Barrel* GetBarrel(AActor* Actor);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category="FRINKYCORE")
		static float GetHealth(AActor* Actor);

	UFUNCTION(BlueprintCallable, BlueprintPure, meta = (BlueprintThreadSafe), Category="FRINKYCORE")
		static float Invert(float in);

	UFUNCTION(BlueprintCallable, BlueprintPure, meta = (BlueprintThreadSafe), Category="FRINKYCORE")
		static float GetDirectionToTargetDotProduct(FVector Origin, FVector Direction, FVector Target);

	UFUNCTION(BlueprintCallable, Category="FRINKYCORE", meta=(WorldContext="WorldContextObject", DeterminesOutputType="ActorClass", DynamicOutputParam="OutActors"))
		static void GetNumberOfAliveOfClass(const UObject* WorldContextObject, TSubclassOf<AActor> Class, TArray<AActor*>& OutActors);

	UFUNCTION(BlueprintCallable, Category="FRINKYCORE")
	static int32 GetClosestRotationIndex(FRotator TargetRotation, const TArray<FRotator>& RotationsArray);
	
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="FRINKYCORE")
	static float GetAngularDistanceBetweenRotators(FRotator Rotator1, FRotator Rotator2);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category="FRINKYCORE")
	static FVector ClosestPointOnLine(const FVector& LineStart, const FVector& LineEnd, const FVector& Point);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category="FRINKYCORE")
	static bool IsLinePointingAtPoint(const FVector& LineStart, const FVector& LineEnd, const FVector& Point);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category="FRINKYCORE", meta = (BlueprintThreadSafe))
	static float OneMinus(float Value);
};
