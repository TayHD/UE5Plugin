// Copyright Frinky 2022


#include "FC_Functions.h"

#include "EngineUtils.h"
#include "FC_GunComponent.h"
#include "FrinkysComponents/Inventory/FC_InventoryComponent.h"
#include "FrinkysComponents/Inventory/FC_ItemComponent.h"
#include "Kismet/GameplayStatics.h"


UFC_HealthComponent* UFC_Functions::GetHealthComponent(AActor* Actor)
{
	if(Actor)
	{
		return Actor->FindComponentByClass<UFC_HealthComponent>();
	}
	return nullptr;}

UFC_ItemComponent* UFC_Functions::GetItemComponent(AActor* Actor)
{
	if(Actor)
	{
		return Actor->FindComponentByClass<UFC_ItemComponent>();
	}
	return nullptr;
}

UFC_InventoryComponent* UFC_Functions::GetInventoryComponent(AActor* Actor)
{
	if(Actor)
	{
		return Actor->FindComponentByClass<UFC_InventoryComponent>();
	}
	return nullptr;
}

UFC_GenericAttachable* UFC_Functions::GetGenericAttachable(AActor* Actor)
{
	if(Actor)
	{
		return Actor->FindComponentByClass<UFC_GenericAttachable>();
	}
	return nullptr;
}

UFC_GenericAttachableHandler* UFC_Functions::GetGenericAttachableHandler(AActor* Actor)
{
	if(Actor)
	{
		return Actor->FindComponentByClass<UFC_GenericAttachableHandler>();
	}
	return nullptr;
}

UFC_EquipmentComponent* UFC_Functions::GetEquipmentComponent(AActor* Actor)
{
	if(Actor)
	{
		return Actor->FindComponentByClass<UFC_EquipmentComponent>();
	}
	return nullptr;
}

UFC_EquipmentHandlerComponent* UFC_Functions::GetEquipmentHandler(AActor* Actor)
{
	if(Actor)
	{
		return Actor->FindComponentByClass<UFC_EquipmentHandlerComponent>();
	}
	return nullptr;
}
UFC_GunComponent* UFC_Functions::GetGunComponent(AActor* Actor)
{
	if(Actor)
	{
		return Actor->FindComponentByClass<UFC_GunComponent>();
	}
	return nullptr;
}

UFC_ArmourComponent* UFC_Functions::GetArmourComponent(AActor* Actor)
{
	if(Actor)
	{
		return Actor->FindComponentByClass<UFC_ArmourComponent>();
	}
	return nullptr;
}

UFC_ArmourHandlerComponent* UFC_Functions::GetArmourHandlerComponent(AActor* Actor)
{
	if(Actor)
	{
		return Actor->FindComponentByClass<UFC_ArmourHandlerComponent>();
	}
	return nullptr;
}

UFC_InteractionManager* UFC_Functions::GetInteractionManagerComponent(AActor* Actor)
{
	if(Actor)
	{
		return Actor->FindComponentByClass<UFC_InteractionManager>();
	}
	return nullptr;
}

UFC_Seat* UFC_Functions::GetActorSeat(AActor* Actor)
{
	if(!IsValid(Actor)) { return nullptr; }
	if(!IsValid(Actor->GetRootComponent())) { return nullptr; }
	
	UFC_Seat* Seat = dynamic_cast<UFC_Seat*>(Actor->GetRootComponent()->GetAttachParent());
	if(Seat)
	{
		return Seat;
		
	}
	return nullptr;
}



UFC_EquipmentComponent* UFC_Functions::GetHeld(AActor* Actor)
{
	if(!Actor)
	{
		return nullptr; // No Actor
	}
	UFC_EquipmentHandlerComponent* Handler =  Actor->FindComponentByClass<UFC_EquipmentHandlerComponent>();
	if(!Handler)
	{
		return nullptr; // No Handler
	}
	return Handler->GetHeld();
	
}

AActor* UFC_Functions::GetHeldActor(AActor* Actor)
{
	if(!Actor)
	{
		return nullptr; // No Actor
	}
	UFC_EquipmentHandlerComponent* Handler =  Actor->FindComponentByClass<UFC_EquipmentHandlerComponent>();
	if(!Handler)
	{
		return nullptr; // No Handler
	}
	return Handler->GetHeldActor();
	
	
}

