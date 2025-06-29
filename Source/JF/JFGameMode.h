// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "JFCharacter.h"
#include "GameFramework/GameModeBase.h"
#include "JFGameMode.generated.h"

UCLASS(minimalapi)
class AJFGameMode : public AGameModeBase
{
	GENERATED_BODY()

	int PlayerSpawnCount = 0;

public:
	AJFGameMode();

	virtual UClass* GetDefaultPawnClassForController_Implementation(AController* InController) override;

	UFUNCTION(Server, Reliable)
	void PlayerKilled(AJFCharacter* Killer, AJFCharacter* Killed);

	virtual void PostLogin(APlayerController* NewPlayer) override;
	virtual void InitSeamlessTravelPlayer(AController* NewController) override;
	
	UPROPERTY(EditAnywhere)
	FString LevelURL = FString("/Game/MainMenu/Lobby/Lobby");
};



