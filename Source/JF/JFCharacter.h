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
#include "Game/JFGameState.h"
#include "GameFramework/Character.h"
#include "Logging/LogMacros.h"
#include "JFCharacter.generated.h"

class UNiagaraSystem;
class USpringArmComponent;
class UCameraComponent;
class UInputMappingContext;
class UInputAction;
struct FInputActionValue;

//Meter
//Each bar is 100, 6x = 600
constexpr float MAX_METER = 600.f;
constexpr float METER_PER_SECOND = 100.f/1.f;
constexpr float METER_PER_HIT = 0.15f;
constexpr float TS_METER_PER_HIT = 0.05f;

//Dash
constexpr float MAX_DASH_CHARGE = 400.f;
constexpr float DASH_CHARGE_PER_SECOND = 100.f/5.f;

//Camera
constexpr float CAMERA_LERP_SPEED = 25.f;
constexpr float PLAYER_LERP_SPEED = 15.f;

constexpr float LOCK_ON_CAMERA_DIST = 300.f;
constexpr float DEFAULT_CAMERA_DIST = 500.f;

constexpr float CAMERA_LERP_MIN_DIST = 100.f;
constexpr float CAMERA_LERP_MAX_DIST = 800.f;

constexpr float LOCK_ON_CAMERA_HEIGHT_OBSCURED = 100.f;
static const FVector LOCK_ON_MIN_CAMERA_SOCKET_OFFSET = FVector(-75, 75, 125);
static const FVector LOCK_ON_MAX_CAMERA_SOCKET_OFFSET = FVector(0, 75, 50);

static const FVector DEFAULT_CAMERA_SOCKET_OFFSET = FVector(0, 0, 75);

//Parry
constexpr float PARRY_PRE_LAG = 0.05f;
constexpr float PARRY_WINDOW = 0.35f;
constexpr float PARRY_POST_LAG = 0.1f;

//Stuns
constexpr float PARRY_STUN_TIME_PER_HIT = 0.012f;
constexpr float PARRY_STUN_TIME_MIN = 0.8f;
constexpr float PARRY_STUN_TIME_MAX = 2.5f;

constexpr float HIT_STUN_TIME_PER_HIT = 0.03f;
constexpr float HIT_STUN_TIME_MIN = 0.2f;
constexpr float HIT_STUN_TIME_MAX = 1.5f;

constexpr float HIT_STUN_LAUNCH_VEL = 25.f; //How Far We go Per 1 Point of Damage

//Time Stop Attacks (Post Time Stop)
constexpr float TIME_STOP_HIT_DELAY = 0.01; //How Long Between Attacks in TS Can We Do

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FParryAnimation);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FStunAnimation);

USTRUCT()
struct FTimeStopHit
{
	GENERATED_BODY()
	
	UPROPERTY()
	AJFCharacter* Char = nullptr;

	UPROPERTY()
	float Damage = 0;
};

USTRUCT()
struct FAbilityInputHandler
{
	GENERATED_BODY()
public:
	UPROPERTY()
	FGameplayAbilitySpecHandle Handle;
	
	UPROPERTY()
	UInputAction* Action = nullptr;
};

UENUM(BlueprintType)
enum ESoundType
{
	Grunt,
	LightHit,
	HeavyHit,
};

DECLARE_LOG_CATEGORY_EXTERN(LogTemplateCharacter, Log, All);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLockedCharChanged, AJFCharacter*, Target);

UCLASS(config=Game)
class AJFCharacter : public ACharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()
private:
	UPROPERTY()
	TArray<FTimeStopHit> TimeStopHits;
	UPROPERTY()
	float TSHitTime = 0.f;
	void TakeTSHit();
	void TickTSHits(float DeltaSeconds);
	void DamageDealerGiveMeter(AJFCharacter* Dealer, float Damage, bool duringTS = false);
	UFUNCTION(NetMulticast, Reliable)
	void PlayMontageMulticast(UAnimMontage* Montage);
