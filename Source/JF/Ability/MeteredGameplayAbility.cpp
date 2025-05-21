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
	
	Character = Char;
	if(!Character->IsValidLowLevel()) CancelAbility(Handle, ActorInfo, ActivationInfo, true);
	
	if(Char)
	{
		float MeterFull = Char->GetMeter();
		MeterFull -= AbilityCost * 100.f;
		
		Char->SetNumericAttribute(UJFAttributeSet::GetMeterAttribute(),
			MeterFull);

		isKeyHeld = true;
		Character->AbilitySystemComponent->AbilityReleasedEvent.AddDynamic(this,
			&UMeteredGameplayAbility::onKeyReleased);

		return true;
	}
	
	return Super::CommitAbility(Handle, ActorInfo, ActivationInfo, OptionalRelevantTags);
}

void UMeteredGameplayAbility::EndAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility, bool bWasCancelled)
{
	if(Character)
	{
		Character->AbilitySystemComponent->AbilityReleasedEvent.RemoveDynamic(this,
			&UMeteredGameplayAbility::onKeyReleased);
	}
	
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UMeteredGameplayAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	if(!Character->IsValidLowLevel())
	{
		Character = Cast<AJFCharacter>(ActorInfo->OwnerActor);
	}
	
	LastTime = GetWorld()->GetTimeSeconds();

	K2_CommitAbility();
	
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
}

void UMeteredGameplayAbility::onTick()
{
	const float CurrentTime = GetWorld()->GetTimeSeconds();
	DeltaTime = CurrentTime - LastTime;
	LastTime = CurrentTime;

	if(Character)
	{
		//Take Meter
		float Meter = Character->GetNumericAttribute(UJFAttributeSet::GetMeterAttribute());
		if(ForceEndOnMeterZero && Meter <= 0) K2_EndAbility();

		Meter -= DeltaTime * MeterPerSecond;
		Meter = FMath::Clamp(Meter, 0, MAX_METER);

		Character->SetNumericAttribute(UJFAttributeSet::GetMeterAttribute(), Meter);
	
		//UKismetSystemLibrary::PrintString(GetWorld(), "Ability - Tick");
	}
}
