// Copyright ChezyName. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Engine/GameInstance.h"
#include "JFGameInstance.generated.h"

/**
 * 
 */
UCLASS()
class JF_API UJFGameInstance : public UGameInstance
{
	GENERATED_BODY()
public:
	static FGameplayTag DoingSomethingTag; // FGameplayTag::RequestGameplayTag(FName("Character.Status.DoingSomething"));
	static FGameplayTag ParryTag;			// FGameplayTag::RequestGameplayTag(FName("Character.Attacking.Parrying"));
	static FGameplayTag CantMoveTag;		// FGameplayTag::RequestGameplayTag(FName("Character.Status.CantMove"));
	static FGameplayTag ParryStunTag;		// FGameplayTag::RequestGameplayTag(FName("GameplayCue.ParryStun"));
	static FGameplayTag HitStunTag;		// FGameplayTag::RequestGameplayTag(FName("GameplayCue.HitStun"));
	static FGameplayTag GAHitStunTag;		// FGameplayTag::RequestGameplayTag(FName("Character.HitStun"));
	static FGameplayTag GrabbedTag;		// FGameplayTag::RequestGameplayTag(FName("Character.Status.Grabbed"));


	virtual void Init() override;
};
