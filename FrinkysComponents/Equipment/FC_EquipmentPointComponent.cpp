// Copyright Frinky 2022


#include "FC_EquipmentPointComponent.h"

#include "FC_EquipmentHandlerComponent.h"
#include "Net/UnrealNetwork.h"

UFC_EquipmentPointComponent::UFC_EquipmentPointComponent()
{
	SetIsReplicatedByDefault(true);
}

void UFC_EquipmentPointComponent::BeginPlay()
{
	Super::BeginPlay();

}

void UFC_EquipmentPointComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	//TODO LogOutputDevice: Error: Attempt to detach SceneComponent 'StaticMesh' owned by 'Equippable_C_1' from AttachParent 'Primary' while not attached.
}


void UFC_EquipmentPointComponent::OnRep_Data()
{
	Super::OnRep_Data();
	
	UFC_EquipmentHandlerComponent* OwningEquipmentHandlerComponent = dynamic_cast<UFC_EquipmentHandlerComponent*>(OwningHandler);
	if(OwningEquipmentHandlerComponent && IsValid(OwningEquipmentHandlerComponent))
	{
		OwningEquipmentHandlerComponent->UpdateHeld();
	}
}
