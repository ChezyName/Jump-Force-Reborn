﻿// Copyright ChezyName. All Rights Reserved.


#include "JFASComponent.h"

#include "AbilitySystemLog.h"
#include "JF/JFCharacter.h"
#include "Kismet/KismetSystemLibrary.h"


namespace EnhancedInputAbilitySystem_Impl
{
	constexpr int32 InvalidInputID = 0;
	int32 IncrementingInputID = InvalidInputID;

	static int32 GetNextInputID()
	{
		return ++IncrementingInputID;
	}
}

void UJFASComponent::SetInputBinding(UInputAction* InputAction, FGameplayAbilitySpecHandle AbilityHandle)
{
	using namespace EnhancedInputAbilitySystem_Impl;

	FGameplayAbilitySpec* BindingAbility = FindAbilitySpec(AbilityHandle);

	//UKismetSystemLibrary::PrintString(GetWorld(), "	Creating Input Binding", true,true,FLinearColor::Red,30);
	
	FAbilityInputBinding* AbilityInputBinding = MappedAbilities.Find(InputAction);
	if (AbilityInputBinding)
	{
		//UKismetSystemLibrary::PrintString(GetWorld(), "	Binding Found -> Adding New Binds", true,true,FLinearColor::Red,30);
		FGameplayAbilitySpec* OldBoundAbility = FindAbilitySpec(AbilityInputBinding->BoundAbilitiesStack.Top());
		if (OldBoundAbility && OldBoundAbility->InputID == AbilityInputBinding->InputID)
		{
			OldBoundAbility->InputID = InvalidInputID;
		}
	}
	else
	{
		//UKismetSystemLibrary::PrintString(GetWorld(), "	No Binding Found in Registry", true,true,FLinearColor::Red,30);
		AbilityInputBinding = &MappedAbilities.Add(InputAction);
		AbilityInputBinding->InputID = GetNextInputID();
	}

	if (BindingAbility)
	{
		BindingAbility->InputID = AbilityInputBinding->InputID;
	}

	AbilityInputBinding->BoundAbilitiesStack.Push(AbilityHandle);
	TryBindAbilityInput(InputAction, *AbilityInputBinding);
}

void UJFASComponent::ClearInputBinding(FGameplayAbilitySpecHandle AbilityHandle)
{
	using namespace EnhancedInputAbilitySystem_Impl;

	if (FGameplayAbilitySpec* FoundAbility = FindAbilitySpec(AbilityHandle))
	{
		// Find the mapping for this ability
		auto MappedIterator = MappedAbilities.CreateIterator();
		while (MappedIterator)
		{
			if (MappedIterator.Value().InputID == FoundAbility->InputID)
			{
				break;
			}

			++MappedIterator;
		}

		if (MappedIterator)
		{
			FAbilityInputBinding& AbilityInputBinding = MappedIterator.Value();

			if (AbilityInputBinding.BoundAbilitiesStack.Remove(AbilityHandle) > 0)
			{
				if (AbilityInputBinding.BoundAbilitiesStack.Num() > 0)
				{
					FGameplayAbilitySpec* StackedAbility = FindAbilitySpec(AbilityInputBinding.BoundAbilitiesStack.Top());
					if (StackedAbility && StackedAbility->InputID == 0)
					{
						StackedAbility->InputID = AbilityInputBinding.InputID;
					}
				}
				else
				{
					// NOTE: This will invalidate the `AbilityInputBinding` ref above
					RemoveEntry(MappedIterator.Key());
				}
				// DO NOT act on `AbilityInputBinding` after here (it could have been removed)


				FoundAbility->InputID = InvalidInputID;
			}
		}
	}
}

void UJFASComponent::ClearAbilityBindings(UInputAction* InputAction)
{
	RemoveEntry(InputAction);
}

