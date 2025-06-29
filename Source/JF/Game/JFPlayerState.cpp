// Copyright ChezyName. All Rights Reserved.


#include "JFPlayerState.h"

#include "Net/UnrealNetwork.h"

void AJFPlayerState::CopyProperties(APlayerState* PlayerState)
{
	if(AJFPlayerState* PS = Cast<AJFPlayerState>(PlayerState))
	{
		PS->SetUsername(Username);
		PS->SetHero(Hero);

		PS->setKD(Kills, Deaths);
		
		UE_LOG(LogTemp, Log, TEXT("JFPS Properties Copied"));
	}
	else UE_LOG(LogTemp, Warning, TEXT("JFPS Properties Failed to Copy"));
	
	Super::CopyProperties(PlayerState);
}

void AJFPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	DOREPLIFETIME(AJFPlayerState, Username);
	DOREPLIFETIME(AJFPlayerState, Hero);
	DOREPLIFETIME(AJFPlayerState, Kills);
	DOREPLIFETIME(AJFPlayerState, Deaths);
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
}
