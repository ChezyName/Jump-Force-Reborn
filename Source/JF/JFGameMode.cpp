// Copyright Epic Games, Inc. All Rights Reserved.

#include "JFGameMode.h"
#include "JFCharacter.h"
#include "JFPlayerController.h"
#include "Game/JFPlayerState.h"
#include "Helpers/BlueprintTravel.h"
#include "UObject/ConstructorHelpers.h"

AJFGameMode::AJFGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}

	bStartPlayersAsSpectators = true;
	bUseSeamlessTravel = true;
}

UClass* AJFGameMode::GetDefaultPawnClassForController_Implementation(AController* InController)
{
	if(InController->GetNetMode() == NM_Standalone) return DefaultPawnClass;
	return nullptr;
}

void AJFGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);
	
	AJFPlayerController* PC = Cast<AJFPlayerController>(NewPlayer);
	AJFPlayerState* PS = NewPlayer->GetPlayerState<AJFPlayerState>();
	
	UE_LOG(LogTemp, Log, TEXT("[GM/PL] Spawning Character for %s"), *NewPlayer->GetName());

	if (PC && PS && PS->GetHero() && PS->GetHero()->Actor)
	{
		//Spawning Character
		UE_LOG(LogTemp, Log, TEXT("[GM/PL] Valid Player State and Hero Data"));
		
		UClass* HeroClass = PS->GetHero()->Actor.LoadSynchronous();
		
		if(HeroClass) {
			PlayerSpawnCount++;
			AActor* StartSpot = FindPlayerStart(PC, FString::FromInt(PlayerSpawnCount));
			FTransform SpawnTransform = StartSpot->GetActorTransform();

			FActorSpawnParameters Params;
			Params.Owner = PC;
			Params.Instigator = PC->GetPawn();

			UE_LOG(LogTemp, Log, TEXT("[GM/PL] Valid Hero Class / Hero Class Loaded"));

			APawn* NewPawn = GetWorld()->SpawnActor<APawn>(HeroClass, SpawnTransform, Params);
			if (NewPawn)
			{
				UE_LOG(LogTemp, Log, TEXT("[GM/PL] Player Posssesed %s"), *NewPawn->GetName());
				PC->Possess(NewPawn);
			}
		}
	}
	else
	{
		if(!PC)
		{
			UE_LOG(LogTemp, Error, TEXT("[GM/PL] Player Controller is Invalid"));
		}
		else if(!PS)
		{
			UE_LOG(LogTemp, Error, TEXT("[GM/PL] Player State is Invalid"));
		}
		else
		{
			if(!PS->GetHero())
			{
				UE_LOG(LogTemp, Error, TEXT("[GM/PL] Player Hero Data is Invalid"));
			}
			else
			{
				if(!PS->GetHero()->Actor)
				{
					UE_LOG(LogTemp, Error, TEXT("[GM/PL] Player Hero Data | Actor is Invalid"));
				}
			}
		}
	}
}

void AJFGameMode::InitSeamlessTravelPlayer(AController* NewController)
{
	Super::InitSeamlessTravelPlayer(NewController);
	AJFPlayerController* PC = Cast<AJFPlayerController>(NewController);
	if(PC)
	{
		UE_LOG(LogTemp, Log, TEXT("[GM/ISTP] Init'n ServerTravel Player -> PostLogining"));
		PostLogin(PC);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("[GM/ISTP] Player Controller Cast Failed or Invalid"));
	}
}

void AJFGameMode::PlayerKilled_Implementation(AJFCharacter* Killer, AJFCharacter* Killed)
{
	//Reload back to lobby
	//Reset Characters Selected
	//Add To Win Counter
	AJFPlayerState* KillerPS = Killer->GetPlayerState<AJFPlayerState>();
	AJFPlayerState* KilledPS = Killed->GetPlayerState<AJFPlayerState>();

	if(KillerPS && KilledPS)
	{
		KillerPS->addKD(1, 0);
		KilledPS->addKD(0, 1);

		KillerPS->SetHero(nullptr);
		KilledPS->SetHero(nullptr);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Cannot Add KD/A for Users, Invalid Player States."));
	}

	GetWorld()->ServerTravel(LevelURL + "?listen",true,false);
}
