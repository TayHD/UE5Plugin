// Copyright Frinky 2022


#include "FC_HealthComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/PawnMovementComponent.h"
#include "Net/UnrealNetwork.h"

// Sets default values for this component's properties
UFC_HealthComponent::UFC_HealthComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
	
	MaxHealth = 100;
	Health = 100;
	AutoBindDamageFunction = false;
	

	// ...
}




// Called when the game starts
void UFC_HealthComponent::BeginPlay()
{
	Super::BeginPlay();
	// Bind owners damage function to our RemoveHealth() function
	AActor* Owner = GetOwner();
	if(IsValid(Owner) && AutoBindDamageFunction)
	{
		Owner->OnTakeAnyDamage.AddDynamic(this, &UFC_HealthComponent::OnOwnerTakeAnyDamage);
	}
}

void UFC_HealthComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME_CONDITION_NOTIFY(UFC_HealthComponent, Health, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME(UFC_HealthComponent, MaxHealth);

}



// Gets if the actor is alive
bool UFC_HealthComponent::IsAlive()
{
	return Health > 0;
}

// Gets the health as a percent
float UFC_HealthComponent::HealthPercent()
{
	return Health/MaxHealth;
}

void UFC_HealthComponent::SetMaxHealth_Implementation(float New)
{
	MaxHealth = New;
	OnRep_MaxHealth();
}

void UFC_HealthComponent::RemoveHealth(float Change)
{
	ChangeHealth(Change*-1);
	CallHitServer();
	CallHitMulti();
}

void UFC_HealthComponent::OnOwnerTakeAnyDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, AController* InstigatedBy, AActor* DamageCauser)
{
	RemoveHealth(Damage);
}


bool UFC_HealthComponent::KillCharacter()
{
	ACharacter* OwningCharacter = Cast<ACharacter>(GetOwner());
	if(IsValid(OwningCharacter))
	{
		//Disable movement
		OwningCharacter->GetMovementComponent()->SetComponentTickEnabled(false);
		OwningCharacter->GetMovementComponent()->SetActive(false);
		//Disable actor tick
		OwningCharacter->SetActorTickEnabled(false);
		//Destroy capsule
		OwningCharacter->GetCapsuleComponent()->DestroyComponent();
		//Disable Input
		OwningCharacter->DisableInput(Cast<APlayerController>(OwningCharacter->GetController()));
		return true;
		
	}
	else
	{
		return false;
	}
	
}

void UFC_HealthComponent::OnRep_Health()
{
	if(Health <= 0 && !HasCalledDeathEvents)
	{
		DeathServer.Broadcast();
		CallDeathMulti();	
		CallDeathClient();

		GetOwner()->SetLifeSpan(ActorLifespanPostDeath); // Set the actors lifespan to whatever is set in the editor
		
		HasCalledDeathEvents = true;
	}
}

void UFC_HealthComponent::OnRep_MaxHealth()
{
	ChangeHealth(0.0);
}


void UFC_HealthComponent::CallDeathMulti_Implementation()
{
	if(CallKillCharacterFunction)
	{
		KillCharacter();
	}
	DeathMulti.Broadcast();
	
}
void UFC_HealthComponent::CallDeathClient_Implementation()
{
	DeathClient.Broadcast();
}
void UFC_HealthComponent::CallHitServer_Implementation()
{
	HitServer.Broadcast();
	CallHitMulti();
}
void UFC_HealthComponent::CallHitMulti_Implementation()
{
	HitMulti.Broadcast();
}
void UFC_HealthComponent::CallHitClient_Implementation()
{
	HitClient.Broadcast();
}


void UFC_HealthComponent::ChangeHealth_Implementation(float Change)
{
	if(Health > 0 || Health + Change > 0)
	{
		Health = FMath::Clamp(Health+Change, 0.0f, MaxHealth);
		OnRep_Health();
	}
	
	
}





