// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "Ability/AbilityData.h"
#include "Ability/JFASComponent.h"
#include "Attributes/JFAttributeSet.h"
#include "Components/ProgressBar.h"
#include "GameFramework/Character.h"
#include "Logging/LogMacros.h"
#include "JFCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UInputMappingContext;
class UInputAction;
struct FInputActionValue;

//Meter
//Each bar is 100, 6x = 600
constexpr float MAX_METER = 600.f;
constexpr float METER_PER_SECOND = 100.f/2.5f;

//Dash
constexpr float MAX_DASH_CHARGE = 400.f;
constexpr float DASH_CHARGE_PER_SECOND = 100.f/3.f;

//Camera
constexpr float CAMERA_LERP_SPEED = 2.5f;

constexpr float LOCK_ON_CAMERA_DIST = 300.f;
constexpr float DEFAULT_CAMERA_DIST = 500.f;

constexpr float CAMERA_LERP_MIN_DIST = 100.f;
constexpr float CAMERA_LERP_MAX_DIST = 800.f;
static const FVector LOCK_ON_MIN_CAMERA_SOCKET_OFFSET = FVector(-50, 300, 100);
static const FVector LOCK_ON_MAX_CAMERA_SOCKET_OFFSET = FVector(0, 100, 50);
static const FVector DEFAULT_CAMERA_SOCKET_OFFSET = FVector(0, 0, 75);

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
	static const inline FGameplayTag DoingSomethingTag = FGameplayTag::RequestGameplayTag(FName("Character.Status.DoingSomething"));
	static const inline FGameplayTag CantMoveTag = FGameplayTag::RequestGameplayTag(FName("Character.Status.CantMove"));

	UFUNCTION(BlueprintCallable, Category = "Character")
	void TakeDamage(float Damage, AJFCharacter* Character);

protected:
	UFUNCTION(Blueprintable)
	void OnDeath(AJFCharacter* Killer);
public:

//========================================================================================>
	
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

	UFUNCTION(BlueprintPure, Category=UI)
	FVector2D GetPlayerInputVector() {return PlayerInputVector;}

	UFUNCTION(BlueprintPure, Category=Dash)
	FVector GetDashVector();

	UFUNCTION(BlueprintPure, Category=UI)
	FVector GetMovementVector();
	
	UFUNCTION(BlueprintPure, Category=UI)
	FVector GetCameraRightVector();

	UFUNCTION(BlueprintPure, Category=UI)
	FVector GetCameraForwardVector();
	
	UFUNCTION(BlueprintPure, Category=UI)
	bool isChargingMeter();

	//Gets Meter Entire Value (ie=600 for max meter)
	UFUNCTION(BlueprintPure, Category=UI)
    float GetMeter()
	{
		return GetNumericAttribute(UJFAttributeSet::GetMeterAttribute());
	}

	UFUNCTION(BlueprintPure, Category=UI)
	int GetMeterText()
	{
		return FMath::Floor(GetMeter()/100.f);
	}

	UFUNCTION(BlueprintPure, Category=UI)
	float GetMeterProgress()
	{
		const float ReturnVal = FMath::Fmod(GetMeter(), 100.0f);
		if(GetMeterText() != 0 && ReturnVal == 0) return 100.f;
		return ReturnVal;
	}

	UFUNCTION(BlueprintCallable, Category=UI)
	void SetMeterProgress(TArray<UProgressBar*> ProgressBars)
	{
		const float MeterTotal = GetMeter();
		const float Split = ProgressBars.Num();

		if(Split == 0) return;

		float cMeter = MeterTotal;
		for(UProgressBar* Bar : ProgressBars)
		{
			const float MeterCharge =
				FMath::Clamp(cMeter, 0.f, 100.f);

			if(Bar) Bar->SetPercent(MeterCharge/100.f);
			cMeter -= MeterCharge;
		}
	}

	UFUNCTION(BlueprintPure, Category=UI)
	float GetDashCharges()
	{
		return GetNumericAttribute(UJFAttributeSet::GetDashChargeAttribute());
	}

	UFUNCTION(BlueprintCallable, Category=UI)
	void SetDashProgressBars(TArray<UProgressBar*> ProgressBars)
	{
		const float Charges = GetDashCharges();
		const float Split = ProgressBars.Num();

		if(Split == 0) return;

		float cCharge = Charges;
		for(UProgressBar* Bar : ProgressBars)
		{
			const float BarCharge =
				FMath::Clamp(cCharge, 0.f, 100.f);

			if(Bar) Bar->SetPercent(BarCharge/100.f);
			cCharge -= BarCharge;
		}
	}

	//a value between 0 and 1
	UFUNCTION(BlueprintPure, Category=UI)
	float GetHealthProgress()
	{
		return
			GetNumericAttribute(UJFAttributeSet::GetHealthAttribute()) /
			GetNumericAttribute(UJFAttributeSet::GetMaxHealthAttribute());
	}

	UFUNCTION(BlueprintPure, Category=UI)
	FString GetHealthText()
	{
		return FString::FromInt(FMath::CeilToInt(
			GetNumericAttribute(UJFAttributeSet::GetHealthAttribute()
			)
		));
	}

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

	/** Lock On Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* LockOnAction;
	
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

public:
	/** Meter Charge FX */
	UPROPERTY(EditAnywhere, Category = "VFX", meta = (AllowPrivateAccess = "true"))
	class UNiagaraComponent* MeterChargeFX;
	
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

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing=PostLockedOnChanged)
	bool isLockedOn = false;
	
	UPROPERTY(BlueprintReadOnly, Replicated)
	AJFCharacter* LockOnCharacter;
	void OnLockOnPressed();

	UFUNCTION(Server, Reliable)
	void setLockedOnServer(bool isLocked = false, AJFCharacter* LockedOnChar = nullptr);

	UFUNCTION()
	void PostLockedOnChanged();

public:
	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }
};

