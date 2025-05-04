// Copyright ChezyName. All Rights Reserved.


#include "JFAttributeSet.h"

#include "Net/UnrealNetwork.h"

UJFAttributeSet::UJFAttributeSet(): Super() {}

void UJFAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);

	if(Attribute == GetHealthAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0, GetMaxHealth());
	}
}

void UJFAttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UJFAttributeSet, Health);
	DOREPLIFETIME(UJFAttributeSet, MaxHealth);
	DOREPLIFETIME(UJFAttributeSet, MovementSpeed);

	DOREPLIFETIME(UJFAttributeSet, LightAttackCombo);
	DOREPLIFETIME(UJFAttributeSet, HeavyAttackCombo);
	
	DOREPLIFETIME(UJFAttributeSet, ComboResetTime);
}
