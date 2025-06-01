// Copyright ChezyName. All Rights Reserved.


#include "JFGameState.h"

#include "Net/UnrealNetwork.h"


void AJFGameState::RepTimeStop_Implementation(bool isTimeStopped, AJFCharacter* _TimeStopper)
{
	bIsTimeStopped = isTimeStopped;
	TimeStopEvent.Broadcast(isTimeStopped, _TimeStopper);
}

void AJFGameState::StopTime(AJFCharacter* _TimeStopper)
{
	if(bIsTimeStopped) return;

	bIsTimeStopped = true;
	TimeStopper = _TimeStopper;
	RepTimeStop(bIsTimeStopped, TimeStopper);
}

void AJFGameState::ResumeTime()
{
	if(!bIsTimeStopped) return;

	bIsTimeStopped = false;
	RepTimeStop(bIsTimeStopped, TimeStopper);
	TimeStopper = nullptr;
}

void AJFGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	DOREPLIFETIME(AJFGameState, bIsTimeStopped);
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
}
