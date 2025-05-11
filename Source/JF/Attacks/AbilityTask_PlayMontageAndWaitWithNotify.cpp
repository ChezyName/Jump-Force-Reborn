// Copyright ChezyName. All Rights Reserved.

#include "AbilityTask_PlayMontageAndWaitWithNotify.h"
#include "Animation/AnimMontage.h"
#include "GameFramework/Character.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemLog.h"
#include "AbilitySystemGlobals.h"
#include "AbilitySystemInterface.h"
#include "Kismet/KismetSystemLibrary.h"

UAbilityTask_PlayMontageAndWaitWithNotify* UAbilityTask_PlayMontageAndWaitWithNotify::CreatePlayMontageAndWaitWithNotifyProxy(
	UGameplayAbility* OwningAbility, FName TaskInstanceName, UAnimMontage* MontageToPlay, float Rate,
	FName StartSection, bool bStopWhenAbilityEnds, float AnimRootMotionTranslationScale, float StartTimeSeconds,
	bool bAllowInterruptAfterBlendOut, AActor* TargetActor)
{
	UAbilitySystemGlobals::NonShipping_ApplyGlobalAbilityScaler_Rate(Rate);

	UAbilityTask_PlayMontageAndWaitWithNotify* MyObj = NewAbilityTask<UAbilityTask_PlayMontageAndWaitWithNotify>(OwningAbility, TaskInstanceName);
	MyObj->MontageToPlay = MontageToPlay;
	MyObj->Rate = Rate;
	MyObj->StartSection = StartSection;
	MyObj->AnimRootMotionTranslationScale = AnimRootMotionTranslationScale;
	MyObj->bStopWhenAbilityEnds = bStopWhenAbilityEnds;
	MyObj->bAllowInterruptAfterBlendOut = bAllowInterruptAfterBlendOut;
	MyObj->StartTimeSeconds = StartTimeSeconds;
	MyObj->TargetActor = TargetActor;
	
	return MyObj;
}

void UAbilityTask_PlayMontageAndWaitWithNotify::ActivateOnTarget(AActor* Actor)
{
	if (Ability == nullptr)
	{
		return;
	}

	bool bPlayedMontage = false;
	UAbilitySystemComponent* ASC = nullptr;

	if(Actor != nullptr)
	{
		if(UAbilitySystemComponent* ASComp = Actor->FindComponentByClass<UAbilitySystemComponent>())
		{
			ASC = ASComp;
		}
	}
	
	if(ASC == nullptr) {
		ASC = AbilitySystemComponent.Get();
	}

	if (ASC != nullptr)
	{
		UAnimInstance* AnimInstance = nullptr;
		if(Actor != nullptr)
		{
			if(USkeletalMeshComponent* SKMesh = Actor->FindComponentByClass<USkeletalMeshComponent>())
			{
				AnimInstance = SKMesh->GetAnimInstance();
			}
		}
		
		if(AnimInstance == nullptr)
		{
			const FGameplayAbilityActorInfo* ActorInfo = Ability->GetCurrentActorInfo();
			AnimInstance = ActorInfo->GetAnimInstance();
		}
		
		if (AnimInstance != nullptr)
		{
			if (ASC->PlayMontage(Ability, Ability->GetCurrentActivationInfo(), MontageToPlay, Rate, StartSection, StartTimeSeconds) > 0.f)
			{
				// Playing a montage could potentially fire off a callback into game code which could kill this ability! Early out if we are  pending kill.
				if (ShouldBroadcastAbilityTaskDelegates() == false)
				{
					return;
				}

				InterruptedHandle = Ability->OnGameplayAbilityCancelled.AddUObject(this, &UAbilityTask_PlayMontageAndWaitWithNotify::OnGameplayAbilityCancelled);

				BlendedInDelegate.BindUObject(this, &UAbilityTask_PlayMontageAndWaitWithNotify::OnMontageBlendedIn);
				AnimInstance->Montage_SetBlendedInDelegate(BlendedInDelegate, MontageToPlay);

				BlendingOutDelegate.BindUObject(this, &UAbilityTask_PlayMontageAndWaitWithNotify::OnMontageBlendingOut);
				AnimInstance->Montage_SetBlendingOutDelegate(BlendingOutDelegate, MontageToPlay);

				MontageEndedDelegate.BindUObject(this, &UAbilityTask_PlayMontageAndWaitWithNotify::OnMontageEnded);
				AnimInstance->Montage_SetEndDelegate(MontageEndedDelegate, MontageToPlay);

				AnimInstance->OnPlayMontageNotifyBegin.AddDynamic(this, &UAbilityTask_PlayMontageAndWaitWithNotify::OnNotifyBegin_FUNC);

				ACharacter* Character = Cast<ACharacter>(GetAvatarActor());
				if (Character && (Character->GetLocalRole() == ROLE_Authority ||
								  (Character->GetLocalRole() == ROLE_AutonomousProxy && Ability->GetNetExecutionPolicy() == EGameplayAbilityNetExecutionPolicy::LocalPredicted)))
				{
					Character->SetAnimRootMotionTranslationScale(AnimRootMotionTranslationScale);
				}

				bPlayedMontage = true;
			}
		}
		else
		{
			ABILITY_LOG(Warning, TEXT("UAbilityTask_PlayMontageAndWaitWithNotify call to PlayMontage failed!"));
		}
	}
	else
	{
		ABILITY_LOG(Warning, TEXT("UAbilityTask_PlayMontageAndWaitWithNotify called on invalid AbilitySystemComponent"));
	}

	if (!bPlayedMontage)
	{
		ABILITY_LOG(Warning, TEXT("UAbilityTask_PlayMontageAndWaitWithNotify called in Ability %s failed to play montage %s; Task Instance Name %s."), *Ability->GetName(), *GetNameSafe(MontageToPlay),*InstanceName.ToString());
		if (ShouldBroadcastAbilityTaskDelegates())
		{
			OnCancelled.Broadcast();
		}
	}

	SetWaitingOnAvatar();
}

