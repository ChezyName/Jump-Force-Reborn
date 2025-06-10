// Copyright ChezyName. All Rights Reserved.


#include "JFPlayerState.h"

#include "Net/UnrealNetwork.h"

void AJFPlayerState::CopyProperties(APlayerState* PlayerState)
{
	if(AJFPlayerState* PS = Cast<AJFPlayerState>(PlayerState))
	{
		PS->SetUsername(Username);
		PS->SetHero(Hero);
	}
	
	Super::CopyProperties(PlayerState);
}

void AJFPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	DOREPLIFETIME(AJFPlayerState, Username);
	DOREPLIFETIME(AJFPlayerState, Hero);
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
}
