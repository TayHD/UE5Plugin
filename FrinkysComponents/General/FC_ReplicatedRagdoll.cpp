// Fill out your copyright notice in the Description page of Project Settings.


#include "FC_ReplicatedRagdoll.h"

#include "Net/UnrealNetwork.h"

void UFC_ReplicatedRagdoll::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UFC_ReplicatedRagdoll, RagdollPositionServer);
}

UFC_ReplicatedRagdoll::UFC_ReplicatedRagdoll()
{
	// By default we won't tick, only when initialized
	PrimaryComponentTick.bCanEverTick = true;
	SetIsReplicatedByDefault(true);
}

void UFC_ReplicatedRagdoll::BeginPlay()
{
	Super::BeginPlay();
	OwningPawn = Cast<APawn>(GetOwner());
	if(!IsValid(OwningPawn))
	{
		GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, TEXT("replicated ragdoll component must be attached to a pawn!"));
		
	}
	
}



void UFC_ReplicatedRagdoll::TickComponent(float DeltaTime, ELevelTick TickType,
                                          FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if(!IsValid(MeshToSync)) return;

	if(!isSimulating()) return;

	if(!IsValid(OwningPawn)) return;

	////////////////////////////

	if(OwningPawn->IsLocallyControlled())
	{
		FTransform CurrentTransform = GetQuantizedRagdollTransform(); // Get the current transform of the ragdoll
		
		if(!CurrentTransform.Equals(RagdollPositionServer, 20))
		{
			ServerSetRagdollTransform(CurrentTransform); // Send the ragdoll transform to the server
		}
		
	}

	ApplyRagdollTransform(); // Apply the ragdoll transform on the client
	
	DrawDebug();
	
}

void UFC_ReplicatedRagdoll::InitializeRagdoll(USkeletalMeshComponent* Mesh)
{

	MeshToSync = Mesh; // Set the mesh to sync on the client calling this function
	
	// Log
	UE_LOG(LogTemp, Warning, TEXT("Ragdoll initialized on %s"), *GetOwner()->GetName());
	
	if(IsValid(Mesh) && IsValid(OwningPawn))
	{
		SetComponentTickEnabled(true); // Enable ticking on this component
	}

}

void UFC_ReplicatedRagdoll::ServerSetRagdollTransform_Implementation(FTransform Transform)
{
	RagdollPositionServer = Transform; // Set the ragdoll position on the server
}

FTransform UFC_ReplicatedRagdoll::GetQuantizedRagdollTransform()
{
	if(!IsValid(MeshToSync)) return FTransform::Identity;

	if(!isSimulating()) return FTransform::Identity;

	if(!IsValid(OwningPawn)) return FTransform::Identity;

	FTransform Out;

	Out.SetLocation(MeshToSync->GetBoneLocation(BoneToSync, EBoneSpaces::WorldSpace)); // Get the location of the bone we want to sync in world space
	Out.SetRotation(MeshToSync->GetBoneQuaternion(BoneToSync, EBoneSpaces::WorldSpace)); // Get the rotation of the bone we want to sync in world space

	// Quantize the transform to save bandwidth
	Out.NormalizeRotation(); // Normalize the rotation

	Out.SetLocation(FVector_NetQuantize10(Out.GetLocation()));
	return Out;	
	
}

FTransform UFC_ReplicatedRagdoll::GetRagdollTransform()
{
	if(!IsValid(MeshToSync)) return FTransform::Identity;

	if(!isSimulating()) return FTransform::Identity;

	if(!IsValid(OwningPawn)) return FTransform::Identity;

	FTransform Out;

	Out.SetLocation(MeshToSync->GetBoneLocation(BoneToSync, EBoneSpaces::WorldSpace)); // Get the location of the bone we want to sync in world space
	Out.SetRotation(MeshToSync->GetBoneQuaternion(BoneToSync, EBoneSpaces::WorldSpace)); // Get the rotation of the bone we want to sync in world space

	return Out;
}

bool UFC_ReplicatedRagdoll::isSimulating()
{
	if(!IsValid(MeshToSync)) return false;

	return MeshToSync->IsSimulatingPhysics(BoneToSync);
}

void UFC_ReplicatedRagdoll::ApplyRagdollTransform()
{
	if(!IsValid(MeshToSync)) return;

	if(!isSimulating()) return;

	if(!IsValid(OwningPawn)) return;

	if(OwningPawn->IsLocallyControlled()) return;

	float LocationDistance = FVector::Dist(RagdollPositionServer.GetLocation(), GetRagdollTransform().GetLocation());

	if (LocationDistance < 50) return; // If the ragdoll is close enough to the server position, don't apply any force

	LocationDistance = LocationDistance / 100;
	LocationDistance = FMath::Clamp(LocationDistance, 0.f, 1.f); // Clamp the distance between 0 and 1
	
	FVector ForceToApply = (RagdollPositionServer.GetLocation() - GetRagdollTransform().GetLocation()) * Force; // Get the force to apply to the ragdoll
	// then, we need to scale the force by the distance, because the closer the ragdoll is to the server position, the less force we need to apply

	// Clamp the force to apply between 0 and 50000
	ForceToApply = ForceToApply.GetClampedToSize(0, Force*10);

	// Then, scale it by the distance
	ForceToApply *= LocationDistance;

	MeshToSync->AddForce(ForceToApply, BoneToSync, false); // Apply the force to the ragdoll


	if(bRotationSync)
	{
		FRotator TorqueToApply = (RagdollPositionServer.GetRotation().Rotator() - GetRagdollTransform().Rotator()) * 10000; // Get the torque to apply to the ragdoll
		TorqueToApply.Normalize();

		MeshToSync->AddTorqueInRadians(TorqueToApply.Euler(), BoneToSync, false); // Apply the torque to the ragdoll

	}
	
}

void UFC_ReplicatedRagdoll::DrawDebug()
{
	if (!bDrawDebug) return;
	DrawDebugSphere(GetWorld(), RagdollPositionServer.GetLocation(), 50, 12, FColor::Green, false, 0.1f); // Draw server ragdoll position
	DrawDebugDirectionalArrow(GetWorld(), RagdollPositionServer.GetLocation(), RagdollPositionServer.GetLocation() + RagdollPositionServer.GetRotation().GetForwardVector() * 100, 50, FColor::Green, false, 0.1f); // Draw server ragdoll forward vector
	
	DrawDebugSphere(GetWorld(), GetRagdollTransform().GetLocation(), 25, 12, FColor::Red, false, 0.1f); // Draw client ragdoll position
	DrawDebugDirectionalArrow(GetWorld(), GetRagdollTransform().GetLocation(), GetRagdollTransform().GetLocation() + GetRagdollTransform().GetRotation().GetForwardVector() * 100, 50, FColor::Red, false, 0.1f); // Draw client ragdoll forward vector
}
