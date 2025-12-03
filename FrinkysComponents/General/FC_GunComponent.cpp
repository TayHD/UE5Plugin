// Copyright Frinky 2022


#include "FC_GunComponent.h"
#include "DrawDebugHelpers.h"
#include "Blueprint/AIBlueprintHelperLibrary.h"
#include "Components/AudioComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/SaveGame.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Net/UnrealNetwork.h"
#include "Perception/AISense_Hearing.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "Serialization/BufferArchive.h"

FBulletImpactStruct UFC_BulletImpactData::GetDataFromHitResult(FHitResult HitResult)
{
	for (FBulletImpactStruct ImpactData : ImpactStructs)
	{
		if(ImpactData.PhysMaterial->GetName() == HitResult.PhysMaterial->GetName())
		{
			return ImpactData;
		}
	}

	return ImpactStructs[0];
}

// Sets default values for this component's properties
UFC_GunComponent::UFC_GunComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
	SetIsReplicatedByDefault(true);


	//Defaults
	Ammo = 30;
	MaxAmmo = 30;
	ReloadIndividualRounds = false;

	TraceDebug = EDrawDebugTrace::None;
	TraceComplex = true;
	TraceChannel = ETraceTypeQuery::TraceTypeQuery1;
	DebugImpacts = false;
	TraceDistance = 25000000;
	

	bDoesPlaySound = true;
	bDoesPlayCameraShake = true;
	bDoesPlayProceduralAnimation = true;

	BaseSpread = 0;
	Flechettes = 1;

	FireAnimationTimer = 1000;

	AIBaseSpread = 12;
	AIAdditiveSpread = 0;
	AISoundHearingRange = 3600;
	AISoundLoudness = 1;
	AISoundTag = FName("Gun");

	RPM = 800;
	BaseDamage = 24;

	OwningPawn = nullptr;
	TimeSinceLastShot = 0.0;

	
	
	// ...
}

void UFC_GunComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UFC_GunComponent, Ammo);
	DOREPLIFETIME(UFC_GunComponent,	 MaxAmmo);
	DOREPLIFETIME_CONDITION(UFC_GunComponent, RPM, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(UFC_GunComponent, BaseDamage, COND_OwnerOnly);
	DOREPLIFETIME(UFC_GunComponent, OwningPawn);
	DOREPLIFETIME_CONDITION(UFC_GunComponent, FireMode, COND_OwnerOnly);
	DOREPLIFETIME(UFC_GunComponent, Barrel);
}


// Called when the game starts
void UFC_GunComponent::BeginPlay()
{
	Super::BeginPlay();
	// set default firemode
	FireAnimationTimer = 1000;
	
	SequentialShots = FMath::RandRange(0, 10000);
	if(Firemodes.Num() > 0)
	{
		SetFiremode(Firemodes[0]);
	}
	else
	{
		SetFiremode(EFireMode::Single);
	}

	// ...
}

