// Copyright ChezyName. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTask.h"
#include "Abilities/GameplayAbility.h"
#include "HitboxTask.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnHitboxTick);

UCLASS()
class JF_API UHitboxTask : public UGameplayTask
{
	GENERATED_BODY()
public:
	UPROPERTY(BlueprintAssignable)
	FOnHitboxTick OnTick;

	float Interval;
	FTimerHandle TickTimerHandle;

	UPROPERTY()
	UGameplayAbility* Ability;

	static UHitboxTask* CreateHitboxTicker(UGameplayAbility* OwningAbility, float TickInterval = 1.f/60.f);

	virtual void Activate() override;
	virtual void OnDestroy(bool bInOwnerFinished) override;

private:
	bool bActive = false;
	
	void TickHitboxes();
};
