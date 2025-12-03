// Copyright Frinky 2022


#include "FC_AnimComponent.h"

#include "Kismet/KismetMathLibrary.h"
#include "Net/UnrealNetwork.h"


// Sets default values for this component's properties
UFC_AnimComponent::UFC_AnimComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
	SetIsReplicatedByDefault(true);

	// ...
}

void UFC_AnimComponent::TickComponent(float DeltaTime, ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if(IsValid(OwningPawn))
	{
		if(OwningPawn->IsLocallyControlled())
		{
			FRotator NewControlRotation = OwningPawn->GetControlRotation();
			AimDifference =  UKismetMathLibrary::NormalizedDeltaRotator(ControlRotation, NewControlRotation);
			
			ControlRotation = OwningPawn->GetControlRotation();
			RelativeControlRotation = UKismetMathLibrary::NormalizedDeltaRotator(OwningPawn->GetActorRotation(), ControlRotation);
			Server_UpdateAimOffset(ControlRotation, RelativeControlRotation);
		}
	}
	
}

void UFC_AnimComponent::BeginPlay()
{
	Super::BeginPlay();
	OwningPawn = dynamic_cast<APawn*>(GetOwner());
}

void UFC_AnimComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

}

void UFC_AnimComponent::MultiUpdateAimOffset_Implementation(FRotator InControlRotation, FRotator InRelativeControlRotation)
{
	if(IsValid(OwningPawn))
	{
		if(!OwningPawn->IsLocallyControlled())
		{
			this->ControlRotation = FMath::RInterpTo(this->ControlRotation, InControlRotation, GetWorld()->GetDeltaSeconds(), 30.0f);
			this->RelativeControlRotation = FMath::RInterpTo(this->RelativeControlRotation, InRelativeControlRotation, GetWorld()->GetDeltaSeconds(), 30.0f);
		}
	}
}

void UFC_AnimComponent::Server_UpdateAimOffset_Implementation( FRotator InAimOffset, FRotator InRelativeAimOffset)
{
	MultiUpdateAimOffset( InAimOffset, InRelativeAimOffset);
}

