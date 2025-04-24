// Copyright ChezyName. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "EnhancedInputComponent.h"
#include "JFASComponent.generated.h"

class UInputAction;

USTRUCT()
struct FAbilityInputBinding
{
	GENERATED_BODY()

	int32  InputID = 0;
	uint32 OnPressedHandle = 0;
	uint32 OnReleasedHandle = 0;
	TArray<FGameplayAbilitySpecHandle> BoundAbilitiesStack;
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class JF_API UJFASComponent : public UAbilitySystemComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UJFASComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:
	UFUNCTION(BlueprintCallable, Category = "Enhanced Input Abilities")
	void SetInputBinding(UInputAction* InputAction, FGameplayAbilitySpecHandle AbilityHandle);

	UFUNCTION(BlueprintCallable, Category = "Enhanced Input Abilities")
	void ClearInputBinding(FGameplayAbilitySpecHandle AbilityHandle);

	UFUNCTION(BlueprintCallable, Category = "Enhanced Input Abilities")
	void ClearAbilityBindings(UInputAction* InputAction);

	UFUNCTION()
	void SetInputComponent(UEnhancedInputComponent* _InputComponent) {InputComponent = _InputComponent;}
	
private:
	void OnAbilityInputPressed(UInputAction* InputAction);

	void OnAbilityInputReleased(UInputAction* InputAction);

	void RemoveEntry(UInputAction* InputAction);

	void TryBindAbilityInput(UInputAction* InputAction, FAbilityInputBinding& AbilityInputBinding);

	FGameplayAbilitySpec* FindAbilitySpec(FGameplayAbilitySpecHandle Handle);

	UPROPERTY(transient)
	TMap<UInputAction*, FAbilityInputBinding> MappedAbilities;

	UPROPERTY(transient)
	UEnhancedInputComponent* InputComponent;
};
