// Copyright ChezyName. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Engine/GameInstance.h"
#include "GameFramework/SaveGame.h"
#include "JFGameInstance.generated.h"

/**
 * 
 */
UCLASS()
class JF_API UJFGameInstance : public UGameInstance
{
	GENERATED_BODY()
private:
	FString Username;
public:
	static FGameplayTag DoingSomethingTag; // FGameplayTag::RequestGameplayTag(FName("Character.Status.DoingSomething"));
	static FGameplayTag ParryTag;			// FGameplayTag::RequestGameplayTag(FName("Character.Attacking.Parrying"));
	static FGameplayTag CantMoveTag;		// FGameplayTag::RequestGameplayTag(FName("Character.Status.CantMove"));
	static FGameplayTag ParryStunTag;		// FGameplayTag::RequestGameplayTag(FName("GameplayCue.ParryStun"));
	static FGameplayTag HitStunTag;		// FGameplayTag::RequestGameplayTag(FName("GameplayCue.HitStun"));
	static FGameplayTag GAHitStunTag;		// FGameplayTag::RequestGameplayTag(FName("Character.HitStun"));
	static FGameplayTag GrabbedTag;		// FGameplayTag::RequestGameplayTag(FName("Character.Status.Grabbed"));

	UFUNCTION(BlueprintCallable)
	void SetUsername(FString NewUsername) { Username = NewUsername; Save();}

	UFUNCTION(BlueprintPure)
	FString GetUsername(){return Username;}

	void Save();

	void Load();

	virtual void Init() override;

private:
	FString SaveSlot = TEXT("UserSave");
	int32 UserIndex = 0;
};


UCLASS()
class JF_API UUsernameSaveGame : public USaveGame
{
	GENERATED_BODY()

public:
	UPROPERTY(VisibleAnywhere)
	FString Username;
};