void UAbilityTask_PlayMontageAndWaitWithNotify::Activate()
{
	ActivateOnTarget(TargetActor);
}

void UAbilityTask_PlayMontageAndWaitWithNotify::OnDestroy(bool bInOwnerFinished)
{
	Super::OnDestroy(bInOwnerFinished);
	
	if (Ability)
	{
		Ability->OnGameplayAbilityCancelled.Remove(InterruptedHandle);
		if (bInOwnerFinished && bStopWhenAbilityEnds)
		{
			if (Ability == nullptr)
			{
				return;
			}

			UAbilitySystemComponent* ASC = nullptr;

			if(TargetActor)
			{
				if(UAbilitySystemComponent* ASComp = TargetActor->FindComponentByClass<UAbilitySystemComponent>())
				{
					ASC = ASComp;
				}
			}

			if(ASC == nullptr) {
				ASC = AbilitySystemComponent.Get();
			}
			
			if (ASC && Ability)
			{
				if (ASC->GetAnimatingAbility() == Ability
					&& ASC->GetCurrentMontage() == MontageToPlay)
				{
					// Stop the montage
					ASC->CurrentMontageStop();

					UAnimInstance* AnimInstance = nullptr;
					if(TargetActor != nullptr)
					{
						if(USkeletalMeshComponent* SKMesh = TargetActor->FindComponentByClass<USkeletalMeshComponent>())
						{
							AnimInstance = SKMesh->GetAnimInstance();
						}
					}
		
					if(AnimInstance == nullptr)
					{
						const FGameplayAbilityActorInfo* ActorInfo = Ability->GetCurrentActorInfo();
						AnimInstance = ActorInfo->GetAnimInstance();
					}

					// Clean up notify bindings
					if (AnimInstance != nullptr)
					{
						AnimInstance->OnPlayMontageNotifyBegin.RemoveDynamic(this,
							&UAbilityTask_PlayMontageAndWaitWithNotify::OnNotifyBegin_FUNC);

						AnimInstance->OnPlayMontageNotifyEnd.RemoveDynamic(this,
								&UAbilityTask_PlayMontageAndWaitWithNotify::OnNotifyEnd_FUNC);
					}
				}
			}
		}
	}
}
