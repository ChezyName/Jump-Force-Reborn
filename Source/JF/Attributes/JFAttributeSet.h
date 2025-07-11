﻿// Copyright ChezyName. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "JFAttributeSet.generated.h"

#define ATTRIBUTE_ACCESSOR(AttributeSetClass, AttributeName) \
GAMEPLAYATTRIBUTE_PROPERTY_GETTER(AttributeSetClass, AttributeName) \
GAMEPLAYATTRIBUTE_VALUE_GETTER(AttributeName) \
GAMEPLAYATTRIBUTE_VALUE_SETTER(AttributeName) \
GAMEPLAYATTRIBUTE_VALUE_INITTER(AttributeName)


/**
 * 
 */
UCLASS()
class JF_API UJFAttributeSet : public UAttributeSet
{
	GENERATED_BODY()
public:
	UJFAttributeSet();

protected:
	UFUNCTION()
	void onHealthChanged(const FGameplayAttributeData& OldHealth) {
		GAMEPLAYATTRIBUTE_REPNOTIFY(UJFAttributeSet, Health, OldHealth)
	}
	UFUNCTION()
	void onMaxHealthChanged(const FGameplayAttributeData& OldMaxHealth) {
		GAMEPLAYATTRIBUTE_REPNOTIFY(UJFAttributeSet, MaxHealth, OldMaxHealth)
	}
	UFUNCTION()
	void onSpeedChanged(const FGameplayAttributeData& OldSpeed)
	{
		GAMEPLAYATTRIBUTE_REPNOTIFY(UJFAttributeSet, MovementSpeed, OldSpeed)
	}
	UFUNCTION()
	void onLightAttackComboChanged(const FGameplayAttributeData& OldAttackCombo)
	{
		GAMEPLAYATTRIBUTE_REPNOTIFY(UJFAttributeSet, LightAttackCombo, OldAttackCombo)
	}
	UFUNCTION()
	void onHeavyAttackComboChanged(const FGameplayAttributeData& OldAttackCombo)
	{
		GAMEPLAYATTRIBUTE_REPNOTIFY(UJFAttributeSet, HeavyAttackCombo, OldAttackCombo)
	}
	UFUNCTION()
	void onComboResetChanged(const FGameplayAttributeData& OldAttackReset)
	{
		GAMEPLAYATTRIBUTE_REPNOTIFY(UJFAttributeSet, ComboResetTime, OldAttackReset)
	}
	UFUNCTION()
	void onMeterChanged(const FGameplayAttributeData& OldMeter)
	{
		GAMEPLAYATTRIBUTE_REPNOTIFY(UJFAttributeSet, Meter, OldMeter)
	}
	UFUNCTION()
	void onDashChargeChanged(const FGameplayAttributeData& OldDashCharge)
	{
		GAMEPLAYATTRIBUTE_REPNOTIFY(UJFAttributeSet, DashCharge, OldDashCharge)
	}
	
	UPROPERTY(BlueprintReadOnly, Category="Character | Health & Speed", ReplicatedUsing=onHealthChanged)
	FGameplayAttributeData Health;
	
	UPROPERTY(BlueprintReadOnly, Category="Character | Health & Speed", ReplicatedUsing=onMaxHealthChanged)
	FGameplayAttributeData MaxHealth;

	UPROPERTY(BlueprintReadOnly, Category="Character | Health & Speed", Replicated, ReplicatedUsing=onSpeedChanged)
	FGameplayAttributeData MovementSpeed;

	UPROPERTY(BlueprintReadOnly, Category="Character | Attacks", Replicated, ReplicatedUsing=onLightAttackComboChanged)
	FGameplayAttributeData LightAttackCombo;

	UPROPERTY(BlueprintReadOnly, Category="Character | Attacks", Replicated, ReplicatedUsing=onHeavyAttackComboChanged)
	FGameplayAttributeData HeavyAttackCombo;
	
	UPROPERTY(BlueprintReadOnly, Category="Character | Attacks", Replicated, ReplicatedUsing=onComboResetChanged)
	FGameplayAttributeData ComboResetTime;

	UPROPERTY(BlueprintReadOnly, Category="Character | Attacks", Replicated, ReplicatedUsing=onMeterChanged)
	FGameplayAttributeData Meter;
	
	UPROPERTY(BlueprintReadOnly, Category="Character | Attacks", Replicated, ReplicatedUsing=onDashChargeChanged)
	FGameplayAttributeData DashCharge;
public:
	ATTRIBUTE_ACCESSOR(UJFAttributeSet, Health);
	ATTRIBUTE_ACCESSOR(UJFAttributeSet, MaxHealth);
	ATTRIBUTE_ACCESSOR(UJFAttributeSet, MovementSpeed);
	ATTRIBUTE_ACCESSOR(UJFAttributeSet, LightAttackCombo);
	ATTRIBUTE_ACCESSOR(UJFAttributeSet, HeavyAttackCombo);
	ATTRIBUTE_ACCESSOR(UJFAttributeSet, ComboResetTime)
	ATTRIBUTE_ACCESSOR(UJFAttributeSet, Meter);
	ATTRIBUTE_ACCESSOR(UJFAttributeSet, DashCharge);

	
	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};
