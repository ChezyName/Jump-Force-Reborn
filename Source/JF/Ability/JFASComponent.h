// Copyright ChezyName. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AbilityData.h"
#include "AbilitySystemComponent.h"
#include "EnhancedInputComponent.h"
#include "JFASComponent.generated.h"

class UInputAction;

constexpr float ABILITY_QUEUE_LIFETIME_DEFAULT = 0.2f;

USTRUCT()
struct FAbilityInputBinding
{
	GENERATED_BODY()

	int32  InputID = 0;
	uint32 OnPressedHandle = 0;
	uint32 OnReleasedHandle = 0;
	TArray<FGameplayAbilitySpecHandle> BoundAbilitiesStack;
};

UENUM()
enum AbilityType : uint8
{
	Light,
	Heavy,
	Ability,
};

USTRUCT(BlueprintType)
struct FQueuedAbility
{
	GENERATED_BODY()

	UPROPERTY()
	FGameplayAbilitySpecHandle Handle;

	UPROPERTY()
	bool bWasRemote = true;

	UPROPERTY()
	float Lifetime = -1;
	
	UPROPERTY()
	TEnumAsByte<AbilityType> Type = Ability;
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class JF_API UJFASComponent : public UAbilitySystemComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UJFASComponent();

	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FAbilityInput);

	UPROPERTY(BlueprintAssignable, Category=Input)
	FAbilityInput AbilityPressedEvent;
	UPROPERTY(BlueprintAssignable, Category=Input)
	FAbilityInput AbilityReleasedEvent;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
public:
	UFUNCTION(BlueprintCallable, Category = "Enhanced Input Abilities")
	void SetInputBinding(UInputAction* InputAction, FGameplayAbilitySpecHandle AbilityHandle);

	UFUNCTION(BlueprintCallable, Category = "Enhanced Input Abilities")
	void ClearInputBinding(FGameplayAbilitySpecHandle AbilityHandle);

	UFUNCTION(BlueprintCallable, Category = "Enhanced Input Abilities")
	void ClearAbilityBindings(UInputAction* InputAction);

	virtual void AbilityLocalInputPressed(int32 InputID) override;

	UFUNCTION()
	void SetInputComponent(UEnhancedInputComponent* _InputComponent) {InputComponent = _InputComponent;}

	FGameplayAbilitySpec* FindAbilitySpec(FGameplayAbilitySpecHandle Handle);

	bool TryActivateOrQueueAbility(FGameplayAbilitySpecHandle Handle, bool bAllowRemoteActivation = true);
	bool TryActivateOrQueueAbilityByClass(TSubclassOf<UGameplayAbility> InAbilityToActivate, bool bAllowRemoteActivation = true);
	
	FQueuedAbility& GetNextAbility() {return NextAbility;}
	void TickAbility(float DeltaSeconds)
	{
		NextAbility.Lifetime = NextAbility.Lifetime - DeltaSeconds;
	}
	void EndAbility()
	{
		NextAbility.Lifetime = -1;
	}
	
protected:
	virtual void OnGiveAbility(FGameplayAbilitySpec& AbilitySpec) override;

	UPROPERTY(BlueprintReadOnly)
	FQueuedAbility NextAbility = {};
	
private:
	bool AbilityDataIsValid(UAbilityData* Data)
	{
		return Data != nullptr && Data->Ability != nullptr && Data->AbilityAction != nullptr;
	}
	
	void OnAbilityInputPressed(UInputAction* InputAction);

	void OnAbilityInputReleased(UInputAction* InputAction);

	void RemoveEntry(UInputAction* InputAction);

	void TryBindAbilityInput(UInputAction* InputAction, FAbilityInputBinding& AbilityInputBinding);

	UPROPERTY(transient)
	TMap<UInputAction*, FAbilityInputBinding> MappedAbilities;

	UPROPERTY(transient)
	UEnhancedInputComponent* InputComponent;
};
