// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "JFGameMode.generated.h"

UCLASS(minimalapi)
class AJFGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AJFGameMode();

	virtual UClass* GetDefaultPawnClassForController_Implementation(AController* InController) override;
};



