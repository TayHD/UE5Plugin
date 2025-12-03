// Copyright Frinky 2022


#include "FC_SeatManager.h"


// Sets default values for this component's properties
UFC_SeatManager::UFC_SeatManager()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);

	// ...
}


// Called when the game starts
void UFC_SeatManager::BeginPlay()
{
	Super::BeginPlay();

	// ...

}


// Called every frame
void UFC_SeatManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}
