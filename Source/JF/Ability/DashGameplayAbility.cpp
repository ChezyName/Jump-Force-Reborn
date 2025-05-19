// Copyright ChezyName. All Rights Reserved.


#include "DashGameplayAbility.h"

#include "JF/JFCharacter.h"

bool UDashGameplayAbility::CheckCost(const FGameplayAbilitySpecHandle Handle,
                                     const FGameplayAbilityActorInfo* ActorInfo, FGameplayTagContainer* OptionalRelevantTags) const
{
	AJFCharacter* Char = Cast<AJFCharacter>(ActorInfo->OwnerActor);
	if(Char)
	{
		const float Charge = Char->GetNumericAttribute(UJFAttributeSet::GetDashChargeAttribute());
		return Charge >= 100.f; //Each 100 is one charge
	}
	
	return Super::CheckCost(Handle, ActorInfo, OptionalRelevantTags);
}

bool UDashGameplayAbility::CommitAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	FGameplayTagContainer* OptionalRelevantTags)
{
	//Take Dash from Character
	AJFCharacter* Char = Cast<AJFCharacter>(ActorInfo->OwnerActor);
	if(Char)
	{
		const float Charge = Char->GetNumericAttribute(UJFAttributeSet::GetDashChargeAttribute());
		
		Char->SetNumericAttribute(UJFAttributeSet::GetDashChargeAttribute(),
			Charge - 100.f);

		return true;
	}
	return Super::CommitAbility(Handle, ActorInfo, ActivationInfo, OptionalRelevantTags);
}
