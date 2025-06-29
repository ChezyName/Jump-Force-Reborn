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

	UPROPERTY(Replicated)
	int Kills;
	
	UPROPERTY(Replicated)
	int Deaths;
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

	void setKD(int kills = 0, int deaths = 0)
	{
		Kills = kills;
		Deaths = deaths;
	}
	
	void addKD(int kills = 0, int deaths = 0)
	{
		Kills = kills;
		Deaths = deaths;
	}

	UFUNCTION(BlueprintPure)
	int GetKills() { return Kills; }
	UFUNCTION(BlueprintPure)
	int GetDeaths() { return Deaths; }
	UFUNCTION(BlueprintPure)
	FString GetKD() { return FString::Printf(TEXT("%d/%d"), GetKills(), GetDeaths()); }

	UFUNCTION(BlueprintPure)
	UHeroData* GetHero(){ return Hero;}
};
