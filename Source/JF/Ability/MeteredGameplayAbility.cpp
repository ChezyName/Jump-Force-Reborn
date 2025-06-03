// Copyright ChezyName. All Rights Reserved.


#include "MeteredGameplayAbility.h"

#include "AbilitySystemLog.h"
#include "JF/JFCharacter.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Net/UnrealNetwork.h"

UMeteredGameplayAbility::UMeteredGameplayAbility()
{
	bPlayGruntSound = false;
}

bool UMeteredGameplayAbility::CheckCost(const FGameplayAbilitySpecHandle Handle,
                                        const FGameplayAbilityActorInfo* ActorInfo, FGameplayTagContainer* OptionalRelevantTags) const
{
	if(isActive && isToggledAbility) return true;
	
	if(AJFCharacter* Char = Cast<AJFCharacter>(ActorInfo->OwnerActor))
	{
		return Char->GetMeter() >= (AbilityCost * 100.f);
	}
	
	return Super::CheckCost(Handle, ActorInfo, OptionalRelevantTags);
}

bool UMeteredGameplayAbility::CommitAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	FGameplayTagContainer* OptionalRelevantTags)
{
	if(isActive && isToggledAbility) return true;
	
	//Take Meter from Character
	AJFCharacter* Char = Cast<AJFCharacter>(ActorInfo->OwnerActor);
	
	Character = Char;
	if(!Character->IsValidLowLevel()) CancelAbility(Handle, ActorInfo, ActivationInfo, true);
	
	if(Character)
	{
		if(!TakesCostOnUse) return true;
		
		float MeterFull = Character->GetMeter();
		MeterFull -= AbilityCost * 100.f;
		
		Character->SetNumericAttribute(UJFAttributeSet::GetMeterAttribute(),
			MeterFull);
		
		return true;
	}
	
	return Super::CommitAbility(Handle, ActorInfo, ActivationInfo, OptionalRelevantTags);
}

void UMeteredGameplayAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	if(isActive && isToggledAbility) {
		UKismetSystemLibrary::PrintString(GetWorld(), "End The Ability");
		isActive = false;
		K2_ActivateAbility();
		return;
	}
	
	if(!Character->IsValidLowLevel())
	{
		Character = Cast<AJFCharacter>(ActorInfo->OwnerActor);
	}

	//Last Check for Character -> Cancel if Invalid
	if(!Character->IsValidLowLevel())
	{
		K2_CancelAbility();
		return;
	}

	K2_CommitAbility();
	
	LastTime = GetWorld()->GetTimeSeconds();
	isKeyHeld = true;
	isActive = true;
	UKismetSystemLibrary::PrintString(GetWorld(), "Starting The Ability");

	callTickEvent = !isToggledAbility;
	if(isToggledAbility)
	{
		TickTask = UHitboxTask::CreateHitboxTicker(this);
		TickTask->OnTick.AddDynamic(this, &UMeteredGameplayAbility::onTickSelf);
		TickTask->ReadyForActivation();
	}
	
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
}

void UMeteredGameplayAbility::onTickSelf()
{
	const float CurrentTime = GetWorld()->GetTimeSeconds();
	DeltaTime = CurrentTime - LastTime;
	LastTime = CurrentTime;

	UKismetSystemLibrary::PrintString(GetWorld(), "Tick", true, true, FLinearColor::Yellow,
		2, FName("TICK CLOCK"));

	if(Character)
	{
		//Take Meter
		float Meter = Character->GetNumericAttribute(UJFAttributeSet::GetMeterAttribute());
		if(ForceEndOnMeterZero && Meter <= 0)
		{
			if(isToggledAbility)  K2_ActivateAbility();
			else  K2_EndAbility();
		}

		Meter -= DeltaTime * MeterPerSecond;
		Meter = FMath::Clamp(Meter, 0, MAX_METER);

		Character->SetNumericAttribute(UJFAttributeSet::GetMeterAttribute(), Meter);
	}

	Super::onTick();
}

void UMeteredGameplayAbility::InputReleased(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo)
{
	UKismetSystemLibrary::PrintString(GetWorld(), "Input Released");
	
	ClientKeyReleased();
	ServerKeyReleased();
	
	Super::InputReleased(Handle, ActorInfo, ActivationInfo);
}

void UMeteredGameplayAbility::ClientKeyReleased()
{
	if(EndAbilityOnKeyRelease) K2_EndAbility();
	isKeyHeld = false;
}

void UMeteredGameplayAbility::ServerKeyReleased_Implementation()
{
	if(EndAbilityOnKeyRelease) K2_EndAbility();
	isKeyHeld = false;
}
