// Copyright ChezyName. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "InputAction.h"
#include "Abilities/GameplayAbility.h"
#include "Engine/DataAsset.h"
#include "AbilityData.generated.h"

UENUM(BlueprintType)
enum class EAbilityInputID : uint8
{
	None			UMETA(DisplayName = "None", BlueprintReadOnly),
	Confirm			UMETA(DisplayName = "Confirm", BlueprintReadOnly),
	Cancel			UMETA(DisplayName = "Cancel", BlueprintReadOnly),
	
	LightAttack		UMETA(DisplayName = "Light Attack", BlueprintReadOnly),	// Right Bumper
	HeavyAttack		UMETA(DisplayName = "Heavy Attack", BlueprintReadOnly),	// Right Trigger
	Dodge			UMETA(DisplayName = "Dodge", BlueprintReadOnly),			// A

	Ability1		UMETA(DisplayName = "Ability1", BlueprintReadOnly),		// X
	Ability2		UMETA(DisplayName = "Ability2", BlueprintReadOnly),		// Y
	Ability3		UMETA(DisplayName = "Ability3", BlueprintReadOnly),		// B

	Ultimate		UMETA(DisplayName = "Ultimate", BlueprintReadOnly),		// Both Bumpers
};

/**
 * 
 */
UCLASS(BlueprintType)
class JF_API UAbilityData : public UPrimaryDataAsset
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, Category="CORE", BlueprintReadOnly)
	FString AbilityName;

	UPROPERTY(EditAnywhere, Category="CORE", BlueprintReadOnly)
	FString AbilityDesc;

	UPROPERTY(EditAnywhere, Category="INPUT", BlueprintReadOnly)
	EAbilityInputID AbilityKey;

	UPROPERTY(EditAnywhere, Category="INPUT", BlueprintReadOnly)
	UInputAction* AbilityAction;

	UPROPERTY(EditAnywhere, Category="CORE", BlueprintReadOnly, BlueprintReadOnly)
	TSubclassOf<UGameplayAbility> Ability;

	UPROPERTY(EditAnywhere, Category="CORE", BlueprintReadOnly, BlueprintReadOnly)
	int AbilityCost = 1;

	static FString AbilityKeyToString(EAbilityInputID Ability)
	{
		switch (Ability)
		{
			case EAbilityInputID::LightAttack:
				return "Light Attack";
			case EAbilityInputID::HeavyAttack:
				return "Heavy Attack";
			case EAbilityInputID::Dodge:
				return "Dodge";
			case EAbilityInputID::Ability1:
				return "Ability 1";
			case EAbilityInputID::Ability2:
				return "Ability 2";
			case EAbilityInputID::Ability3:
				return "Ability 3";
			case EAbilityInputID::Ultimate:
				return "Ultimate Ability";
			default:
				return "N/A";
		}
	}
};
