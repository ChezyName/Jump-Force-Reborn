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

//Each bar is 100, 6x = 600
constexpr float MAX_METER = 600.f;

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
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Character")
    UJFASComponent* AbilitySystemComponent;
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Character|Abilities")
    TArray<UAbilityData*> CharacterAbilities;
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Character|Abilities")
    UAbilityData* DashAbility;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character|Attacks")
	TArray<TSubclassOf<UGameplayAbility>> LightAttacks;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character|Attacks")
	TArray<TSubclassOf<UGameplayAbility>> HeavyAttacks;

	UFUNCTION(BlueprintPure)
	FVector2D GetPlayerInputVector() {return PlayerInputVector;}

	UFUNCTION(BlueprintPure)
	FVector GetMovementVector();
	
	UFUNCTION(BlueprintPure)
	FVector GetCameraRightVector();

	UFUNCTION(BlueprintPure)
	FVector GetCameraForwardVector();

	//Default Attributes
	//Players Max Health
	UPROPERTY(EditDefaultsOnly, Category="Character|Defaults")
	float MaxHealth = 1500;
	
	//Players Movement Speed
	UPROPERTY(EditDefaultsOnly, Category="Character|Defaults", meta=(Units="cm/s"))
	float MovementSpeed = 500;
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

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* MeterChargeAction;

	void PossessedBy(AController* NewController) override;
	virtual void OnRep_PlayerState() override;

	UPROPERTY(Replicated)
	FVector2D PlayerInputVector;

	UFUNCTION(BlueprintPure)
	bool isChargingMeter();

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
	
	float GetNumericAttribute(const FGameplayAttribute &Attribute) const
	{
		if(GetAbilitySystemComponent())
		{
			return GetAbilitySystemComponent()->GetNumericAttribute(Attribute);
		}

		return 0;
	}

	void SetNumericAttribute(const FGameplayAttribute &Attribute, float Value) const
	{
		if(GetAbilitySystemComponent())
		{
			GetAbilitySystemComponent()->SetNumericAttributeBase(Attribute, Value);
		}
	}

	UFUNCTION(Server, Reliable)
	void InitAbilitiesServer();
	UFUNCTION(Client, Reliable)
	void InitAbilitiesClient(const TArray<FAbilityInputHandler>& Abilities);
	
	void InitAbilitiesInputSys();
	
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	UFUNCTION(Server, Reliable)
	void SetMeter(bool isActive);

protected:
	UPROPERTY(Replicated)
	bool isTryingMeterCharge = false;
	
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Character")
	UJFAttributeSet* CoreAttributes;

	UFUNCTION(Server, Unreliable)
	void UpdatePlayerMovementVector(FVector2D MovementVector);

	/** Called for movement input */
	void Move(const FInputActionValue& Value);

	/** Called for looking input */
	void Look(const FInputActionValue& Value);

	void TickQueuedAttack(float DeltaSeconds);

	void TickMeter(float DeltaSeconds);

	FName LastAttack = NAME_None;
	int SyncAttacks(bool isLight = false);
	void LightAttack(bool Queue = true);
	void HeavyAttack(bool Queue = true);

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

