// Copyright Frinky 2022

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "FC_ShooterGameMode.generated.h"

/**
 * 
 */

struct PlayerHistory
{
	FVector pos;
	FRotator rot;
	APawn* pawn;
};

UCLASS()
class FRINKYSCOMPONENTS_API AFC_ShooterGameMode : public AGameMode
{
	GENERATED_BODY()
	
	
	virtual void Tick(float DeltaSeconds) override;

	virtual void PostLogin(APlayerController* NewPlayer) override;

	virtual void Logout(AController* Exiting) override;

	
	
public:
	static constexpr unsigned short MAX_HISTORY = 1024;
	
	TArray<AController*> PlayerList;
	
	TArray<PlayerHistory> TransformHistory[MAX_HISTORY];
	int HistoryIndex = 0;
};