void UJFASComponent::AbilityLocalInputPressed(int32 InputID)
{
	// Consume the input if this InputID is overloaded with GenericConfirm/Cancel and the GenericConfim/Cancel callback is bound
	if (IsGenericConfirmInputBound(InputID))
	{
		LocalInputConfirm();
		return;
	}

	if (IsGenericCancelInputBound(InputID))
	{
		LocalInputCancel();
		return;
	}

	// ---------------------------------------------------------

	ABILITYLIST_SCOPE_LOCK();
	for (FGameplayAbilitySpec& Spec : ActivatableAbilities.Items)
	{
		if (Spec.InputID == InputID)
		{
			if (Spec.Ability)
			{
				Spec.InputPressed = true;
				if (Spec.IsActive())
				{
					if (Spec.Ability->bReplicateInputDirectly && IsOwnerActorAuthoritative() == false)
					{
						ServerSetInputPressed(Spec.Handle);
					}

					AbilitySpecInputPressed(Spec);

					PRAGMA_DISABLE_DEPRECATION_WARNINGS
										// Fixing this up to use the instance activation, but this function should be deprecated as it cannot work with InstancedPerExecution
										UE_CLOG(Spec.Ability->GetInstancingPolicy() == EGameplayAbilityInstancingPolicy::InstancedPerExecution, LogAbilitySystem, Warning, TEXT("%hs: %s is InstancedPerExecution. This is unreliable for Input as you may only interact with the latest spawned Instance"), __func__, *GetNameSafe(Spec.Ability));
					TArray<UGameplayAbility*> Instances = Spec.GetAbilityInstances();
					const FGameplayAbilityActivationInfo& ActivationInfo = Instances.IsEmpty() ? Spec.ActivationInfo : Instances.Last()->GetCurrentActivationInfoRef();
					PRAGMA_ENABLE_DEPRECATION_WARNINGS
										// Invoke the InputPressed event. This is not replicated here. If someone is listening, they may replicate the InputPressed event to the server.
										InvokeReplicatedEvent(EAbilityGenericReplicatedEvent::InputPressed, Spec.Handle, ActivationInfo.GetActivationPredictionKey());					
				}
				else
				{
					// Ability is not active, so try to activate it
					bool isActive = TryActivateAbility(Spec.Handle);
					if(!isActive)
					{
						//Queue Ability
						NextAbility = {
							Spec.Handle,
							true,
							ABILITY_QUEUE_LIFETIME_DEFAULT,
							Ability
						};

						UKismetSystemLibrary::PrintString(GetWorld(), "Ability Queued");
					}
				}
			}
		}
	}
}

void UJFASComponent::OnGiveAbility(FGameplayAbilitySpec& AbilitySpec)
{
	Super::OnGiveAbility(AbilitySpec);

	AJFCharacter* Char = Cast<AJFCharacter>(GetOwnerActor());
	if(Char)
	{
		UAbilityData* AbilityFound = nullptr;
		if(AbilityDataIsValid(Char->DashAbility))
		{
			if(FindAbilitySpecFromClass(Char->DashAbility->Ability) == &AbilitySpec)
			{
				AbilityFound = Char->DashAbility;
			}
		}

		for (UAbilityData* CharAbility : Char->CharacterAbilities)
		{
			if(AbilityFound != nullptr) break;

			if(AbilityDataIsValid(CharAbility))
			{
				if(FindAbilitySpecFromClass(CharAbility->Ability) == &AbilitySpec)
				{
					AbilityFound = CharAbility;
				}
			}
		}

		if(AbilityFound != nullptr && AbilityFound->AbilityAction != nullptr && AbilityFound->Ability != nullptr)
		{
			SetInputBinding(AbilityFound->AbilityAction, AbilitySpec.Handle);
		}
	}
}

void UJFASComponent::OnAbilityInputPressed(UInputAction* InputAction)
{
	using namespace EnhancedInputAbilitySystem_Impl;

	FAbilityInputBinding* FoundBinding = MappedAbilities.Find(InputAction);
	if (FoundBinding && ensure(FoundBinding->InputID != InvalidInputID))
	{
		AbilityLocalInputPressed(FoundBinding->InputID);
		AbilityPressedEvent.Broadcast();
	}
}

void UJFASComponent::OnAbilityInputReleased(UInputAction* InputAction)
{
	using namespace EnhancedInputAbilitySystem_Impl;

	FAbilityInputBinding* FoundBinding = MappedAbilities.Find(InputAction);
	if (FoundBinding && ensure(FoundBinding->InputID != InvalidInputID))
	{
				
		AbilityLocalInputReleased(FoundBinding->InputID);
		AbilityReleasedEvent.Broadcast();
	}
}

