// Copyright ChezyName. All Rights Reserved.


#include "JFGameState.h"

#include "Kismet/KismetSystemLibrary.h"
#include "Net/UnrealNetwork.h"


void AJFGameState::RepTimeStop_Implementation(bool isTimeStopped, AJFCharacter* _TimeStopper)
{
	bIsTimeStopped = isTimeStopped;
	TimeStopEvent.Broadcast(isTimeStopped, _TimeStopper);
}

void AJFGameState::StopTime(AJFCharacter* _TimeStopper)
{
	if(bIsTimeStopped) return;

	UKismetSystemLibrary::PrintString(GetWorld(),"Time Has Been Stopped", true,
true, FLinearColor::Yellow, 60, FName("TimeStopServer"));

	if(!HasAuthority()) return;
	
	bIsTimeStopped = true;
	TimeStopper = _TimeStopper;
	RepTimeStop(bIsTimeStopped, TimeStopper);
}

void AJFGameState::ResumeTime()
{
	if(!bIsTimeStopped) return;

	UKismetSystemLibrary::PrintString(GetWorld(),"Time Has Been Resumed", true,
true, FLinearColor::Yellow, 60, FName("TimeStopServer"));

	if(!HasAuthority()) return;

	bIsTimeStopped = false;
	RepTimeStop(bIsTimeStopped, TimeStopper);
	TimeStopper = nullptr;
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
