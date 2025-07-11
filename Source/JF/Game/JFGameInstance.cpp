﻿// Copyright ChezyName. All Rights Reserved.

#include "JFGameInstance.h"
#include "GameplayTags.h"
#include "Kismet/GameplayStatics.h"

FGameplayTag UJFGameInstance::DoingSomethingTag;
FGameplayTag UJFGameInstance::ParryTag;
FGameplayTag UJFGameInstance::CantMoveTag;
FGameplayTag UJFGameInstance::CantLockTag;
FGameplayTag UJFGameInstance::ParryStunTag;
FGameplayTag UJFGameInstance::HitStunTag;
FGameplayTag UJFGameInstance::GAHitStunTag;
FGameplayTag UJFGameInstance::GrabbedTag;
FGameplayTag UJFGameInstance::TimestopTag;
FGameplayTag UJFGameInstance::HyperArmorTag;
FGameplayTag UJFGameInstance::ParryArmorTag;
FGameplayTag UJFGameInstance::NoMeterTag;

void UJFGameInstance::Save()
{
	auto SaveGame = Cast<UUsernameSaveGame>(UGameplayStatics::CreateSaveGameObject(UUsernameSaveGame::StaticClass()));
	SaveGame->Username = Username;
	UGameplayStatics::SaveGameToSlot(SaveGame, SaveSlot, UserIndex);
}

void UJFGameInstance::Load()
{
	if (UGameplayStatics::DoesSaveGameExist(SaveSlot, UserIndex))
	{
		auto LoadedGame = Cast<UUsernameSaveGame>(UGameplayStatics::LoadGameFromSlot(SaveSlot, UserIndex));
		Username = LoadedGame->Username;
	}
	else
	{
		FMath::RandInit(FDateTime::Now().GetTicks());
		const int32 RandomNumber = FMath::RandRange(11111, 99999);
		Username = FString::Printf(TEXT("User%d"), RandomNumber);

	}
}

void UJFGameInstance::Init()
{
	UE_LOG(LogTemp, Warning, TEXT("UJFGameInstance::Init called"));

	DoingSomethingTag = FGameplayTag::RequestGameplayTag(FName("Character.Status.DoingSomething"));
	ParryTag = FGameplayTag::RequestGameplayTag(FName("Character.Attacking.Parrying"));
	CantMoveTag = FGameplayTag::RequestGameplayTag(FName("Character.Status.CantMove"));
	CantLockTag = FGameplayTag::RequestGameplayTag(FName("Character.Status.CantLock"));
	ParryStunTag = FGameplayTag::RequestGameplayTag(FName("GameplayCue.ParryStun"));
	HitStunTag = FGameplayTag::RequestGameplayTag(FName("GameplayCue.HitStun"));
	GAHitStunTag = FGameplayTag::RequestGameplayTag(FName("Character.HitStun"));
	GrabbedTag = FGameplayTag::RequestGameplayTag(FName("Character.Status.Grabbed"));
	TimestopTag = FGameplayTag::RequestGameplayTag(FName("Character.Status.TimeStopped"));
	HyperArmorTag = FGameplayTag::RequestGameplayTag(FName("Character.Status.HyperArmor"));
	ParryArmorTag = FGameplayTag::RequestGameplayTag(FName("Character.Status.ParryArmor"));
	NoMeterTag = FGameplayTag::RequestGameplayTag(FName("Character.Status.NoMeter"));
	
	Load();
	
	Super::Init();
}
