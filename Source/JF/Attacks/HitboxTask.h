// Copyright ChezyName. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTask.h"
#include "Abilities/GameplayAbility.h"
#include "Abilities/Tasks/AbilityTask.h"
#include "HitboxTask.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnHitboxTick);

UCLASS()
class JF_API UHitboxTask : public UAbilityTask
{
	GENERATED_BODY()
public:
	UPROPERTY(BlueprintAssignable)
	FOnHitboxTick OnTick;

	float Interval = KINDA_SMALL_NUMBER;
	FTimerHandle TickTimerHandle;

	UPROPERTY()
	UGameplayAbility* ParentAbility;

	static UHitboxTask* CreateHitboxTicker(UGameplayAbility* OwningAbility, float TickInterval = KINDA_SMALL_NUMBER);

	virtual void Activate() override;
	virtual void OnDestroy(bool bInOwnerFinished) override;

private:
	bool bActive = false;
	
	void TickHitboxes();
};
