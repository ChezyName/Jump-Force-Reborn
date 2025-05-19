// Copyright ChezyName. All Rights Reserved.


#include "MeteredGameplayAbility.h"

#include "JF/JFCharacter.h"
#include "Kismet/KismetSystemLibrary.h"

bool UMeteredGameplayAbility::CheckCost(const FGameplayAbilitySpecHandle Handle,
                                        const FGameplayAbilityActorInfo* ActorInfo, FGameplayTagContainer* OptionalRelevantTags) const
{
	AJFCharacter* Char = Cast<AJFCharacter>(ActorInfo->OwnerActor);
	if(Char)
	{
		return Char->GetMeterText() >= AbilityCost;
	}
	
	return Super::CheckCost(Handle, ActorInfo, OptionalRelevantTags);
}

bool UMeteredGameplayAbility::CommitAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	FGameplayTagContainer* OptionalRelevantTags)
{
	//Take Meter from Character
	AJFCharacter* Char = Cast<AJFCharacter>(ActorInfo->OwnerActor);
	if(Char)
	{
		float MeterFull = Char->GetMeter();
		MeterFull -= AbilityCost * 100.f;
		
		Char->SetNumericAttribute(UJFAttributeSet::GetMeterAttribute(),
			MeterFull);

		return true;
	}
	
	return Super::CommitAbility(Handle, ActorInfo, ActivationInfo, OptionalRelevantTags);
}