// Called every frame
void UFC_GunComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                     FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
	if(OwningPawn && OwningPawn->IsLocallyControlled()) {
		TimeSinceLastShot += DeltaTime;
	}

	if(OwningPawn == nullptr || !bDoesPlayProceduralAnimation)
	{
		return;
	}
	
	FireAnimationTimer += DeltaTime * RecoilStruct.TimeDilation;

	const float RandomRecoilFloat = FMath::PerlinNoise1D(float(SequentialShots) / 5) * 4 * RecoilStruct.RandomMulitplier;
	const float LocationFloat = RecoilStruct.LocationCurve->GetFloatValue(FireAnimationTimer * RecoilStruct.LocationTimeDilation) / 2;
	const float RotationFloat = RecoilStruct.RotationCurve->GetFloatValue(FireAnimationTimer * RecoilStruct.RotationTimeDilation) / 3;

	const float LocationFinal = LocationFloat / 2 * RecoilStruct.LocationStrength;
	const FVector Location = FVector(RecoilStruct.xForward ? LocationFinal : 0.0, RecoilStruct.xForward ? 0.0 : LocationFinal, 0.0) * RecoilStruct.AnimationStrength;
	const FVector Rotation = FVector(RecoilStruct.xForward ? RotationFloat * RandomRecoilFloat : RotationFloat * RecoilStruct.PitchStrength * (RecoilStruct.InvertPitch ? -1.0 : 1.0), RecoilStruct.xForward ? RotationFloat * RecoilStruct.PitchStrength * (RecoilStruct.InvertPitch ? -1.0 : 1.0) : RotationFloat * RandomRecoilFloat, RotationFloat * RandomRecoilFloat) * RecoilStruct.AnimationStrength;

	FTransform Transform(FQuat::MakeFromEuler(Rotation), Location );

	AddInput(RotationFloat * DeltaTime * -2 * RandomRecoilFloat * RecoilStruct.CameraRecoilStrength, RotationFloat * DeltaTime * -8.0 * RecoilStruct.CameraRecoilStrength);

	RecoilTransform.SetLocation(FMath::VInterpTo(RecoilTransform.GetLocation(), Transform.GetLocation(), DeltaTime, RecoilStruct.LocationDampening));
	RecoilTransform.SetRotation(FMath::QInterpTo(RecoilTransform.GetRotation(), Transform.GetRotation(), DeltaTime, RecoilStruct.RotationDampening));
}

void UFC_GunComponent::FireStart()
{
	if(FireMode == EFireMode::Single)
	{
		FireShot();
	}
	else if(FireMode == EFireMode::Auto)
	{
		FireTimer = UKismetSystemLibrary::K2_SetTimer(this, "FireShot", GetFireRate(), true, 0, 0);
		FireShot();
	}
	
}

void UFC_GunComponent::FireStop()
{
	UKismetSystemLibrary::K2_ClearAndInvalidateTimerHandle(GetWorld(), FireTimer);
}

void UFC_GunComponent::FireOnce()
{
	FireShot();
}

void UFC_GunComponent::SetAmmo(int NewAmmo)
{
	Ammo = NewAmmo;
}

void UFC_GunComponent::SetFiremode_Implementation(EFireMode NewFireMode)
{
	FireMode = NewFireMode;
}

void UFC_GunComponent::Init_Implementation(UFC_Barrel* InBarrel, APawn* InOwningCharacter)
{
	if(UKismetSystemLibrary::IsServer(GetWorld()))
	{
		Barrel = InBarrel;
		if(InOwningCharacter)
		{
			OwningPawn = InOwningCharacter;
		}
	}
	
}

FTransform UFC_GunComponent::GetRecoilTransform()
{
	return RecoilTransform;	
}