void UJFASComponent::RemoveEntry(UInputAction* InputAction)
{
	if (FAbilityInputBinding* Bindings = MappedAbilities.Find(InputAction))
	{
		if (InputComponent)
		{
			InputComponent->RemoveBindingByHandle(Bindings->OnPressedHandle);
			InputComponent->RemoveBindingByHandle(Bindings->OnReleasedHandle);
		}

		for (FGameplayAbilitySpecHandle AbilityHandle : Bindings->BoundAbilitiesStack)
		{
			using namespace EnhancedInputAbilitySystem_Impl;

			FGameplayAbilitySpec* AbilitySpec = FindAbilitySpec(AbilityHandle);
			if (AbilitySpec && AbilitySpec->InputID == Bindings->InputID)
			{
				AbilitySpec->InputID = InvalidInputID;
			}
		}

		MappedAbilities.Remove(InputAction);
	}
}

FGameplayAbilitySpec* UJFASComponent::FindAbilitySpec(FGameplayAbilitySpecHandle Handle)
{
	FGameplayAbilitySpec* FoundAbility = nullptr;
	FoundAbility = FindAbilitySpecFromHandle(Handle);
	return FoundAbility;
}

void UJFASComponent::TryBindAbilityInput(UInputAction* InputAction, FAbilityInputBinding& AbilityInputBinding)
{
	if (InputComponent)
	{
		// Pressed event
		if (AbilityInputBinding.OnPressedHandle == 0)
		{
			AbilityInputBinding.OnPressedHandle = InputComponent->BindAction(InputAction, ETriggerEvent::Started, this, &UJFASComponent::OnAbilityInputPressed, InputAction).GetHandle();

			//UKismetSystemLibrary::PrintString(GetWorld(), "Bound Input > On Presesd Handle", true,true,FLinearColor::Red,30);
		}

		// Released event
		if (AbilityInputBinding.OnReleasedHandle == 0)
		{
			AbilityInputBinding.OnReleasedHandle = InputComponent->BindAction(InputAction, ETriggerEvent::Completed, this, &UJFASComponent::OnAbilityInputReleased, InputAction).GetHandle();

			//UKismetSystemLibrary::PrintString(GetWorld(), "Bound Input > On Presesd Handle", true,true,FLinearColor::Red,30);
		}
	}
	//else UKismetSystemLibrary::PrintString(GetWorld(), "InputComponent is Invalid...", true,true,FLinearColor::Red,30);
}

UJFASComponent::UJFASComponent() : Super()
{
	SetIsReplicated(true);

	bAutoActivate = true;
}

void UJFASComponent::BeginPlay() {
	Super::BeginPlay();
	
	SetReplicationMode(EGameplayEffectReplicationMode::Mixed);
	
	if(InputComponent == nullptr)
	{
		AActor* Owner = GetOwner();
		if (IsValid(Owner) && Owner->InputComponent) {
			InputComponent = CastChecked<UEnhancedInputComponent>(Owner->InputComponent);
		}
	}
}

void UJFASComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

bool UJFASComponent::TryActivateOrQueueAbility(FGameplayAbilitySpecHandle Handle, bool bAllowRemoteActivation)
{
	const bool bSuccess = TryActivateAbility(Handle, bAllowRemoteActivation);

	if (!bSuccess)
	{
		NextAbility = {
			Handle,
			bAllowRemoteActivation,
			ABILITY_QUEUE_LIFETIME_DEFAULT,
		};
	}

	return bSuccess;
}

bool UJFASComponent::TryActivateOrQueueAbilityByClass(TSubclassOf<UGameplayAbility> InAbilityToActivate,
	bool bAllowRemoteActivation)
{
	bool bSuccess = false;

	const UGameplayAbility* const InAbilityCDO = InAbilityToActivate.GetDefaultObject();

	for (const FGameplayAbilitySpec& Spec : ActivatableAbilities.Items)
	{
		if (Spec.Ability == InAbilityCDO)
		{
			bSuccess |= TryActivateOrQueueAbility(Spec.Handle, bAllowRemoteActivation);
			break;
		}
	}

	return bSuccess;
}