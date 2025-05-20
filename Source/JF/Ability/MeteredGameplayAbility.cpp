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

		return true;
	}
	
	return Super::CommitAbility(Handle, ActorInfo, ActivationInfo, OptionalRelevantTags);
}

void UMeteredGameplayAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	if(!Character->IsValidLowLevel())
	{
		Character = Cast<AJFCharacter>(ActorInfo->OwnerActor);
	}
	
	if(MeterPerSecond > 0)
	{
		TickTask = UHitboxTask::CreateHitboxTicker(this);
		TickTask->OnTick.AddDynamic(this, &UMeteredGameplayAbility::onTick);
		TickTask->ReadyForActivation();

		LastTime = GetWorld()->GetTimeSeconds();
	}

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
		if(Meter <= 0) K2_EndAbility();

		Meter -= DeltaTime * MeterPerSecond;

		Character->SetNumericAttribute(UJFAttributeSet::GetMeterAttribute(), Meter);
	
		//UKismetSystemLibrary::PrintString(GetWorld(), "Ability - Tick");
	}
}
