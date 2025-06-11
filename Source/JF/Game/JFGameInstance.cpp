// Copyright ChezyName. All Rights Reserved.

#include "JFGameInstance.h"
#include "GameplayTags.h"
#include "Kismet/GameplayStatics.h"

FGameplayTag UJFGameInstance::DoingSomethingTag;
FGameplayTag UJFGameInstance::ParryTag;
FGameplayTag UJFGameInstance::CantMoveTag;
FGameplayTag UJFGameInstance::ParryStunTag;
FGameplayTag UJFGameInstance::HitStunTag;
FGameplayTag UJFGameInstance::GAHitStunTag;
FGameplayTag UJFGameInstance::GrabbedTag;

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
	ParryStunTag = FGameplayTag::RequestGameplayTag(FName("GameplayCue.ParryStun"));
	HitStunTag = FGameplayTag::RequestGameplayTag(FName("GameplayCue.HitStun"));
	GAHitStunTag = FGameplayTag::RequestGameplayTag(FName("Character.HitStun"));
	GrabbedTag = FGameplayTag::RequestGameplayTag(FName("Character.Status.Grabbed"));
	
	Load();
	
	Super::Init();
}
