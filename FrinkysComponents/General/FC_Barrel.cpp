// Copyright Frinky 2022


#include "FC_Barrel.h"

#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"

UFC_Barrel::UFC_Barrel()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);

}

void UFC_Barrel::Shoot()
{
	OnShoot.Broadcast();
	
	//todo setup particle spawns (tommorow job)
}

