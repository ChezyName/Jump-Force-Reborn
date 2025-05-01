// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "Ability/AbilityData.h"
#include "Ability/JFASComponent.h"
#include "Attributes/JFAttributeSet.h"
#include "GameFramework/Character.h"
#include "Logging/LogMacros.h"
#include "JFCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UInputMappingContext;
class UInputAction;
struct FInputActionValue;

USTRUCT()
struct FAbilityInputHandler
{
	GENERATED_BODY()
public:
	UPROPERTY()
	FGameplayAbilitySpecHandle Handle;
	UPROPERTY()
	UInputAction* Action;
	
};

DECLARE_LOG_CATEGORY_EXTERN(LogTemplateCharacter, Log, All);

UCLASS(config=Game)
class AJFCharacter : public ACharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()
public:
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Abilities")
    UJFASComponent* AbilitySystemComponent;
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Character | Abilities")
    TArray<UAbilityData*> CharacterAbilities;
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Character | Abilities")
    UAbilityData* DashAbility;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character | Attacks")
	TArray<TSubclassOf<UGameplayAbility>> LightAttacks;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character | Attacks")
	TArray<TSubclassOf<UGameplayAbility>> HeavyAttacks;

	UFUNCTION(BlueprintPure)
	FVector2D GetPlayerInputVector() {return PlayerInputVector;}

	UFUNCTION(BlueprintPure)
	FVector GetMovementVector();
	
	UFUNCTION(BlueprintPure)
	FVector GetCameraRightVector();

	UFUNCTION(BlueprintPure)
	FVector GetCameraForwardVector();
private:
	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FollowCamera;
	
	/** MappingContext */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputMappingContext* DefaultMappingContext;

	/** Move Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* MoveAction;

	/** Look Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* LookAction;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* LightAttackAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* HeavyAttackAction;

	void PossessedBy(AController* NewController) override;
	virtual void OnRep_PlayerState() override;

	UPROPERTY(Replicated)
	FVector2D PlayerInputVector;

public:
	AJFCharacter();
	
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override
	{
		return AbilitySystemComponent;
	}

	UJFAttributeSet* GetCoreAttributes()
	{
		return CoreAttributes;
	}

	UFUNCTION(Server, Reliable)
	void InitAbilitiesServer();
	UFUNCTION(Client, Reliable)
	void InitAbilitiesClient(const TArray<FAbilityInputHandler>& Abilities);
	
	void InitAbilitiesInputSys();
	
	virtual void BeginPlay() override;

protected:
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Abilities")
	UJFAttributeSet* CoreAttributes;

	UFUNCTION(Server, Unreliable)
	void UpdatePlayerMovementVector(FVector2D MovementVector);

	/** Called for movement input */
	void Move(const FInputActionValue& Value);

	/** Called for looking input */
	void Look(const FInputActionValue& Value);
	
	void LightAttack();
	void HeavyAttack();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
protected:
	virtual void NotifyControllerChanged() override;

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

public:
	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }
};

