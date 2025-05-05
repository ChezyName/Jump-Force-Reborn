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
	if (ParentAbility && ParentAbility->GetWorld())
	{
		ParentAbility->GetWorld()->GetTimerManager().SetTimer(
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
	if (ParentAbility && ParentAbility->GetWorld())
	{
		ParentAbility->GetWorld()->GetTimerManager().ClearTimer(TickTimerHandle);
	}

	Super::OnDestroy(bInOwnerFinished);
}

void UHitboxTask::TickHitboxes()
{
	OnTick.Broadcast();
}
