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

	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};
