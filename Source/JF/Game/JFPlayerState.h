// Copyright ChezyName. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "JF/Class/HeroData.h"
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

	UPROPERTY(Replicated)
	UHeroData* Hero;
public:
	virtual void CopyProperties(APlayerState* PlayerState) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(BlueprintCallable)
	void SetUsername(FString NewUsername) { Username = NewUsername;}

	UFUNCTION(BlueprintPure)
	FString GetUsername(){return Username;}

	UFUNCTION(BlueprintCallable)
	void SetHero(UHeroData* NewHero)
	{
		Hero = NewHero;
	}

	UFUNCTION(BlueprintPure)
	UHeroData* GetHero(){ return Hero;}
};
