// Copyright Frinky 2022


#include "FrinkysComponents/General/FC_ShooterGameMode.h"

void AFC_ShooterGameMode::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	
	
	for (AController* Player : PlayerList)
	{
		APawn* PlayerChar = Player->GetPawn();
		if(IsValid(PlayerChar))
		{
			FVector pos = PlayerChar->GetActorLocation();
			FRotator rot = PlayerChar->GetActorRotation();
		
		
			PlayerHistory CurrentHistory {PlayerChar->K2_GetActorLocation(), PlayerChar->GetActorRotation(), PlayerChar};
		
		
			TransformHistory[HistoryIndex].Reset();
			TransformHistory[HistoryIndex].Add(CurrentHistory);
			HistoryIndex = (HistoryIndex+1) % MAX_HISTORY;
		}
	}
	
}



void AFC_ShooterGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);
	PlayerList.Add(NewPlayer);
}

void AFC_ShooterGameMode::Logout(AController* Exiting)
{

	Super::Logout(Exiting);
	PlayerList.Remove(Exiting);
}
