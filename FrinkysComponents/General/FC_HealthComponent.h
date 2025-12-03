// Copyright Frinky 2022

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "FC_HealthComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FHealthComponentSigniture);

UCLASS(Blueprintable, ClassGroup=(FRINKYCORE), meta=(BlueprintSpawnableComponent))
class FRINKYSCOMPONENTS_API UFC_HealthComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UFC_HealthComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:	

	
	//Property's

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (ToolTip = "If true, the health component will automatically handle the actors death, if false, you will need to call the KillCharacter function yourself"))
	bool CallKillCharacterFunction = true;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (ToolTip = "If true, the health component will bind to the owners OnTakeAnyDamage function, if false, you will need to call the OnOwnerTakeAnyDamage function yourself"))
	bool AutoBindDamageFunction = false;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (ToolTip = "If greator then 0, the health component will automatically set the actor to despawn after htre set time, if 0, actor wont despawn"))
	float ActorLifespanPostDeath = 0.0;

	UPROPERTY()
	bool HasCalledDeathEvents = false;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, ReplicatedUsing=OnRep_MaxHealth)
	float MaxHealth; //Duh, you know what this is
	
	UPROPERTY(BlueprintReadOnly, EditAnywhere, ReplicatedUsing = OnRep_Health)
	float Health; //Think about it real hard

	//Functions
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Health", meta = (ToolTip = "boolean determining whether the player is alive"))
	bool IsAlive();
	

	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Health", meta = (ToolTip = "Get the health as a percent (0.0-1.0"))
	float HealthPercent();
	
	UFUNCTION(BlueprintCallable, Server, Reliable, Category="Health", meta = (ToolTip = "Float is ADDED to the players health, health cannot go above max, or below 0"))
	void ChangeHealth(float Change);

	UFUNCTION(BlueprintCallable, Server, Reliable, Category="Health", meta = (ToolTip = "Float is ADDED to the players health, health cannot go above max, or below 0"))
	void SetMaxHealth(float New);

	UFUNCTION(BlueprintCallable, Category="Health", meta = (ToolTip = "Float is REMOVED to the players health, health cannot go above max, or below 0"))
	void RemoveHealth(float Change);

	UFUNCTION()
	void OnOwnerTakeAnyDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, AController* InstigatedBy, AActor* DamageCauser);
	

	UFUNCTION(BlueprintCallable, Category="Health", meta=(ToolTip = "generally disable the character, ONLY WORKS ON A CHARACTER CLASS"))
	bool KillCharacter();

	UFUNCTION()
	void OnRep_Health();
	
	UFUNCTION()
	void OnRep_MaxHealth();
	
	
	UFUNCTION(NetMulticast, Reliable)
		void CallDeathMulti();
	
	UFUNCTION(Client, Reliable)
		void CallDeathClient();
	
	UFUNCTION(Server, Reliable)
		void CallHitServer();
	
	UFUNCTION(NetMulticast, Reliable)
		void CallHitMulti();
	
	UFUNCTION(Client, Reliable)
		void CallHitClient();

	
	//Delegates
		//  Death Delegates
	UPROPERTY(BlueprintAssignable)
		FHealthComponentSigniture DeathServer;
	UPROPERTY(BlueprintAssignable)
		FHealthComponentSigniture DeathClient;
	UPROPERTY(BlueprintAssignable)
		FHealthComponentSigniture DeathMulti;
	
		//  Hit Delegates
	UPROPERTY(BlueprintAssignable)
		FHealthComponentSigniture HitServer;
	UPROPERTY(BlueprintAssignable)
		FHealthComponentSigniture HitClient;
	UPROPERTY(BlueprintAssignable)
		FHealthComponentSigniture HitMulti;
};
