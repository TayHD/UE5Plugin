// Copyright Frinky 2022


#include "FC_InputHandler.h"

#include "GameFramework/Character.h"

// Sets default values for this component's properties
UFC_InputHandler::UFC_InputHandler()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
	Sensitivity = 1.0;
	
	

	// ...
}


// Called when the game starts
void UFC_InputHandler::BeginPlay()
{
	Super::BeginPlay();
	APawn* TempPawn = dynamic_cast<APawn*>(this->GetOwner());
	if(IsValid(TempPawn))
	{
		Pawn = TempPawn;
		GEngine->AddOnScreenDebugMessage(-1, 2, FColor::Green, TEXT("Input component successfully initilized!"));
		PrimaryComponentTick.bCanEverTick = true;
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(-1, 2, FColor::Red, TEXT("Input component not added to pawn!"));
		PrimaryComponentTick.bCanEverTick = false;
	}

	// ...
	
}


// Called every frame
void UFC_InputHandler::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	if(IsValid(Pawn) && Pawn->IsLocallyControlled())
	{
		Pawn->AddMovementInput(Pawn->GetActorForwardVector(), ForwardInput);
		Pawn->AddMovementInput(Pawn->GetActorRightVector(), RightInput);
	
	}
	// ...
}

FVector2D UFC_InputHandler::HandleMouseInput(FVector MouseInput)
{
	if(IsValid(Pawn))
	{
		FVector2D FinalVector = FVector2D(MouseInput.X*Sensitivity, MouseInput.Y*-1.0*Sensitivity);
		Pawn->AddControllerYawInput(FinalVector.X);
		Pawn->AddControllerPitchInput(FinalVector.Y);
		MouseDirection = FinalVector;
		return FinalVector;
	}
	else
	{
		return FVector2D();
	}
}

void UFC_InputHandler::HandleRightInput(float Input)
{
	RightInput = Input;	 

}

void UFC_InputHandler::HandleForwardInput(float Input)
{
	ForwardInput = Input;
}