UFC_GunComponent* UFC_Functions::GetHeldGunComponent(AActor* Actor)
{
	AActor* HeldActor;
	HeldActor = GetHeldActor(Actor);

	if(HeldActor)
	{
		return HeldActor->FindComponentByClass<UFC_GunComponent>();
	}
	return nullptr;
	

	
}



UFC_AimHandler* UFC_Functions::GetAimHandler(AActor* Actor)
{
	if(Actor)
	{
		return Actor->FindComponentByClass<UFC_AimHandler>();
	}
	return nullptr;
}

UFC_Barrel* UFC_Functions::GetBarrel(AActor* Actor)
{
	if(Actor)
	{
		return Actor->FindComponentByClass<UFC_Barrel>();
	}
	return nullptr;
}

float UFC_Functions::GetHealth(AActor* Actor)
{
	if(!Actor)
	{
		return -1; // No Actor
	}
	UFC_HealthComponent* HealthComp =  Actor->FindComponentByClass<UFC_HealthComponent>();
	if(!HealthComp)
	{
		return -1; // No Handler
	}
	return HealthComp->Health;
	
}

float UFC_Functions::Invert(float in)
{
	return in*-1;
}

float UFC_Functions::GetDirectionToTargetDotProduct(FVector Origin, FVector Direction, FVector Target)
{
	FVector ToTarget = Target - Origin;
	ToTarget.Normalize();

	// Calculate the dot product of the direction vector and the vector pointing to the target
	float DotProduct = FVector::DotProduct(Direction, ToTarget);

	// Return the dot product
	return DotProduct;
}

void UFC_Functions::GetNumberOfAliveOfClass(const UObject* WorldContextObject, TSubclassOf<AActor> Class, TArray<AActor*>& OutActors)
{
	if (UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull))
	{
		for (TActorIterator<AActor> It(World, Class); It; ++It)
		{
			AActor* AliveActor = *It;

			if (IsValid(AliveActor))
			{
				UFC_HealthComponent* HealthComponent = AliveActor->FindComponentByClass<UFC_HealthComponent>();
				if (IsValid(HealthComponent) && HealthComponent->IsAlive())
				{
					OutActors.Add(AliveActor);
				}
			}
		}
	}
}

int32 UFC_Functions::GetClosestRotationIndex(FRotator TargetRotation, const TArray<FRotator>& RotationsArray)
{
	int32 Index = 0;
	float DeltaAngle = FLT_MAX;

	for (int32 i = 0; i < RotationsArray.Num(); i++)
	{
		float TempDelta = FQuat::Error(TargetRotation.Quaternion(), RotationsArray[i].Quaternion());

		if (DeltaAngle > TempDelta)
		{
			DeltaAngle = TempDelta;
			Index = i;
		}
	}

	return Index;
}

float UFC_Functions::GetAngularDistanceBetweenRotators(FRotator Rotator1, FRotator Rotator2)
{
    float AngularDistance = FQuat::Error(Rotator1.Quaternion(), Rotator2.Quaternion());
    return AngularDistance;
}



FVector UFC_Functions::ClosestPointOnLine(const FVector& LineStart, const FVector& LineEnd, const FVector& Point)
{
	FVector LineVector = LineEnd - LineStart;
	FVector PointVector = Point - LineStart;


	float LineLengthSquared = FVector::DotProduct(LineVector, LineVector);

	if (LineLengthSquared == 0.0f)
	{
		return LineStart;
	}

	float t = FMath::Max(0.0f, FMath::Min(1.0f, FVector::DotProduct(PointVector, LineVector) / LineLengthSquared));

	return LineStart + LineVector * t;
}

bool UFC_Functions::IsLinePointingAtPoint(const FVector& LineStart, const FVector& LineEnd, const FVector& Point)
{
	FVector LineVector = LineEnd - LineStart;
	FVector PointVector = Point - LineStart;

	// Check if the line of fire is pointing towards the player
	if (FVector::DotProduct(LineVector, PointVector) < 0.0f)
	{
		return false;
	}
	
	return true;
}

float UFC_Functions::OneMinus(float Value)
{
	return 1 - Value;
}
