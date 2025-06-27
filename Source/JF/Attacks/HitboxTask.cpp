// Copyright ChezyName. All Rights Reserved.

#include "HitboxTask.h"
#include "TimerManager.h"
#include "GameFramework/Actor.h"
#include "Abilities/GameplayAbility.h"
#include "Kismet/KismetSystemLibrary.h"

UHitboxTask* UHitboxTask::CreateHitboxTicker(UGameplayAbility* OwningParentAbility, float TickInterval)
{
	UHitboxTask* Task = NewAbilityTask<UHitboxTask>(OwningParentAbility);
	Task->Interval = TickInterval;
	Task->ParentAbility = OwningParentAbility;
	return Task;
}

void UHitboxTask::Activate()
{
	TickDelegate = FWorldDelegates::OnWorldTickStart.AddUObject(this, &UHitboxTask::TickHitboxes);
	Super::Activate();
}

void UHitboxTask::OnDestroy(bool bInOwnerFinished)
{
	if (TickDelegate.IsValid())
	{
		FWorldDelegates::OnWorldTickStart.Remove(TickDelegate);
		TickDelegate.Reset();
	}

	Super::OnDestroy(bInOwnerFinished);
}

void UHitboxTask::TickHitboxes(UWorld* World, ELevelTick Tick, float Time)
{
	OnTick.Broadcast();
}
