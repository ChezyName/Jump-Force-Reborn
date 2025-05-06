// Copyright ChezyName. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "Animation/AnimInstance.h"
#include "Abilities/Tasks/AbilityTask.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "AbilityTask_PlayMontageAndWaitWithNotify.generated.h"

/**
 * 
 */
UCLASS()
class JF_API UAbilityTask_PlayMontageAndWaitWithNotify : public UAbilityTask_PlayMontageAndWait
{
	GENERATED_BODY()
public:
	UPROPERTY(BlueprintAssignable)
	FMontageWaitSimpleDelegate OnNotifyBegin;

	UPROPERTY(BlueprintAssignable)
	FMontageWaitSimpleDelegate OnNotifyEnd;

	/** 
	 * Start playing an animation montage on the avatar actor and wait for it to finish
	 * If StopWhenAbilityEnds is true, this montage will be aborted if the ability ends normally. It is always stopped when the ability is explicitly cancelled.
	 * On normal execution, OnBlendOut is called when the montage is blending out, and OnCompleted when it is completely done playing
	 * OnInterrupted is called if another montage overwrites this, and OnCancelled is called if the ability or task is cancelled
	 *
	 * @param _TaskInstanceName Set to override the name of this task, for later querying
	 * @param _MontageToPlay The montage to play on the character
	 * @param _Rate Change to play the montage faster or slower
	 * @param _StartSection If not empty, named montage section to start from
	 * @param _bStopWhenAbilityEnds If true, this montage will be aborted if the ability ends normally. It is always stopped when the ability is explicitly cancelled
	 * @param _AnimRootMotionTranslationScale Change to modify size of root motion or set to 0 to block it entirely
	 * @param _StartTimeSeconds Starting time offset in montage, this will be overridden by StartSection if that is also set
	 * @param _bAllowInterruptAfterBlendOut If true, you can receive OnInterrupted after an OnBlendOut started (otherwise OnInterrupted will not fire when interrupted, but you will not get OnComplete).
	 */
	UFUNCTION(BlueprintCallable, Category="Ability|Tasks", meta = (DisplayName="PlayMontageAndWaitWithNotify",
		HidePin = "OwningAbility", DefaultToSelf = "OwningAbility", BlueprintInternalUseOnly = "TRUE"))
	static UAbilityTask_PlayMontageAndWaitWithNotify* CreatePlayMontageAndWaitWithNotifyProxy(UGameplayAbility* OwningAbility,
		FName _TaskInstanceName, UAnimMontage* _MontageToPlay, float _Rate = 1.f, FName _StartSection = NAME_None, bool _bStopWhenAbilityEnds = true, float _AnimRootMotionTranslationScale = 1.f, float _StartTimeSeconds = 0.f, bool _bAllowInterruptAfterBlendOut = false);

protected:
	UFUNCTION()
	void OnNotifyBegin_FUNC(FName NotifyName, const FBranchingPointNotifyPayload& BranchingPayload) {OnNotifyBegin.Broadcast();}

	UFUNCTION()
	void OnNotifyEnd_FUNC(FName NotifyName, const FBranchingPointNotifyPayload& BranchingPayload) {OnNotifyEnd.Broadcast();}
	
	virtual void Activate() override;

	virtual void OnDestroy(bool bInOwnerFinished) override;
};