TArray<FCustomHitData> UFC_GunComponent::LineTrace()
{
	TArray<FCustomHitData> HitDataArray;
	FCollisionQueryParams TraceParams;


	//Repeat for flechettes, aka shotgun pellets
	if (IsValid(Barrel) && IsValid(OwningPawn))
	{
		for (int i = 0; i < Flechettes; ++i)
		{
			FVector Start = FVector();
			FVector End = FVector();
			GetBarrelInfo(Start, End);
			End = UKismetMathLibrary::RandomUnitVectorInConeInDegrees(End, (GetSpread()));
			End = Start + (End * TraceDistance);
			AActor* PenetrateActor = nullptr;
			float time = 0.0;
			
			if(TraceDebug == EDrawDebugTrace::ForDuration)
			{
				time = 0.5;
			}
			else if(TraceDebug == EDrawDebugTrace::Persistent)
			{
				time = 100000;
			}
			else if(TraceDebug == EDrawDebugTrace::ForOneFrame)
			{
				time = GetWorld()->DeltaTimeSeconds;
			}
			
			bool bStop = false;
			int TraceIterations = 0;

			float BulletVelocity = 1;
			
			if(IsValid(BulletImpactData))
			{
				BulletVelocity = BulletImpactData->BulletBaseVelocity;
			}
			
			while(!bStop && BulletVelocity >= 0.3) //if we're not stopping and also aren't doing 500 trace iterations
			{
				// TRACE !
				
				FHitResult HitOut;
				bool bHit = UKismetSystemLibrary::LineTraceSingle(GetWorld(), Start, End, TraceChannel, TraceComplex,
																  TArray<AActor*>() = {OwningPawn, GetOwner()},
																  EDrawDebugTrace::None, HitOut, true, FLinearColor::Red,
																  FLinearColor::Green, time);

				// TRACE !
				
				if (bHit)
				{
					FBulletImpactStruct BulletImpact;
					BulletImpact = BulletImpactData->GetDataFromHitResult(HitOut); //get our bullet data for this surface

					
					HitDataArray.Add(FCustomHitData(HitOut.GetActor(), Barrel->GetForwardVector(), HitOut, BulletVelocity)); //add to array to be damaged

					bool bRicochet = true;
					bool bPenetrate = true;
					
					
					
					float HitAngle = abs( FVector::DotProduct(HitOut.ImpactNormal, UKismetMathLibrary::GetDirectionUnitVector(Start, End)));
					HitAngle = (1 - HitAngle) * 90;
					if(DebugSurfaceImpacts)
					{
						GEngine->AddOnScreenDebugMessage(1, 100, FColor::Red, "HitAngle : " + FString::SanitizeFloat(HitAngle));
						GEngine->AddOnScreenDebugMessage(2, 100, FColor::Red, "BulletVelocityPreImpact : " + FString::SanitizeFloat(BulletVelocity));
						DrawDebugLine(GetWorld(), Start, HitOut.ImpactPoint, BulletImpact.DebugColour, false, time, 0, BulletVelocity*2);
						DrawDebugString(GetWorld(), HitOut.ImpactPoint, "Hit: " + HitOut.PhysMaterial->GetName() + " | Velocity: " + FString::SanitizeFloat(BulletVelocity), 0, FColor::Green, time, false, 1 );
						DrawDebugSphere(GetWorld(), HitOut.ImpactPoint,  BulletVelocity*15, 4, BulletImpact.DebugColour, false, time);
					}
					if(HitAngle > BulletImpact.RicochetAngleThreshold && UKismetMathLibrary::RandomBoolWithWeight(BulletImpact.RicochetChance))
					{
						if(DebugSurfaceImpacts)
						{
							GEngine->AddOnScreenDebugMessage(3, 100, FColor::Blue, "BULLET RICOCHET! - " + BulletImpact.PhysMaterial->GetName() );
						}
						// RICOCHET
						Start = HitOut.ImpactPoint;
						End =  Start + (UKismetMathLibrary::GetReflectionVector(UKismetMathLibrary::GetDirectionUnitVector(Start, End), HitOut.ImpactNormal))*TraceDistance;
						BulletVelocity = BulletVelocity - BulletImpact.BulletRicochetVelocityReduction; 
						TraceIterations++;
						continue;
					}
					
					if(UKismetMathLibrary::RandomBoolWithWeight(BulletImpact.PenetrationChance))
					{
						if(DebugSurfaceImpacts)
						{
							GEngine->AddOnScreenDebugMessage(3, 100, FColor::Blue, "BULLET PENETRATED - " + BulletImpact.PhysMaterial->GetName() );
						}
						// PENETRATION
						
						FHitResult PenTrace;
						FVector PenA = UKismetMathLibrary::GetDirectionUnitVector(Start, End) * BulletImpact.PenetrationPower + HitOut.ImpactPoint;
						
						bool bPenHit = UKismetSystemLibrary::LineTraceSingle(GetWorld(), PenA, HitOut.ImpactPoint, TraceChannel, TraceComplex,
																		  TArray<AActor*>() = {OwningPawn, GetOwner()},
																		  TraceDebug, PenTrace, true, FLinearColor::Red,
																		  FLinearColor::Green, time);

						if (bPenHit)
						{
							Start = PenTrace.ImpactPoint + UKismetMathLibrary::GetDirectionUnitVector(Start, End);
							End = Start +  UKismetMathLibrary::RandomUnitVectorInConeInDegrees(UKismetMathLibrary::GetDirectionUnitVector(Start, End), BulletImpact.PenetrationRicochetAngle) * TraceDistance;
							BulletVelocity = BulletVelocity - BulletImpact.BulletPenetrationVelocityReduction; 
							TraceIterations++;
							continue;
						}
						else
						{
							bStop = true;
						}
						
						
					
					}


					//GEngine->AddOnScreenDebugMessage(4, 100, FColor::Red, "BulletVelocityPostImpact : " + FString::SanitizeFloat(BulletVelocity));
				}
				
				bStop = true; //stop traces if we've hit our limit
			}
			
		}
	}
	else
	{
		// Log that we couldn't shoot 
		UE_LOG(LogTemp, Error, TEXT("Attempted to trace without a barrel or owning character"));
		GEngine->AddOnScreenDebugMessage(-1, 2, FColor::Red, "Attempted to trace without a barrel or owning character");
	}


	return HitDataArray;
}

