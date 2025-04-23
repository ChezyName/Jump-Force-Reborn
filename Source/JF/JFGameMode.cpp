// Copyright Epic Games, Inc. All Rights Reserved.

#include "JFGameMode.h"
#include "JFCharacter.h"
#include "UObject/ConstructorHelpers.h"

AJFGameMode::AJFGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
