// Fill out your copyright notice in the Description page of Project Settings.


#include "FC_AnimInstance.h"

#include "FC_AnimComponent.h"
#include "Engine/GameEngine.h"
#include "GameFramework/Character.h"
#include "GameFramework/PawnMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Net/UnrealNetwork.h"

UFC_AnimInstance::UFC_AnimInstance()
{
	
	LookDeltaInterp = 5;
	LookDeltaHorizontalMultiplier = 2;
	LookDeltaVerticalMultiplier = 2;
}


void UFC_AnimInstance::NativeBeginPlay()
{
	Super::NativeBeginPlay();
	Pawn = TryGetPawnOwner();
	Character = dynamic_cast<ACharacter*>(Pawn);
	HealthComponent = Pawn->FindComponentByClass<UFC_HealthComponent>();
	

}

void UFC_AnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);
	
	if(Pawn)
	{
		
		bIsMontagePlaying = Montage_IsPlaying(nullptr);
		Velocity = Pawn->GetVelocity();
		Speed = Velocity.Size();
		ForwardVelocity = FVector::DotProduct(Velocity, Pawn->GetActorForwardVector());
		RightVelocity = FVector::DotProduct(Velocity, Pawn->GetActorRightVector());
		
		bIsLocallyControlled = Pawn->IsLocallyControlled();
		if(!Pawn->IsPlayerControlled()) { bIsLocallyControlled = false;  }
		
		FRotator AimDifference;
		UFC_AnimComponent* AnimComponentRef = TryGetPawnOwner()->FindComponentByClass<UFC_AnimComponent>();
		if(IsValid(AnimComponentRef))
		{
			AimDifference = AnimComponentRef->AimDifference; // get the aim difference
			ControlRotation = AnimComponentRef->ControlRotation; // set the control rotation
			RelativeControlRotation = AnimComponentRef->RelativeControlRotation; // get the relative control rotation
			
		}
		else
		{
			FRotator NewControlRotation = Pawn->GetControlRotation();
			
			AimDifference =  UKismetMathLibrary::NormalizedDeltaRotator(ControlRotation, NewControlRotation); // get the aim difference
			ControlRotation = Pawn->GetControlRotation(); // set the control rotation
			RelativeControlRotation = UKismetMathLibrary::NormalizedDeltaRotator(Pawn->GetActorRotation(), ControlRotation); // get the relative control rotation
		}
		
		LookDelta = FMath::Vector2DInterpTo(LookDelta, FVector2D( AimDifference.Yaw * LookDeltaHorizontalMultiplier, AimDifference.Pitch * LookDeltaVerticalMultiplier), DeltaSeconds, LookDeltaInterp);
		if(IsValid(Character) && IsValid(Character->GetMovementComponent()))
		{
			bIsFalling = Character->GetMovementComponent()->IsFalling();
			bIsCrouching = Character->GetMovementComponent()->IsCrouching();
			isSwimming = Character->GetMovementComponent()->IsSwimming();
		}
		if(IsValid(HealthComponent))
		{
			bIsAlive = HealthComponent->IsAlive();
			Health = HealthComponent->Health;
			HealthPercent = HealthComponent->HealthPercent();
		}
	}
	else
	{
		Pawn = TryGetPawnOwner();
	}
}

void UFC_AnimInstance::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
}


void UFC_AnimInstance::MultiSetControlRotation_Implementation(FRotator Rotation)
{
	if(!Pawn->IsLocallyControlled())
	{
		//**old float ViewAngle = Pawn->GetRemoteViewPitch * 360.f/255.f;**
		float ViewAngle = Pawn->GetRemoteViewPitch() * 360.f/255.f;
		ControlRotation = FRotator(ViewAngle, Rotation.Yaw, Rotation.Roll);
	}
}

void UFC_AnimInstance::ServerSetControlRotation_Implementation(FRotator Rotation)
{
	MultiSetControlRotation(Rotation);
}