void UFC_GunComponent::ClientFire_Implementation()
{
	if(OwningPawn)
	{
		if (Ammo > 0)
		{
			//Damage trace
			const TArray<FCustomHitData> Hits = LineTrace();
			ServerApplyDamageToHitResults(Hits);
			OnClientHit.Broadcast(Hits);
		
			if (OwningPawn->IsLocallyControlled())
			{
				OnClientFire.Broadcast();
				if(GetOwnerRole() != ROLE_Authority)
				{
					Ammo--;
				}
				FireVisuals();

				// Play camera shake
				if(!bDoesPlayCameraShake)
				{
					return;
				}
				
				AController* Controller = OwningPawn->GetController();
				if(Controller)
				{
					const APlayerController* PlayerController = dynamic_cast<APlayerController*>(Controller);
					if(PlayerController)
					{
						PlayerController->PlayerCameraManager->StartCameraShake(RecoilStruct.MacroShake, RecoilStruct.MacroShakeMultiplier);
						PlayerController->PlayerCameraManager->StartCameraShake(RecoilStruct.MicroShake, RecoilStruct.MicroShakeMultiplier);
					}
				}
			}
		}
	}
	else
	{
		FireStop();
	}
}

void UFC_GunComponent::ServerFire_Implementation()
{
	if (Ammo > 0)
	{
		Ammo--;
		AIReportSound();
		MultiFire();
	}
	
}

void UFC_GunComponent::MultiFire_Implementation()
{
	if (OwningPawn && !OwningPawn->IsLocallyControlled())
	{
		OnProxyFire.Broadcast();
		FireVisuals();
	}
}

void UFC_GunComponent::CycleFiremodes_Implementation()
{
	int Index = Firemodes.Find(FireMode);
	if(Index == Firemodes.Num()-1)
	{
		Index = 0;
	}
	else
	{
		Index++;
	}

	SetFiremode(Firemodes[Index]);
}

void UFC_GunComponent::FireShot()
{
	if((TimeSinceLastShot+ (GetWorld()->DeltaTimeSeconds*2)) >= GetFireRate())
	{
		TimeSinceLastShot = 0;
		ClientFire();
		ServerFire();
	}
}

void UFC_GunComponent::AIReportSound_Implementation()
{
	if(OwningPawn)
	{
		UAISense_Hearing::ReportNoiseEvent(GetWorld(), GetOwner()->GetActorLocation(), AISoundLoudness, OwningPawn, AISoundHearingRange, AISoundTag);
	}
}

void UFC_GunComponent::SetOwningPawn_Implementation(APawn* NewPawn)
{
	OwningPawn = NewPawn;
}

void UFC_GunComponent::ServerApplyDamageToHitResults_Implementation(const TArray<FCustomHitData>& HitResults)
{
	if (IsValid(OwningPawn))
	{
		for (const auto Hit : HitResults)
		{
			float FinalDamage = BaseDamage * Hit.HitStrength;
			if(DidHeadshot(Hit.HitResult))
			{
				FinalDamage = FinalDamage * HeadshotDamageMulitplier;
			}
			
			//Apply damage
			UGameplayStatics::ApplyPointDamage(Hit.HitActor, FinalDamage, Hit.HitDirection, Hit.HitResult,
			                                   OwningPawn->GetController(), GetOwner(), DamageType);

			//Debug impacts by spawning a box (if enabled)
			if (DebugImpacts)
			{
				DrawDebugBox(GetWorld(), Hit.HitResult.Location, FVector(5, 5, 5), FColor::Yellow, false, 1, 0, 1);
			}
		}
		OnServerHit.Broadcast(HitResults);
		
	}
}

