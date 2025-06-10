// Copyright ChezyName. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "JF/Ability/AbilityData.h"
#include "HeroData.generated.h"

/**
 * 
 */
UCLASS(BlueprintType)
class JF_API UHeroData : public UPrimaryDataAsset
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FString Name;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FString Desc;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<TSoftObjectPtr<UAbilityData>> Abilities;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSoftClassPtr<AActor> Actor;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, BlueprintReadOnly)
	UTexture2D* Portrait;
};
