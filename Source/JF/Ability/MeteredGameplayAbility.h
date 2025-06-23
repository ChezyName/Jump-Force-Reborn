// Copyright ChezyName. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "JF/JFCharacter.h"
#include "JF/Attacks/GameplayAbility_JFAttack.h"
#include "JF/Attacks/HitboxTask.h"
#include "MeteredGameplayAbility.generated.h"

/**
 * 
 */
UCLASS()
class JF_API UMeteredGameplayAbility : public UGameplayAbility_JFAttack
{
	GENERATED_BODY()
	virtual bool CheckCost(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, FGameplayTagContainer* OptionalRelevantTags) const override;
	virtual bool CommitAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, FGameplayTagContainer* OptionalRelevantTags) override;

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	
	float DeltaTime;
	float LastTime;

	UPROPERTY()
	AJFCharacter* Character;

	virtual void onTick() override
	{
		onTickSelf();
		Super::onTick();
	}

	void onTickSelf();

	virtual void InputReleased(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) override;

	UFUNCTION(Server, Reliable)
	void ServerKeyReleased();

	UFUNCTION()
	void ClientKeyReleased();

	UPROPERTY()
	bool isActive = false;

	UPROPERTY()
	UHitboxTask* TickTask;
public:
	UMeteredGameplayAbility();
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta=(ClampMin=0, ClampMax=6), Category="Ability")
	bool isToggledAbility = false;
	
	//Will Cost Full Bar (1 - 6)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta=(ClampMin=0, ClampMax=6), Category="Ability")
	int AbilityCost = 1;

	//Will Auto Take The Cost Upon Use
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta=(ClampMin=0, ClampMax=6), Category="Ability")
	bool TakesCostOnUse = true;

	//Will Auto Take The Cost Upon Use
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, meta=(ClampMin=0, ClampMax=6), Category="Ability")
	bool TakesMeterPerSecond = true;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta=(Units="m/s", ClampMin=0, ClampMax=600), Category="Ability")
	float MeterPerSecond = 0.f;

	//If Not Toggled Ability
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Ability")
	bool ForceEndOnMeterZero = true;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Ability")
	bool EndAbilityOnKeyRelease = false;

	UFUNCTION(BlueprintPure)
	AJFCharacter* GetCharacter() {return Character;}

	UPROPERTY(BlueprintReadOnly)
	bool isKeyHeld = true;
};