const float UFC_GunComponent::GetFireRate()
{
	return 1 / (RPM / 60);
}

bool UFC_GunComponent::isBot()
{
	if (IsValid(OwningPawn))
	{
		if(UAIBlueprintHelperLibrary::GetAIController(OwningPawn))
		{
			return true;
		}
	}
	return false;
}

bool UFC_GunComponent::DidHeadshot(FHitResult HitResult)
{
	return HeadshotBones.Contains(HitResult.BoneName);
}


bool UFC_GunComponent::DidHitClass(bool& isHeadshot, TSubclassOf<AActor> ActorClass, TArray<FCustomHitData> Hits)
{
	for (const auto Hit : Hits)
	{
		if(Hit.HitActor->GetClass()->IsChildOf(ActorClass.Get()))
		{
			if(Hit.HitResult.BoneName == FName("head"))
			{
				isHeadshot = true;
			}
			else
			{
				isHeadshot = false;
			}
			return true;
		}
	}
	return false;
}

void UFC_GunComponent::GetBarrelInfo(FVector& Location, FVector& Forward)
{
	if(IsValid(Barrel) && IsValid(OwningPawn))
	{
		APlayerController* Controller = dynamic_cast<APlayerController*>(OwningPawn->GetController());
	
		if(Controller)
		{
			if(ThirdPersonStyleTrace)
			{
				Location = Controller->PlayerCameraManager->GetCameraLocation();
				Forward = UKismetMathLibrary::GetForwardVector(OwningPawn->GetBaseAimRotation());
				FVector End;
				FHitResult HitOut;
				bool bHit = UKismetSystemLibrary::LineTraceSingle(GetWorld(), Location, Forward*TraceDistance, TraceChannel, TraceComplex,
																  TArray<AActor*>() = {OwningPawn, GetOwner()},
																  EDrawDebugTrace::None, HitOut, true, FLinearColor::Red,
																  FLinearColor::Green);
				if(!bHit)
				{
					End = HitOut.TraceEnd;
				}
				else
				{
					End = HitOut.Location;
				}
				FHitResult SecondHitOut;
				bool bSecondHit = UKismetSystemLibrary::LineTraceSingle(GetWorld(), Barrel->GetComponentLocation(), End + Forward, TraceChannel, TraceComplex,
																  TArray<AActor*>() = {OwningPawn, GetOwner()},
																  EDrawDebugTrace::None, SecondHitOut, true, FLinearColor::Red,
																  FLinearColor::Green);
				
				if(bSecondHit){ Forward = UKismetMathLibrary::GetDirectionUnitVector(Location, SecondHitOut.ImpactPoint); }

			}
			else
			{
				if(TraceFromCamera)
				{
					Location = Controller->PlayerCameraManager->GetCameraLocation();

				}
				else
				{
					Location = Barrel->GetComponentLocation();
				}
				if(TraceToCamera)
				{
					Forward = UKismetMathLibrary::GetForwardVector(OwningPawn->GetBaseAimRotation());

				}
				else
				{
					Forward = Barrel->GetForwardVector();
				}
				return;
			}
			
		}
		else
		{
			Location = Barrel->GetComponentLocation();
			Forward = Barrel->GetForwardVector();
			return;
		}
	}
	
}

float UFC_GunComponent::GetSpread()
{
	float Spread = 0.0;
	Spread += BaseSpread;
	if(isBot())
	{
		Spread += AIBaseSpread + AIAdditiveSpread;
	}

	return Spread;
}

bool UFC_GunComponent::IsSuppressed()
{
	if(Barrel)
	{
		return Barrel->bIsSuppressor;
	}
	return false;
}

