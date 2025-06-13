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
	ServerDoTimestop(_TimeStopper, true);
}

void AJFGameState::ResumeTime()
{
	ServerDoTimestop(nullptr, false);
}

void AJFGameState::ServerDoTimestop_Implementation(AJFCharacter* _TimeStopper, bool Timestopping)
{
	if(bIsTimeStopped && !Timestopping)
	{
		//Resume Time
		bIsTimeStopped = false;
		RepTimeStop(bIsTimeStopped, TimeStopper);
		TimeStopper = nullptr;
	}
	else if(Timestopping)
	{
		bIsTimeStopped = true;
		TimeStopper = _TimeStopper;
		RepTimeStop(bIsTimeStopped, TimeStopper);
	}
}

void AJFGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	DOREPLIFETIME(AJFGameState, bIsTimeStopped);
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
}
