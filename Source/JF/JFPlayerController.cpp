// Copyright ChezyName. All Rights Reserved.


#include "JFPlayerController.h"

#include "JFCharacter.h"

void AJFPlayerController::AcknowledgePossession(APawn* P)
{
	Super::AcknowledgePossession(P);

	AJFCharacter* CharacterBase = Cast<AJFCharacter>(P);
	if (CharacterBase)
	{
		CharacterBase->GetAbilitySystemComponent()->InitAbilityActorInfo(CharacterBase, CharacterBase);
	}
}
