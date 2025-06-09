// Copyright ChezyName. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "JFPlayerState.generated.h"

/**
 * 
 */
UCLASS()
class JF_API AJFPlayerState : public APlayerState
{
	GENERATED_BODY()
private:
	UPROPERTY(Replicated)
	FString Username;
public:
	virtual void CopyProperties(APlayerState* PlayerState) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(BlueprintCallable)
	void SetUsername(FString NewUsername) { Username = NewUsername;}

	UFUNCTION(BlueprintPure)
	FString GetUsername(){return Username;}
};