public:
	UPROPERTY(BlueprintReadOnly)
	AJFGameState* GS;

	UFUNCTION(BlueprintCallable)
	void PlayMontageReplicated(UAnimMontage* Montage);
	
	UFUNCTION(BlueprintCallable, Category = "Character")
	void TakeDamage(float Damage, AJFCharacter* DamageDealer, bool IgnoreHitStun = false, bool DuringTimestop = false, AActor* Hitter = nullptr);

	UFUNCTION(NetMulticast, Unreliable)
	void TakeDamageFXs(float Damage);

	UPROPERTY(BlueprintAssignable)
	FParryAnimation ParryAnimationEvent;
	UPROPERTY(BlueprintAssignable)
	FStunAnimation StunAnimationEvent;

	UFUNCTION(Server, Reliable, BlueprintCallable)
	void setMeshVisibilityServer(bool isVisible = true);

	UFUNCTION(Server, Reliable, BlueprintCallable)
	void HealPlayer(float HealAmount);

	UFUNCTION(BlueprintImplementableEvent)
	void onVisibilityChanged(bool isVisible);
protected:
	UFUNCTION(Blueprintable)
	void OnDeath(AJFCharacter* Killer);

	UFUNCTION()
	void onMeshVisibilityChanged()
	{
		if(GetMesh()) GetMesh()->SetVisibility(bMeshVisibility);
		onVisibilityChanged(bMeshVisibility);
	}

	UPROPERTY(ReplicatedUsing=onMeshVisibilityChanged)
	bool bMeshVisibility = true;

	UFUNCTION(NetMulticast, Unreliable)
	void PlaySoundMulti(USoundWave* SoundWave, bool ForcePlay);

	UFUNCTION(NetMulticast, Unreliable)
	void PlaySoundMultiLocation(USoundWave* SoundWave, FVector Location,
		FVector2D VolumeRange = FVector2D(0.75, 1.25),
		FVector2D PitchRange = FVector2D(0.75, 1.25)
	);

	UFUNCTION(NetMulticast, Unreliable)
	void StopSoundMulti();
public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character|Sounds")
	TArray<USoundWave*> AttackGrunts;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character|Sounds")
	TArray<USoundWave*> LightHitSounds;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character|Sounds")
	TArray<USoundWave*> HeavyHitSounds;

	UFUNCTION(BlueprintPure)
	USoundWave* GetAttackGruntSound()
	{
		if(AttackGrunts.Num() == 0) return nullptr;
		int index = FMath::RandRange(0, AttackGrunts.Num() - 1);
		return AttackGrunts[index];
	}

	UFUNCTION(BlueprintPure)
	USoundWave* GetLightAttackSound()
	{
		if(LightHitSounds.Num() == 0) return nullptr;
		int index = FMath::RandRange(0, LightHitSounds.Num() - 1);
		return LightHitSounds[index];
	}

	UFUNCTION(BlueprintPure)
	USoundWave* GetHeavyAttackSound()
	{
		if(HeavyHitSounds.Num() == 0) return nullptr;
		int index = FMath::RandRange(0, HeavyHitSounds.Num() - 1);
		return HeavyHitSounds[index];
	}

	/**
	 * SERVER ONLY FUNCTION
	 * Plays a Sound for this character,
	 * If other sound is playing, does not play unless @param ForcePlay is true
	 * @param Sound The Sound to play
	 * @param ForcePlay If Sound should always play
	 */
	UFUNCTION(BlueprintCallable, Category="Audio", meta=(AdvancedDisplay = "3", UnsafeDuringActorConstruction = "true", Keywords = "play"))
	void PlaySoundByWave(USoundWave* Sound, bool ForcePlay = false);
	/**
	 * SERVER ONLY FUNCTION
	 * Plays a Sound for this character,
	 * If other sound is playing, does not play unless @param ForcePlay is true
	 * @param Sound The Sound Type -- Auto Selects Random From Sound List
	 * @param ForcePlay If Sound should always play
	 */
	UFUNCTION(BlueprintCallable, Category="Audio", meta=(AdvancedDisplay = "3", UnsafeDuringActorConstruction = "true", Keywords = "play"))
	void PlaySoundByType(ESoundType Sound, bool ForcePlay = false);

	/**
	 * SERVER ONLY FUNCTION
	 * Plays a Sound for this character,
	 * @param Sound The Sound to play
	 * @param WorldLocation Location of the object
	 * @param VolumeRange the range for the random volume range
	 * @param PitchRange the range for the random pitch range
	 */
	UFUNCTION(BlueprintCallable, Category="Audio", meta=(AdvancedDisplay = "3", UnsafeDuringActorConstruction = "true", Keywords = "play"))
	void PlaySoundByWaveAtLocation(
		USoundWave* Sound,
		FVector WorldLocation,
		FVector2D VolumeRange = FVector2D(0.75, 1.25),
		FVector2D PitchRange = FVector2D(0.75, 1.25)
	);
	/**
	 * SERVER ONLY FUNCTION
	 * Plays a Sound for this character,
	 * @param Sound The Sound Type -- Auto Selects Random From Sound List
	 * @param WorldLocation Location of the object
	 * @param VolumeRange the range for the random volume range
	 * @param PitchRange the range for the random pitch range
	 */
	UFUNCTION(BlueprintCallable, Category="Audio", meta=(AdvancedDisplay = "3", UnsafeDuringActorConstruction = "true", Keywords = "play"))
	void PlaySoundByTypeAtLocation(
		ESoundType Sound,
		FVector WorldLocation,
		FVector2D VolumeRange = FVector2D(0.75, 1.25),
		FVector2D PitchRange = FVector2D(0.75, 1.25)
	);
	/**
	 * SERVER ONLY FUNCTION
	 * Stops The Sound The Voice Player is Playing.
	 */
	UFUNCTION(BlueprintCallable, Category="Audio")
	void StopSound() {StopSoundMulti();}

	//========================================================================================== COMPONENTS
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character")
	UNiagaraSystem* BloodFX;
	
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Character")
    UJFASComponent* AbilitySystemComponent;
	
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Character")
	UAudioComponent* VoicePlayer;
    
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
	float MaxHealth = 5000;
	
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

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* ParryAction;

	void PossessedBy(AController* NewController) override;
	virtual void OnRep_PlayerState() override;

	UPROPERTY(Replicated)
	FVector2D PlayerInputVector;

