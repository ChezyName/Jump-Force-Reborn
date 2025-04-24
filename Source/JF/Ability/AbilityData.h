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
	None			UMETA(DisplayName = "None"),
	Confirm			UMETA(DisplayName = "Confirm"),
	Cancel			UMETA(DisplayName = "Cancel"),
	
	LightAttack		UMETA(DisplayName = "Light Attack"),	// Right Bumper
	HeavyAttack		UMETA(DisplayName = "Heavy Attack"),	// Right Trigger
	Dodge			UMETA(DisplayName = "Dodge"),			// A

	Ability1		UMETA(DisplayName = "Ability1"),		//Left Bumper + X
	Ability2		UMETA(DisplayName = "Ability2"),		//Left Bumper + Y
	Ability3		UMETA(DisplayName = "Ability3"),		//Left Bumper + B
	Ability4		UMETA(DisplayName = "Ability4"),		//Right Bumper + X
	Ability5		UMETA(DisplayName = "Ability5"),		//Right Bumper + Y
	Ability6		UMETA(DisplayName = "Ability6"),		//Right Bumper + B

	Ultimate		UMETA(DisplayName = "Ultimate"),		//Both Bumpers
};

/**
 * 
 */
UCLASS(BlueprintType)
class JF_API UAbilityData : public UPrimaryDataAsset
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, Category="CORE")
	FString AbilityName;

	UPROPERTY(EditAnywhere, Category="CORE")
	FString AbilityDesc;

	UPROPERTY(EditAnywhere, Category="INPUT")
	EAbilityInputID AbilityKey;

	UPROPERTY(EditAnywhere, Category="INPUT")
	UInputAction* AbilityAction;

	UPROPERTY(EditAnywhere, Category="", BlueprintReadOnly)
	TSubclassOf<UGameplayAbility> Ability;
};
