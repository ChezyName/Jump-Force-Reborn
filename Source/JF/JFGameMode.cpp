// Copyright Epic Games, Inc. All Rights Reserved.

#include "JFGameMode.h"
#include "JFCharacter.h"
#include "Game/JFPlayerState.h"
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

UClass* AJFGameMode::GetDefaultPawnClassForController_Implementation(AController* InController)
{
	AJFPlayerState* PS = InController->GetPlayerState<AJFPlayerState>();
	if(PS && PS->GetHero() && PS->GetHero()->Actor.IsValid())
	{
		//Load Actor
		UClass* LoadedHero = PS->GetHero()->Actor.LoadSynchronous();
		if (LoadedHero)
		{
			return LoadedHero;
		}
	}
	
	return Super::GetDefaultPawnClassForController_Implementation(InController);
}
