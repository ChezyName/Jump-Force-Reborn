// Copyright ChezyName. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "HeroData.h"
#include "Engine/DataAsset.h"
#include "JFRoster.generated.h"

/**
 * 
 */
UCLASS(BlueprintType)
class JF_API UJFRoster : public UPrimaryDataAsset
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<TSoftObjectPtr<UHeroData>> Characters;
};