void UFC_AnimInstance::CalculateBlocking(float DeltaTime, float Length, float Width, FTransform CurrentBlockingTransform, bool& DisableAimLowered, FTransform& BlockingTransform)
{
	//Get BlockingBone Transform and set trace vars
	FTransform BlockingBoneSocketTransform = GetOwningComponent()->GetSocketTransform(FName(TEXT("VB BlockingBone")), RTS_World);
	FVector BoxTraceStart = BlockingBoneSocketTransform.GetLocation() + FVector(0,0,2) + (BlockingBoneSocketTransform.GetRotation().GetRightVector() * -15.0f);
	FVector BoxTraceEnd = BlockingBoneSocketTransform.GetLocation() + FVector(0,0,2) + (BlockingBoneSocketTransform.GetRotation().GetRightVector() * Length);
	static TArray<AActor*> ActorsToIgnore;
	ActorsToIgnore.Reset();
	ActorsToIgnore.Add(TryGetPawnOwner());
	
	FHitResult HitResult;
	//Do Box Trace
	bool successful = (UKismetSystemLibrary::BoxTraceSingle(GetWorld(), BoxTraceStart, BoxTraceEnd, FVector(Width, 0, 4.0f), FRotator(BlockingBoneSocketTransform.GetRotation().X,
		BlockingBoneSocketTransform.GetRotation().Y, BlockingBoneSocketTransform.GetRotation().Z) , ETraceTypeQuery::TraceTypeQuery1,
		false, ActorsToIgnore, EDrawDebugTrace::None, HitResult, true ,FLinearColor::Gray, FLinearColor::Blue, 2.0f));
	
	double dist = successful ? (HitResult.Distance - FVector::Distance(HitResult.TraceStart, HitResult.TraceEnd)) : 0.0f;
	if((dist < -27.0))
	{
		BlockingTransform = UKismetMathLibrary::TInterpTo(CurrentBlockingTransform,UKismetMathLibrary::MakeTransform(FVector(-5, -20, -10), FRotator (-65.0, 0, 0), FVector (1,1,1)), DeltaTime, 10.0f);
		DisableAimLowered = dist >= -8.0;
			
	} else
	{
		BlockingTransform = UKismetMathLibrary::TInterpTo(CurrentBlockingTransform, UKismetMathLibrary::MakeTransform(FVector(dist / 4, dist, 0), FRotator (0, dist, 0), FVector (0,0,0)), DeltaTime, 10.0f);
		DisableAimLowered = false;
	}
	
}

void UFC_AnimInstance::ProceduralADSBreathing(float DeltaTime, float TimeSeconds, float NoiseInterpSpeed, float MovementMultiplier,
	float IdleValue, FVector Location, FRotator Rotation, FVector& OutLocation, FRotator& OutRotation)
{
	// Update Noise
	InterpedSpeedRatio = FMath::FInterpTo(InterpedSpeedRatio, FMath::Abs(Speed) / Pawn->GetMovementComponent()->GetMaxSpeed(), DeltaTime, 5.0f);
	const float BaseValue = (InterpedSpeedRatio * MovementMultiplier) + IdleValue;
	const float DeltaAimingNoise = BaseValue * FMath::FRandRange(0.2, 0.28) * DeltaTime;
	AimingNoise = FMath::FInterpTo(AimingNoise, AimingNoise + DeltaAimingNoise, DeltaTime, NoiseInterpSpeed);

	// Generate Location
	const double OutLocX = (FMath::PerlinNoise1D(0.01 * TimeSeconds) * BaseValue * 0.2 * FMath::Cos(AimingNoise)) + Location.X;
	const double OutLocY = (FMath::PerlinNoise1D(0.2 * TimeSeconds) * FMath::Sin(AimingNoise)) + Location.Y;
	const double OutLocZ = 0.05 * FMath::Sin(AimingNoise) + Location.Z;
	OutLocation = FVector(OutLocX, OutLocY, OutLocZ);

	// Generate Rotation
	const float OutRotX = (0.4 * FMath::Sin(AimingNoise) * BaseValue) + (FMath::Cos(AimingNoise) * 0.01 * BaseValue) + Rotation.Roll;
	const float OutRotY = FMath::PerlinNoise1D(0.22 * TimeSeconds) * (FMath::Cos(AimingNoise) + FMath::PerlinNoise1D(3.0 * TimeSeconds) * FMath::Sin(AimingNoise)) + Rotation.Pitch;
	const float OutRotZ = (FMath::Cos(AimingNoise) + FMath::PerlinNoise1D(2.0 * TimeSeconds) * FMath::Sin(AimingNoise)) * 0.08 + Rotation.Yaw;
	OutRotation = FRotator(OutRotX, OutRotY, OutRotZ);
}