// Copyright ChezyName. All Rights Reserved.

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
	void onLightAttackResetChanged(const FGameplayAttributeData& OldAttackReset)
	{
		GAMEPLAYATTRIBUTE_REPNOTIFY(UJFAttributeSet, LightAttackComboResetTime, OldAttackReset)
	}
	UFUNCTION()
	void onHeavyAttackResetChanged(const FGameplayAttributeData& OldAttackReset)
	{
		GAMEPLAYATTRIBUTE_REPNOTIFY(UJFAttributeSet, HeavyAttackComboResetTime, OldAttackReset)
	}
	

public:
	UPROPERTY(BlueprintReadOnly, Category="Character | Health & Speed", ReplicatedUsing=onHealthChanged)
	FGameplayAttributeData Health;
	ATTRIBUTE_ACCESSOR(UJFAttributeSet, Health);
	
	UPROPERTY(BlueprintReadOnly, Category="Character | Health & Speed", ReplicatedUsing=onHealthChanged)
	FGameplayAttributeData MaxHealth;
	ATTRIBUTE_ACCESSOR(UJFAttributeSet, MaxHealth);

	UPROPERTY(BlueprintReadOnly, Category="Character | Health & Speed", Replicated, ReplicatedUsing=onSpeedChanged)
	FGameplayAttributeData MovementSpeed;
	ATTRIBUTE_ACCESSOR(UJFAttributeSet, MovementSpeed);

	UPROPERTY(BlueprintReadOnly, Category="Character | Attacks", Replicated, ReplicatedUsing=onSpeedChanged)
	FGameplayAttributeData LightAttackCombo;
	ATTRIBUTE_ACCESSOR(UJFAttributeSet, LightAttackCombo);
	
	UPROPERTY(BlueprintReadOnly, Category="Character | Attacks", Replicated, ReplicatedUsing=onSpeedChanged)
	FGameplayAttributeData LightAttackComboResetTime;
	ATTRIBUTE_ACCESSOR(UJFAttributeSet, LightAttackComboResetTime);

	UPROPERTY(BlueprintReadOnly, Category="Character | Attacks", Replicated, ReplicatedUsing=onSpeedChanged)
	FGameplayAttributeData HeavyAttackCombo;
	ATTRIBUTE_ACCESSOR(UJFAttributeSet, HeavyAttackCombo);
	
	UPROPERTY(BlueprintReadOnly, Category="Character | Attacks", Replicated, ReplicatedUsing=onSpeedChanged)
	FGameplayAttributeData HeavyAttackComboResetTime;
	ATTRIBUTE_ACCESSOR(UJFAttributeSet, HeavyAttackComboResetTime);

	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};
