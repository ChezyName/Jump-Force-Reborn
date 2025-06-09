// Copyright ChezyName. All Rights Reserved.

#include "JFGameInstance.h"
#include "GameplayTags.h"

FGameplayTag UJFGameInstance::DoingSomethingTag;
FGameplayTag UJFGameInstance::ParryTag;
FGameplayTag UJFGameInstance::CantMoveTag;
FGameplayTag UJFGameInstance::ParryStunTag;
FGameplayTag UJFGameInstance::HitStunTag;
FGameplayTag UJFGameInstance::GAHitStunTag;
FGameplayTag UJFGameInstance::GrabbedTag;

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
	Super::Init();
}
