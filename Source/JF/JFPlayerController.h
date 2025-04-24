// Copyright ChezyName. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "JFPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class JF_API AJFPlayerController : public APlayerController
{
	GENERATED_BODY()
	
	virtual void AcknowledgePossession(APawn* P) override;
};