void UFC_GunComponent::SetSpread_Implementation(float NewSpread)
{
	BaseSpread = NewSpread;
}

void UFC_GunComponent::SetFlechettes_Implementation(int NewFlechettes)
{
	Flechettes = NewFlechettes;
}

void UFC_GunComponent::SetDamage_Implementation(float NewDamage)
{
	BaseDamage = NewDamage;
}

void UFC_GunComponent::SetMaxAmmo_Implementation(int NewMaxAmmo)
{
	MaxAmmo = NewMaxAmmo;
}

void UFC_GunComponent::SetBarrel_Implementation(UFC_Barrel* NewBarrel)
{
	Barrel = NewBarrel;
}

void UFC_GunComponent::Reload_Implementation()
{
	if(!ReloadIndividualRounds)
	{
		Ammo = MaxAmmo;
	}
	else
	{
		Ammo++;
		Ammo = FMath::Clamp(Ammo, 0, MaxAmmo);
	}

	OnReloadComplete.Broadcast();
	
}

int UFC_GunComponent::ReloadWithAmmoPool(int AmmoPool)
{
	// This function is called when we want to reload with a pool of ammo (e.g. from a magazine) Its up to you how to implement this

	
	int AmmoToReload = 0;
	if(ReloadIndividualRounds)
	{
		// If we reload individual rounds (1 at a time), just reload 1 round
		AmmoToReload = FMath::Min(AmmoPool, 1);
	}
	else
	{
		//reload only what we have in the pool, and retun the remaining ammo in the pool
		AmmoToReload = FMath::Min(AmmoPool, MaxAmmo - Ammo);
	}
	
	Ammo += AmmoToReload;

	OnReloadComplete.Broadcast();
	return AmmoPool - AmmoToReload;
}



int32 UFC_GunComponent::GetCurrentAmmo()
{
	return Ammo;
}

int32 UFC_GunComponent::GetMaxAmmo()
{
	return MaxAmmo;
}

void UFC_GunComponent::FireVisuals()
{
	SequentialShots++;
	FireAnimationTimer = 0.0;
	
	if(Barrel)
	{
		Barrel->Shoot();
		OnFireVisuals.Broadcast();
		if(Gunshot)
		{
			Gunshot->FadeOut(GunshotFadeTime, 0.0);
		}

		if(!bDoesPlaySound)
		{
			return;
		}
		if(Barrel->bIsSuppressor)
		{
			Gunshot = UGameplayStatics::SpawnSoundAttached(SuppressorSound, Barrel, NAME_None, FVector(), FRotator(), EAttachLocation::SnapToTarget,
			true, 1, 1, 0, SuppressorAttenuation, SuppressorConcurrency, true);
		}
		else
		{
			Gunshot = UGameplayStatics::SpawnSoundAttached(Sound, Barrel, NAME_None, FVector(), FRotator(), EAttachLocation::SnapToTarget,
			true, 1, 1, 0, Attenuation, Concurrency, true);
		}
		
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(-1, 1000, FColor::Red, "GUN COMPONENT ERROR : NO BARREL SET");
	}
	
}

void UFC_GunComponent::AddInput(float x, float y)
{
	if(OwningPawn)
	{
		if(OwningPawn->IsLocallyControlled())
		{
			OwningPawn->AddControllerPitchInput(y);
			OwningPawn->AddControllerYawInput(x);
		}
	}
	
}

void UFC_GunComponent::SaveStats()
{
	FMemoryWriter MemoryWriter(SaveData, true);
	FObjectAndNameAsStringProxyArchive arc(MemoryWriter, false);
	arc.ArIsSaveGame = true;
	arc.ArNoDelta = true;
	Serialize(arc);
	
}

void UFC_GunComponent::LoadStats()
{
	FMemoryReader MemoryReader(SaveData, true);
	FObjectAndNameAsStringProxyArchive arc(MemoryReader, false);
	arc.ArIsSaveGame = true;
	arc.ArNoDelta = true;
	Serialize(arc);

}



void UFC_GunComponent::Serialize(FArchive& Ar)
{
	Super::Serialize(Ar);
	
}

