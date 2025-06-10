// Copyright ChezyName. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "BlueprintTravel.generated.h"

/**
 * 
 */
UCLASS()
class JF_API UBlueprintTravel : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintCallable, meta=(WorldContext="WorldContextObject", AdvancedDisplay = "2", DisplayName = "ServerTravel"), Category="Game")
	static void SeamlessTravel(const UObject* WorldContextObject,const TSoftObjectPtr<UWorld> Level);
};
