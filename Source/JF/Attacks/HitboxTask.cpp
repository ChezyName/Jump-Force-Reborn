// Copyright ChezyName. All Rights Reserved.

#include "HitboxTask.h"
#include "TimerManager.h"
#include "GameFramework/Actor.h"
#include "Abilities/GameplayAbility.h"

UHitboxTask* UHitboxTask::CreateHitboxTicker(UGameplayAbility* OwningAbility, float TickInterval)
{
	UHitboxTask* Task = NewObject<UHitboxTask>(OwningAbility);
	Task->Interval = TickInterval;
	Task->Ability = OwningAbility;
	return Task;
}

void UHitboxTask::Activate()
{
	if (Ability && Ability->GetWorld())
	{
		Ability->GetWorld()->GetTimerManager().SetTimer(
			TickTimerHandle,
			this,
			&UHitboxTask::TickHitboxes,
			Interval,
			true
		);
	}
	
	Super::Activate();
}

void UHitboxTask::OnDestroy(bool bInOwnerFinished)
{
	if (Ability && Ability->GetWorld())
	{
		Ability->GetWorld()->GetTimerManager().ClearTimer(TickTimerHandle);
	}

	Super::OnDestroy(bInOwnerFinished);
}

void UHitboxTask::TickHitboxes()
{
	OnTick.Broadcast();
}
