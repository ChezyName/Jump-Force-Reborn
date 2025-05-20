// Copyright ChezyName. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "JF/JFCharacter.h"
#include "JF/Attacks/HitboxTask.h"
#include "MeteredGameplayAbility.generated.h"

/**
 * 
 */
UCLASS()
class JF_API UMeteredGameplayAbility : public UGameplayAbility
{
	GENERATED_BODY()
	virtual bool CheckCost(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, FGameplayTagContainer* OptionalRelevantTags) const override;
	virtual bool CommitAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, FGameplayTagContainer* OptionalRelevantTags) override;

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	
	UPROPERTY()
	UHitboxTask* TickTask;
	
	float DeltaTime;
	float LastTime;

	UPROPERTY()
	AJFCharacter* Character;

	UFUNCTION()
	void onTick();
public:
	//Will Cost Full Bar (1 - 6)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(ClampMin=0, ClampMax=6))
	int AbilityCost = 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(Units="m/s", ClampMin=0, ClampMax=600))
	float MeterPerSecond = 0.f;

	UFUNCTION(BlueprintPure)
	AJFCharacter* GetCharacter() {return Character;}
};