public:
	/** Meter Charge FX */
	UPROPERTY(EditAnywhere, Category = "VFX", meta = (AllowPrivateAccess = "true"))
	class UNiagaraComponent* MeterChargeFX;

	/** Parry Eyes */
	UPROPERTY(EditAnywhere, Category = "VFX", meta = (AllowPrivateAccess = "true"))
	class UNiagaraComponent* RParryEyes;

	/** Parry Eyes */
	UPROPERTY(EditAnywhere, Category = "VFX", meta = (AllowPrivateAccess = "true"))
	class UNiagaraComponent* LParryEyes;
	
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
	virtual void Destroyed() override;
	virtual void Tick(float DeltaSeconds) override;

	UFUNCTION(Server, Reliable)
	void SetMeter(bool isActive);

	UFUNCTION(Server, Reliable)
	void Parry();

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnLockedCharChanged LockedOnEvent;

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

	UFUNCTION()
	void TickParry(float DeltaSeconds);

	UFUNCTION()
	void TickStun(float DeltaSeconds, bool ForceEnd = false);

	UFUNCTION()
	void TickHitStun(float DeltaSeconds);

	UFUNCTION(NetMulticast, Reliable)
	void CallParryEvent();

	UFUNCTION(NetMulticast, Reliable)
	void ParryWindowEvent();

	UFUNCTION(NetMulticast, Reliable)
	void ParryEndEvent();
	
	UFUNCTION(NetMulticast, Reliable)
	void CallStunEvent();

	UFUNCTION(Server, Reliable)
	void onParried(float Damage, AJFCharacter* Character);
	
	UPROPERTY()
	float ParryTime = 0;

	UPROPERTY()
	float HitStunTime = 0;

	UPROPERTY()
	float StunTime = 0;
	bool isStunned = false;

	bool bStartParryWindow;
	bool bEndParryWindow;
	bool bIsParrying;

	UFUNCTION()
	void TimeStopEvent(bool isTimeStopped, AJFCharacter* Char);
	bool isTSEventBound = false;
	bool wasCharStopped = false;

	UFUNCTION(NetMulticast, Reliable)
	void TimeStopEffects(bool isTimeStopped);
public:
	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }
};

