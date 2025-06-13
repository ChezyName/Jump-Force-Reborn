// Copyright ChezyName. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "JFGameState.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FTimeStopEvent,bool, isTimeStopped , AJFCharacter*, TimeStopper);

/**
 * 
 */
UCLASS()
class JF_API AJFGameState : public AGameStateBase
{
	GENERATED_BODY()
public:
	FTimeStopEvent TimeStopEvent;

	UFUNCTION(BlueprintCallable)
	void StopTime(AJFCharacter* _TimeStopper);
	
	UFUNCTION(BlueprintCallable)
	void ResumeTime();

	UFUNCTION(BlueprintPure)
	bool IsTimeStopped()
	{
		return bIsTimeStopped;
	}
private:
	UPROPERTY()
	AJFCharacter* TimeStopper;
	
	UPROPERTY(Replicated)
	bool bIsTimeStopped = false;

	UFUNCTION(Server, Reliable)
	void ServerDoTimestop(AJFCharacter* _TimeStopper, bool Timestopping = false);
	
	UFUNCTION(NetMulticast, Reliable)
	void RepTimeStop(bool isTimeStopped, AJFCharacter* _TimeStopper);

protected:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>&) const override;
};
