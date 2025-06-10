// Copyright ChezyName. All Rights Reserved.

#include "JFCharacter.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "Engine/LocalPlayer.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/AudioComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Controller.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "Ability/JFASComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Net/UnrealNetwork.h"
#include "InputAction.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "InputMappingContext.h"
#include "NiagaraFunctionLibrary.h"
#include "Game/JFGameInstance.h"
#include "Game/JFGameState.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"

class UAbilityData;
DEFINE_LOG_CATEGORY(LogTemplateCharacter);

//////////////////////////////////////////////////////////////////////////
// AJFCharacter

AJFCharacter::AJFCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
		
	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	GetMesh()->CustomDepthStencilValue = 1;
	GetMesh()->bRenderCustomDepth = true;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 99999.f, 0.0f); // ...at this rotation rate

	// Note: For faster iteration times these variables, and many more, can be tweaked in the Character Blueprint
	// instead of recompiling to adjust them
	GetCharacterMovement()->JumpZVelocity = 700.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 99999.f;
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;
	GetCharacterMovement()->MaxAcceleration = 99999.f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = DEFAULT_CAMERA_DIST; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller
	CameraBoom->SocketOffset = FVector(0, 0, 75.f);
	CameraBoom->SetRelativeLocation(FVector(0,0,40));

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named ThirdPersonCharacter (to avoid direct content references in C++)

	AbilitySystemComponent = CreateDefaultSubobject<UJFASComponent>(TEXT("AbilitySystemComp"));
	AbilitySystemComponent->SetIsReplicated(true);
	CoreAttributes = CreateDefaultSubobject<UJFAttributeSet>(TEXT("CoreAttributes"));
	AbilitySystemComponent->AddAttributeSetSubobject(CoreAttributes);

	VoicePlayer = CreateDefaultSubobject<UAudioComponent>(TEXT("VoiceLinePlayer"));
	VoicePlayer->SetupAttachment(GetMesh(), "head");

	//Load Attenuation
	static ConstructorHelpers::FObjectFinder<USoundAttenuation>
		VL_SoundAttenuation(TEXT("/Script/Engine.SoundAttenuation'/Game/_CORE/Audio/VL_Attenuation.VL_Attenuation'"));
	if (VL_SoundAttenuation.Succeeded())
	{
		VoicePlayer->SetAttenuationSettings(VL_SoundAttenuation.Object.Get());
	}

	//Meter Charge FX
	MeterChargeFX = CreateDefaultSubobject<UNiagaraComponent>(TEXT("MeterChargeFX"));
	MeterChargeFX->SetupAttachment(GetMesh());
	MeterChargeFX->SetAutoActivate(false);

	//Parry Eyes Effect (Anime Glowy Eyes)
	RParryEyes = CreateDefaultSubobject<UNiagaraComponent>(TEXT("ParryEyes_R"));
	RParryEyes->SetupAttachment(GetMesh(), "head");
	RParryEyes->SetAutoActivate(true);
	
	LParryEyes = CreateDefaultSubobject<UNiagaraComponent>(TEXT("ParryEyes_L"));
	LParryEyes->SetupAttachment(GetMesh(), "head");
	LParryEyes->SetAutoActivate(true);

	LParryEyes->SetVisibility(false);
	RParryEyes->SetVisibility(false);

	//Position Parry VFXs
	RParryEyes->SetRelativeLocation(FVector(5, -11, 10));
	LParryEyes->SetRelativeLocation(FVector(-5, -11, 10));

	//Load TS FX
	static ConstructorHelpers::FObjectFinder<UMaterial>
		TWMaterial(TEXT("/Script/Engine.Material'/Game/Characters/DIO/TimeStop_PP.TimeStop_PP'"));
	if (TWMaterial.Succeeded() && GetFollowCamera())
	{
		FPostProcessSettings& PPS = GetFollowCamera()->PostProcessSettings;
		PPS.WeightedBlendables.Array.Add(FWeightedBlendable(0.f, TWMaterial.Object));
	}

	//Load Dash Ability
	static ConstructorHelpers::FObjectFinder<UAbilityData>
		DataAssetRef(TEXT("/Script/JF.AbilityData'/Game/Characters/_Core/_Dash/DashAbilityData.DashAbilityData'"));
	if (DataAssetRef.Succeeded())
	{
		DashAbility = DataAssetRef.Object;
	}

	//Default Input Bindings for Auto Creation

	//Input Mapping
	static ConstructorHelpers::FObjectFinder<UInputMappingContext>
		InputMapping(TEXT("/Script/EnhancedInput.InputMappingContext'/Game/_CORE/Input/IMC_Default.IMC_Default'"));
	if (InputMapping.Succeeded())
	{
		DefaultMappingContext = InputMapping.Object;
	}

	//Move Action
	static ConstructorHelpers::FObjectFinder<UInputAction>
		MoveActionFinder(TEXT("/Script/EnhancedInput.InputAction'/Game/_CORE/Input/Actions/IA_Move.IA_Move'"));
	if (MoveActionFinder.Succeeded())
	{
		MoveAction = MoveActionFinder.Object;
	}

	//Look Action
	static ConstructorHelpers::FObjectFinder<UInputAction>
		LookActionFinder(TEXT("/Script/EnhancedInput.InputAction'/Game/_CORE/Input/Actions/IA_Look.IA_Look'"));
	if (LookActionFinder.Succeeded())
	{
		LookAction = LookActionFinder.Object;
	}

	//Lock On Action
	static ConstructorHelpers::FObjectFinder<UInputAction>
		LockOnActionFinder(TEXT("/Script/EnhancedInput.InputAction'/Game/_CORE/Input/Actions/IA_CameraLock.IA_CameraLock'"));
	if (LockOnActionFinder.Succeeded())
	{
		LockOnAction = LockOnActionFinder.Object;
	}

	//Light Attack
	static ConstructorHelpers::FObjectFinder<UInputAction>
		LightAttackActionFinder(TEXT("/Script/EnhancedInput.InputAction'/Game/_CORE/Input/AttackActions/IA_LightAttack.IA_LightAttack'"));
	if (LightAttackActionFinder.Succeeded())
	{
		LightAttackAction = LightAttackActionFinder.Object;
	}

	//Heavy Attack
	static ConstructorHelpers::FObjectFinder<UInputAction>
		HeavyAttackActionFinder(TEXT("/Script/EnhancedInput.InputAction'/Game/_CORE/Input/AttackActions/IA_HeavyAttack.IA_HeavyAttack'"));
	if (HeavyAttackActionFinder.Succeeded())
	{
		HeavyAttackAction = HeavyAttackActionFinder.Object;
	}

	//Meter Charge
	static ConstructorHelpers::FObjectFinder<UInputAction>
		MeterChargeActionFinder(TEXT("InputAction'/Game/_CORE/Input/Actions/IA_Meter_Charge.IA_Meter_Charge'"));

	if (MeterChargeActionFinder.Succeeded())
	{
		MeterChargeAction = MeterChargeActionFinder.Object;
	}

	//Parry
	static ConstructorHelpers::FObjectFinder<UInputAction>
		ParryActionFinder(TEXT("/Script/EnhancedInput.InputAction'/Game/_CORE/Input/Actions/IA_Parry.IA_Parry'"));

	if (ParryActionFinder.Succeeded())
	{
		ParryAction = ParryActionFinder.Object;
	}

	// Load Niagara System asset - Meter Charge FX
	static ConstructorHelpers::FObjectFinder<UNiagaraSystem>
		MeterChargeFXFinder(TEXT("NiagaraSystem'/Game/Characters/_Core/Meter/MeterCharge.MeterCharge'"));
	if (MeterChargeFX && MeterChargeFXFinder.Succeeded())
	{
		MeterChargeFX->SetAsset(MeterChargeFXFinder.Object.Get());
	}

	// Load Niagara System asset - Meter Charge FX
	static ConstructorHelpers::FObjectFinder<UNiagaraSystem>
		ParryVFXFinder(TEXT("NiagaraSystem'/Game/Characters/_Core/Parry/ParryEyes.ParryEyes'"));
	if (LParryEyes && RParryEyes && ParryVFXFinder.Succeeded())
	{
		LParryEyes->SetAsset(ParryVFXFinder.Object.Get());
		RParryEyes->SetAsset(ParryVFXFinder.Object.Get());
	}

	
	// Load Niagara System asset - Blood FX
    static ConstructorHelpers::FObjectFinder<UNiagaraSystem>
    	BloodFXFinder(TEXT("NiagaraSystem'/Game/Characters/_Core/_Blood/BloodSplatter_v2.BloodSplatter_v2'"));
    if (BloodFXFinder.Succeeded())
    {
    	BloodFX = BloodFXFinder.Object.Get();
    }
}

void AJFCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->InitAbilityActorInfo(this, this);
	}

	// ASC MixedMode replication requires that the ASC Owner's Owner be the Controller.
	SetOwner(NewController);
}

void AJFCharacter::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();

	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->InitAbilityActorInfo(this, this);
	}
}

//////////////////////////////////////////////////////////////////////////
// Input

void AJFCharacter::NotifyControllerChanged()
{
	Super::NotifyControllerChanged();

	// Add Input Mapping Context
	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}
}

void AJFCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent)) {
		//Ability System
		AbilitySystemComponent->SetInputComponent(EnhancedInputComponent);
		
		InitAbilitiesInputSys();
		
		// Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AJFCharacter::Move);
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Completed, this, &AJFCharacter::Move);

		// Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AJFCharacter::Look);

		//Lock On
		EnhancedInputComponent->BindAction(LockOnAction, ETriggerEvent::Triggered, this, &AJFCharacter::OnLockOnPressed);

		//Light Attack
		EnhancedInputComponent->BindAction(LightAttackAction, ETriggerEvent::Triggered,
			this, &AJFCharacter::LightAttack, true);

		//Heavy Attack
		EnhancedInputComponent->BindAction(HeavyAttackAction, ETriggerEvent::Triggered,
			this, &AJFCharacter::HeavyAttack, true);

		//Meter Charge
		EnhancedInputComponent->BindAction(MeterChargeAction, ETriggerEvent::Triggered, this, &AJFCharacter::SetMeter, true);
		EnhancedInputComponent->BindAction(MeterChargeAction, ETriggerEvent::Completed, this, &AJFCharacter::SetMeter, false);

		//Parry
		EnhancedInputComponent->BindAction(ParryAction, ETriggerEvent::Triggered, this, &AJFCharacter::Parry);
	}
	else
	{
		UE_LOG(LogTemplateCharacter, Error, TEXT("'%s' Failed to find an Enhanced Input component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}
}

void AJFCharacter::UpdatePlayerMovementVector_Implementation(FVector2D MovementVector)
{
	PlayerInputVector = MovementVector;
}

FVector AJFCharacter::GetMovementVector()
{
	if (Controller != nullptr)
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	
		// get right vector 
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		return (ForwardDirection * PlayerInputVector.Y) + (RightDirection * PlayerInputVector.X);
	}

	return FVector::ZeroVector;
}

FVector AJFCharacter::GetDashVector()
{
	if(isLockedOn)
	{
		FVector ForwardVec = GetActorForwardVector();
		FVector RightVec = GetActorRightVector();
		
		FVector2D Input = GetPlayerInputVector();
		if(Input == FVector2D::ZeroVector)
		{
			return ForwardVec * -1;
		}

		return (ForwardVec * Input.Y) + (RightVec * Input.X);
	}

	//Not Locked on Camera
	FVector Movement = GetMovementVector();
	if(Movement == FVector::ZeroVector)
	{
		return GetCameraForwardVector() * -1;
	}

	return Movement;
}

FVector AJFCharacter::GetCameraRightVector()
{
	if (Controller != nullptr)
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);
		
		// get right vector 
		return FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
	}

	return FVector::ZeroVector;
}

FVector AJFCharacter::GetCameraForwardVector()
{
	if (Controller != nullptr)
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		return FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	}

	return FVector::ZeroVector;
}

void AJFCharacter::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();
	UpdatePlayerMovementVector(MovementVector);

	if (Controller != nullptr && !isChargingMeter()
		&& !AbilitySystemComponent->HasMatchingGameplayTag(UJFGameInstance::DoingSomethingTag)
		&& !AbilitySystemComponent->HasMatchingGameplayTag(UJFGameInstance::CantMoveTag)
		&& !AbilitySystemComponent->HasMatchingGameplayTag(UJFGameInstance::GAHitStunTag)
		&& !AbilitySystemComponent->HasMatchingGameplayTag(UJFGameInstance::GrabbedTag)
	)
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	
		// get right vector 
		FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		
		if(isLockedOn)
		{
			ForwardDirection = GetActorForwardVector();
			RightDirection = GetActorRightVector();
		}

		// add movement 
		AddMovementInput(ForwardDirection, MovementVector.Y);
		AddMovementInput(RightDirection, MovementVector.X);
	}
}

void AJFCharacter::Look(const FInputActionValue& Value)
{
	if(isLockedOn && LockOnCharacter) return;
	
	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}

void AJFCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AJFCharacter, PlayerInputVector);
	DOREPLIFETIME(AJFCharacter, isTryingMeterCharge);
	DOREPLIFETIME(AJFCharacter, isLockedOn);
	DOREPLIFETIME(AJFCharacter, LockOnCharacter);
	DOREPLIFETIME(AJFCharacter, bMeshVisibility);
}

/*
void AJFCharacter::InitAbilities()
{
	const int ABILITY_LVL = 1;
	//Grant Abilities to This Actor
	if(DashAbility != nullptr && DashAbility->Ability != nullptr)
	{
		const FGameplayAbilitySpec AbilitySpec = FGameplayAbilitySpec(DashAbility->Ability, ABILITY_LVL,
			static_cast<uint8>(DashAbility->AbilityKey));
		AbilitySystemComponent->GiveAbility(AbilitySpec);
	}

	for(int i = 0; i < CharacterAbilities.Num(); i++)
	{
		UAbilityData* Ability = CharacterAbilities[i];
		if(Ability == nullptr || Ability->Ability == nullptr) continue;
		UKismetSystemLibrary::PrintString(GetWorld(), "Initing Ability: " + Ability->AbilityName + " on Key [" +
			Ability->AbilityAction->GetName() + "] for " + GetName(), true,true,FLinearColor::Green,30);
		
		const FGameplayAbilitySpec AbilitySpec = FGameplayAbilitySpec(Ability->Ability, ABILITY_LVL,
			static_cast<uint8>(Ability->AbilityKey));
		AbilitySystemComponent->GiveAbility(AbilitySpec);
	}
}
*/

void AJFCharacter::InitAbilitiesServer_Implementation()
{
	//Dash / Dodge Ability
	if(DashAbility != nullptr && DashAbility->Ability != nullptr)
	{
		AbilitySystemComponent->GiveAbility(DashAbility->Ability);
	}

	//Character Abilities
	for(int i = 0; i < CharacterAbilities.Num(); i++)
	{
		UAbilityData* Ability = CharacterAbilities[i];
		if(Ability == nullptr || Ability->Ability == nullptr) continue;
		AbilitySystemComponent->GiveAbility(Ability->Ability);
	}

	//Init Attack Abilities -> Light / Heavy Attacks
	for (const TSubclassOf<UGameplayAbility>& AbilityClass : LightAttacks)
	{
		AbilitySystemComponent->GiveAbility(FGameplayAbilitySpec(AbilityClass, 1, INDEX_NONE, this));
	}

	for (const TSubclassOf<UGameplayAbility>& AbilityClass : HeavyAttacks)
	{
		AbilitySystemComponent->GiveAbility(FGameplayAbilitySpec(AbilityClass, 1, INDEX_NONE, this));
	}
}

void AJFCharacter::InitAbilitiesClient_Implementation(const TArray<FAbilityInputHandler>& Abilities)
{
	for(FAbilityInputHandler Ability : Abilities)
	{
		AbilitySystemComponent->SetInputBinding(Ability.Action, Ability.Handle);
	}
}

void AJFCharacter::InitAbilitiesInputSys()
{
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(InputComponent))
	{
		//UKismetSystemLibrary::PrintString(GetWorld(), "Input System [HANDSHAKE] Ability System",
		//	true,true,FLinearColor::Green,30);
		AbilitySystemComponent->SetInputComponent(EnhancedInputComponent);
	}
	
	//Init
	InitAbilitiesServer();
	
	//UKismetSystemLibrary::PrintString(GetWorld(), "==================================================================",
	//	true,true,FLinearColor::Green,30);
}

void AJFCharacter::TimeStopEvent(bool isTimeStopped, AJFCharacter* Char)
{
	TimeStopEffects(isTimeStopped);
	
	if(isTimeStopped && Char != this)
	{
		//We Are The TS Target
		if(HasAuthority())
		{
			AbilitySystemComponent->AddReplicatedLooseGameplayTag(UJFGameInstance::DoingSomethingTag);
			AbilitySystemComponent->AddLooseGameplayTag(UJFGameInstance::DoingSomethingTag);
			
			AbilitySystemComponent->AddReplicatedLooseGameplayTag(UJFGameInstance::CantMoveTag);
			AbilitySystemComponent->AddLooseGameplayTag(UJFGameInstance::CantMoveTag);
			wasCharStopped = true;
		}

		//Stop Animation Movement
		GetMesh()->bPauseAnims = true;
	}

	if(!isTimeStopped)
	{
		if(wasCharStopped)
		{
			AbilitySystemComponent->RemoveReplicatedLooseGameplayTag(UJFGameInstance::DoingSomethingTag);
			AbilitySystemComponent->RemoveLooseGameplayTag(UJFGameInstance::DoingSomethingTag);
			
			AbilitySystemComponent->RemoveReplicatedLooseGameplayTag(UJFGameInstance::CantMoveTag);
			AbilitySystemComponent->RemoveLooseGameplayTag(UJFGameInstance::CantMoveTag);
		}

		GetMesh()->bPauseAnims = false;

		if(HasAuthority())
		{
			//Do TS Hits
			
		}
	}
}

void AJFCharacter::TimeStopEffects_Implementation(bool isTimeStopped)
{
	//Time Stop Effects
	FPostProcessSettings& PPS = GetFollowCamera()->PostProcessSettings;
	if(PPS.WeightedBlendables.Array.Num() > 0)
	{
		PPS.WeightedBlendables.Array[0].Weight = isTimeStopped ? 1.f : 0.f;
	}
}

void AJFCharacter::BeginPlay()
{
	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->InitAbilityActorInfo(this, this);
	}

	if(AJFGameState* _GS = Cast<AJFGameState>(GetWorld()->GetGameState()))
	{
		GS = _GS;
		GS->TimeStopEvent.AddDynamic(this, &AJFCharacter::TimeStopEvent);
		isTSEventBound = true;
	}

	//InitAbilities();

	//Init Base Attributes
	if(HasAuthority())
	{
		//Internal Clamp Filter will set value to zero if MaxHealth is not init'd first
		SetNumericAttribute(UJFAttributeSet::GetMaxHealthAttribute(), MaxHealth);
		SetNumericAttribute(UJFAttributeSet::GetHealthAttribute(), MaxHealth);

		SetNumericAttribute(UJFAttributeSet::GetMovementSpeedAttribute(), MovementSpeed);

		SetNumericAttribute(UJFAttributeSet::GetDashChargeAttribute(), MAX_DASH_CHARGE);

		//DEBUG ONLY
		SetNumericAttribute(UJFAttributeSet::GetMeterAttribute(), MAX_METER);
	}
	
	Super::BeginPlay();
}

void AJFCharacter::Destroyed()
{
	if(AJFGameState* _GS = Cast<AJFGameState>(GetWorld()->GetGameState()))
	{
		_GS->TimeStopEvent.RemoveDynamic(this, &AJFCharacter::TimeStopEvent);
		isTSEventBound = false;
	}
	Super::Destroyed();
}

void AJFCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	if(IsLocallyControlled()) TickQueuedAttack(DeltaSeconds);

	//VFX & Cant Move During Meter Charge
	MeterChargeFX->SetActive(isChargingMeter());

	//Player Look At
	if(isLockedOn && LockOnCharacter &&
		!AbilitySystemComponent->HasMatchingGameplayTag(UJFGameInstance::GrabbedTag) &&
		!AbilitySystemComponent->HasMatchingGameplayTag(UJFGameInstance::ParryStunTag) &&
		!GS->IsTimeStopped())
	{
		//Player Look At Target
		FVector PlayerLocation = GetActorLocation();
		FVector TargetLocation = LockOnCharacter->GetActorLocation();

		FRotator PlayerRotation = UKismetMathLibrary::FindLookAtRotation(PlayerLocation, TargetLocation);
		PlayerRotation.Pitch = GetActorRotation().Pitch;
		PlayerRotation.Roll = GetActorRotation().Roll;

		SetActorRotation(PlayerRotation);
	}

	//Lock on Camera
	if(IsLocallyControlled())
	{
		//Socket Length & Socket Offset
		if(GetCameraBoom())
		{
			GetCameraBoom()->TargetArmLength = FMath::FInterpTo(
				GetCameraBoom()->TargetArmLength,
				isLockedOn ? LOCK_ON_CAMERA_DIST : DEFAULT_CAMERA_DIST,
				DeltaSeconds,
				CAMERA_LERP_SPEED
			);

			FVector FinalLockedOnSocketOffset = DEFAULT_CAMERA_SOCKET_OFFSET;

			if(isLockedOn && LockOnCharacter)
			{
				const float TargetDist = FVector::Dist(GetActorLocation(), LockOnCharacter->GetActorLocation());
				const float Alpha = FMath::GetMappedRangeValueClamped(
					FVector2D(CAMERA_LERP_MIN_DIST, CAMERA_LERP_MAX_DIST),
					FVector2D(0.0f, 1.0f),
					TargetDist
				);
				FinalLockedOnSocketOffset =
					FMath::Lerp(LOCK_ON_MIN_CAMERA_SOCKET_OFFSET, LOCK_ON_MAX_CAMERA_SOCKET_OFFSET, Alpha);
				
				
				UKismetSystemLibrary::PrintString(GetWorld(),
					"Target Distance: " + FString::SanitizeFloat(TargetDist), true,
					true, FLinearColor::Yellow, 2, FName("TargetLock"));
			}
			
			GetCameraBoom()->SocketOffset = FMath::VInterpTo(
			GetCameraBoom()->SocketOffset,
			FinalLockedOnSocketOffset,
			DeltaSeconds,
				CAMERA_LERP_SPEED
			);

			//Camera Look at Target
			if(GetFollowCamera() && LockOnCharacter)
			{
				FRotator LookAtRot = UKismetMathLibrary::FindLookAtRotation(\
					GetFollowCamera()->GetComponentLocation(),
					LockOnCharacter->GetActorLocation());

				GetFollowCamera()->SetWorldRotation(LookAtRot);
			}
		}
	}

	if(HasAuthority())
	{
		TickMeter(DeltaSeconds);
		TickParry(DeltaSeconds);
		TickStun(DeltaSeconds);
		TickHitStun(DeltaSeconds);
		TickTSHits(DeltaSeconds);
		
		//If Gameplay Tags == Light or Heavy -> Ignore (Using Specific Ability)
		bool hasAttackTags = false;

		if(GetAbilitySystemComponent())
		{
			FGameplayTagContainer TagsToCheck;
			TagsToCheck.AddTag(FGameplayTag::RequestGameplayTag(FName("Character.Attacking.Light")));
			TagsToCheck.AddTag(FGameplayTag::RequestGameplayTag(FName("Character.Attacking.Heavy")));
			TagsToCheck.AddTag(FGameplayTag::RequestGameplayTag(FName("Character.Status.DoingSomething")));

			hasAttackTags = GetAbilitySystemComponent()->HasAnyMatchingGameplayTags(TagsToCheck);
		}

		if(!hasAttackTags)
		{
			//Tick down Attack Combo Reset Timers
			float ComboCDR =
				GetNumericAttribute(UJFAttributeSet::GetComboResetTimeAttribute());
		
			ComboCDR = FMath::Max(ComboCDR - DeltaSeconds, 0.f);

			SetNumericAttribute(UJFAttributeSet::GetComboResetTimeAttribute(), ComboCDR);

			//Reset Combo if Reset Time <= 0
			if(ComboCDR <= 0)
			{
				SetNumericAttribute(UJFAttributeSet::GetLightAttackComboAttribute(), 0.f);
				SetNumericAttribute(UJFAttributeSet::GetHeavyAttackComboAttribute(), 0.f);
				LastAttack = NAME_None;
			}
		}

		//Tick Dash Charge
		float cDashCharge = GetNumericAttribute(UJFAttributeSet::GetDashChargeAttribute());
		cDashCharge += DeltaSeconds * DASH_CHARGE_PER_SECOND;
		cDashCharge = FMath::Clamp(cDashCharge, 0.f, MAX_DASH_CHARGE);
		SetNumericAttribute(UJFAttributeSet::GetDashChargeAttribute(), cDashCharge);
	}
}

void AJFCharacter::SetMeter_Implementation(bool isActive)
{
	isTryingMeterCharge = isActive;
}

bool AJFCharacter::isChargingMeter()
{
	if(!GetAbilitySystemComponent()) return false;
	
	return !AbilitySystemComponent->HasMatchingGameplayTag(UJFGameInstance::DoingSomethingTag)
	&& !AbilitySystemComponent->HasMatchingGameplayTag(UJFGameInstance::GAHitStunTag)
	&& !AbilitySystemComponent->HasMatchingGameplayTag(UJFGameInstance::GrabbedTag)
	&& isTryingMeterCharge;
}

void AJFCharacter::TickMeter(float DeltaSeconds)
{
	//Tick The Meter, Assume is Server
	if(isChargingMeter())
	{
		//Try Meter Charge
		const float CurrMeter = GetNumericAttribute(UJFAttributeSet::GetMeterAttribute());

		SetNumericAttribute(UJFAttributeSet::GetMeterAttribute(),
			FMath::Clamp(CurrMeter + METER_PER_SECOND*DeltaSeconds, 0.f, MAX_METER));
	}
}

//====================================================================================
// Attack Functions

int AJFCharacter::SyncAttacks(bool isLight)
{
	//Same Attacks - Return Current Index
	if((LastAttack == "Light" && isLight) ||
		(LastAttack == "Heavy" && !isLight) || LastAttack == NAME_None)
	{
		//return base
		if(isLight) return GetNumericAttribute(UJFAttributeSet::GetLightAttackComboAttribute());
		return GetNumericAttribute(UJFAttributeSet::GetHeavyAttackComboAttribute());
	}

	//Last Attack is not same as Current
	//Convert Last Attack Index into Current Attack Index
	const float base = GetNumericAttribute(!isLight ?
		UJFAttributeSet::GetLightAttackComboAttribute() :
		UJFAttributeSet::GetHeavyAttackComboAttribute());
	
	const float div = (!isLight ? LightAttacks.Num() : HeavyAttacks.Num());
	const float prog = (base/div);
	
	const float opp_base = GetNumericAttribute(isLight ?
    	UJFAttributeSet::GetLightAttackComboAttribute() :
    	UJFAttributeSet::GetHeavyAttackComboAttribute());
	const float opp_div = (!isLight ? LightAttacks.Num() : HeavyAttacks.Num());

	//Calculate New Attack Index
	const float BaseIndex = prog * opp_div;
	const int Round = FMath::CeilToInt(BaseIndex);
	const int IndexClamped = FMath::Clamp(Round, 0, opp_div - 1);
	const int Final = FMath::Max(IndexClamped, opp_base + 1);

	/*
	UKismetSystemLibrary::PrintString(GetWorld(), "Prog   : " + FString::SanitizeFloat(prog));
	UKismetSystemLibrary::PrintString(GetWorld(), "   Base: " + FString::SanitizeFloat(base));
	UKismetSystemLibrary::PrintString(GetWorld(), "   Div : " + FString::SanitizeFloat(div));
	UKismetSystemLibrary::PrintString(GetWorld(), "==================================");
	UKismetSystemLibrary::PrintString(GetWorld(), "Base Index   : " + FString::SanitizeFloat(BaseIndex));
	UKismetSystemLibrary::PrintString(GetWorld(), "Round Index  : " + FString::SanitizeFloat(Round));
	UKismetSystemLibrary::PrintString(GetWorld(), "Index Clamped: " + FString::SanitizeFloat(IndexClamped));
	UKismetSystemLibrary::PrintString(GetWorld(), "Final        : " + FString::SanitizeFloat(Final));
	*/
	
	return Final;
}

void AJFCharacter::LightAttack(bool Queue)
{
	if(LightAttacks.Num() == 0) return;
	if(AbilitySystemComponent == nullptr) return;

	const int CurrComboNumber = SyncAttacks(true);
	
	//Server Func for Light Attack
	const int ComboNumber = CurrComboNumber % LightAttacks.Num();

	//Call Ability, Do Not Queue
	if(!Queue)
	{
		if(AbilitySystemComponent->TryActivateAbilityByClass(LightAttacks[ComboNumber]))
		{
			SetNumericAttribute(
				UJFAttributeSet::GetLightAttackComboAttribute(),
				ComboNumber
			);
		
			LastAttack = "Light";
					
			FQueuedAbility& Attack = AbilitySystemComponent->GetNextAbility();
			Attack.Lifetime = -1;
		}
		return;
	}

	if(!AbilitySystemComponent->TryActivateOrQueueAbilityByClass(LightAttacks[ComboNumber]))
	{
		AbilitySystemComponent->GetNextAbility().Type = Light;
		UKismetSystemLibrary::PrintString(GetWorld(), "Light Attack Queued");
	}
	else
	{
		SetNumericAttribute(
			UJFAttributeSet::GetLightAttackComboAttribute(),
			ComboNumber
		);
		
		LastAttack = "Light";
	}
}

void AJFCharacter::HeavyAttack(bool Queue)
{
	if(HeavyAttacks.Num() == 0) return;
	if(AbilitySystemComponent == nullptr) return;
	
	//Server Func for Light Attack
	const int CurrComboNumber = SyncAttacks();
	
	//Server Func for Light Attack
	const int ComboNumber = CurrComboNumber % HeavyAttacks.Num();

	//Call Ability, Do Not Queue
	if(!Queue)
	{
		if(AbilitySystemComponent->TryActivateAbilityByClass(HeavyAttacks[ComboNumber]))
		{
			SetNumericAttribute(
				UJFAttributeSet::GetHeavyAttackComboAttribute(),
				ComboNumber
			);
		
			LastAttack = "Heavy";
			
			FQueuedAbility& Attack = AbilitySystemComponent->GetNextAbility();
			Attack.Lifetime = -1;
		}
		return;
	}

	if(!AbilitySystemComponent->TryActivateOrQueueAbilityByClass(HeavyAttacks[ComboNumber]))
	{
		AbilitySystemComponent->GetNextAbility().Type = Heavy;
		UKismetSystemLibrary::PrintString(GetWorld(), "Heavy Attack Queued");
	}
	else
	{
		SetNumericAttribute(
			UJFAttributeSet::GetHeavyAttackComboAttribute(),
			ComboNumber
		);
		
		LastAttack = "Heavy";
	}
}


void AJFCharacter::TickQueuedAttack(float DeltaSeconds)
{
	//Only Tick if Attack Light/Heavy
	if(AbilitySystemComponent == nullptr) return;
	FQueuedAbility& Attack = AbilitySystemComponent->GetNextAbility();
	if(Attack.Type == Ability || Attack.Lifetime <= 0) return;

	//Tick Attack
	Attack.Lifetime -= DeltaSeconds;
	
	if(Attack.Type == Light) LightAttack(false);
	if(Attack.Type == Heavy) HeavyAttack(false);
}

void AJFCharacter::HealPlayer_Implementation(float HealAmount)
{
	if(HealAmount <= 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("Cannot Heal Number Smaller Than 0, Healing=[%f]"), HealAmount);
		return;
	}
	float cHealth = GetNumericAttribute(UJFAttributeSet::GetHealthAttribute());
	float cMaxHealth = GetNumericAttribute(UJFAttributeSet::GetMaxHealthAttribute());
	
	cHealth = FMath::Clamp(cHealth + HealAmount, 0.f, cMaxHealth);
	SetNumericAttribute(UJFAttributeSet::GetHealthAttribute(), cHealth);
}

void AJFCharacter::TickTSHits(float DeltaSeconds)
{
	if(TimeStopHits.Num() <= 0 || !HasAuthority()) return;
	TSHitTime -= DeltaSeconds;
	if(TSHitTime <= 0) TakeTSHit();
}

void AJFCharacter::TakeTSHit()
{
	if((GS && GS->IsTimeStopped()) || TSHitTime > 0) return;
	FTimeStopHit Hit = TimeStopHits.Pop();
	TakeDamage(Hit.Damage, Hit.Char);
	TSHitTime = TIME_STOP_HIT_DELAY;
}

//Damage Function
void AJFCharacter::TakeDamage(float Damage, AJFCharacter* DamageDealer, bool IgnoreHitStun)
{
	if(DamageDealer == nullptr || Damage == 0) return;

	//If In TimeStop
	if(GS && GS->IsTimeStopped())
	{
		//Put Damage in Damage Log
		TimeStopHits.Add({
			DamageDealer,
			Damage
		});
		return;
	}

	//Server Only Function
	if(!HasAuthority())
	{
		UE_LOG(LogTemp, Warning, TEXT("Cannot Take Damage from Client Version"));
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("%s is Damaging %s for %f"), *DamageDealer->GetName(),
		*GetName(), Damage);

	//Check for Parry
	if(AbilitySystemComponent->HasMatchingGameplayTag(UJFGameInstance::ParryTag))
	{
		//Parry Attack & End Parry
		DamageDealer->onParried(Damage, this);
		TickParry(999);

		//End Parry Window Early Since Attack was Parried
		GEngine->AddOnScreenDebugMessage(-1,25,FColor::Green, "Ending Parry Window Early -> Attack Parried");
		ParryTime = 0;
		return;
	}

	//End Stun if Stunned
	TickStun(0, true);

	//Deal Damage
	float cHealth = GetNumericAttribute(UJFAttributeSet::GetHealthAttribute());
	cHealth -= Damage;
	SetNumericAttribute(UJFAttributeSet::GetHealthAttribute(), cHealth);

	//FXS - Scale Damage (Blood FX) for Project - Seals
	TakeDamageFXs(Damage);

	//Give other player some meter
	DamageDealerGiveMeter(DamageDealer, Damage);
	
	if(!IgnoreHitStun)
	{
		//Hit Stun
		HitStunTime = HIT_STUN_TIME;
		AbilitySystemComponent->AddGameplayCue(UJFGameInstance::HitStunTag);
		AbilitySystemComponent->AddLooseGameplayTag(UJFGameInstance::GAHitStunTag);
		AbilitySystemComponent->AddReplicatedLooseGameplayTag(UJFGameInstance::GAHitStunTag);

		//Knock Back
		float LaunchBackVel = FMath::Clamp(Damage * HIT_STUN_LAUNCH_VEL, 0.f, 1000.f);
		FVector LaunchBack = DamageDealer->GetActorForwardVector() * LaunchBackVel;
		LaunchCharacter(LaunchBack, true, false);
	}

	if(cHealth <= 0) OnDeath(DamageDealer);
}

void AJFCharacter::DamageDealerGiveMeter(AJFCharacter* Dealer, float Damage)
{
	if(!HasAuthority() || Dealer == nullptr || Damage <= 0) return;

	//Take Meter
	float Meter = Dealer->GetNumericAttribute(UJFAttributeSet::GetMeterAttribute());
	Meter += Damage * METER_PER_HIT;
	Meter = FMath::Clamp(Meter, 0, MAX_METER);

	Dealer->SetNumericAttribute(UJFAttributeSet::GetMeterAttribute(), Meter);
}

void AJFCharacter::TakeDamageFXs_Implementation(float Damage)
{
	if(!BloodFX) return;
	
	UNiagaraComponent* NiagaraComp = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
		GetWorld(),
		BloodFX,
		GetActorLocation(),
		GetActorRotation(),
		FVector(1.0f),    // Scale
		true,             // bAutoDestroy
		true,             // bAutoActivate
		ENCPoolMethod::None,
		true              // bPreCullCheck
	);

	if (NiagaraComp)
	{
		NiagaraComp->SetVariableInt(TEXT("Damage"), Damage);
	}
}

void AJFCharacter::setMeshVisibilityServer_Implementation(bool isVisible)
{
	bMeshVisibility = isVisible;
	onMeshVisibilityChanged();
}

void AJFCharacter::OnDeath(AJFCharacter* Killer)
{
	if(Killer == nullptr) return;

	//Server Only Function
	if(!HasAuthority())
	{
		UE_LOG(LogTemp, Warning, TEXT("Cannot Kill Character from Client Version"));
		return;
	}
	
	UE_LOG(LogTemp, Log, TEXT("%s has KILLED: %s"), *Killer->GetName(),
		*GetName())

	//Kill User
}

//Camera Lock on / Delock
void AJFCharacter::OnLockOnPressed()
{
	//Either Lock on or Release Lock on Back to Normal Camera Conditions
	if(isLockedOn)
	{
		isLockedOn = false;
		LockOnCharacter = nullptr;
		setLockedOnServer();
		GetController()->SetControlRotation(GetCameraBoom()->GetTargetRotation());
		GetCameraBoom()->bUsePawnControlRotation = true;
		GetFollowCamera()->SetRelativeRotation(FRotator::ZeroRotator);
		LockedOnEvent.Broadcast(LockOnCharacter);
		UKismetSystemLibrary::PrintString(GetWorld(), "Target Un-Locked");
		return;
	}

	UKismetSystemLibrary::PrintString(GetWorld(), "Looking for Lock on Target");

	//Lock on Target
	TArray<AActor*> Enemies;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AJFCharacter::StaticClass(), Enemies);

	AActor* FinalTarget = nullptr;
	float FinalDot = -1;
	for (AActor* Target : Enemies)
	{
		if(Target == this) continue;
		
		FVector ToTarget = Target->GetActorLocation() - GetActorLocation();
		float Distance = ToTarget.Size();

		FVector Forward = GetControlRotation().Vector();
		float Dot = FVector::DotProduct(Forward.GetSafeNormal(), ToTarget.GetSafeNormal());

		//FOV Angle = 90deg
		//if (Dot < FMath::Cos(90.f * 0.5f)) continue; // angle check

		//Check if Better Target
		float TargetDot = FVector::DotProduct(GetCameraForwardVector(),
				(Target->GetActorLocation() - GetFollowCamera()->GetComponentLocation()).GetSafeNormal());

		if(TargetDot > FinalDot || FinalDot == -1)
		{
			FinalTarget = Target;
			FinalDot = TargetDot;
		}
	}

	if(FinalTarget)
	{
		GetController()->SetControlRotation(FRotator::ZeroRotator);
		LockOnCharacter = static_cast<AJFCharacter*>(FinalTarget);
		isLockedOn = true;
		setLockedOnServer(true, LockOnCharacter);
		GetCameraBoom()->bUsePawnControlRotation = false;
		LockedOnEvent.Broadcast(LockOnCharacter);

		UKismetSystemLibrary::PrintString(GetWorld(), "Target Locked: " + FinalTarget->GetName());
	}
}

void AJFCharacter::PostLockedOnChanged()
{
	GetCharacterMovement()->bOrientRotationToMovement = !isLockedOn;
}

void AJFCharacter::setLockedOnServer_Implementation(bool isLocked, AJFCharacter* TargetChar)
{
	isLockedOn = isLocked;
	LockOnCharacter = TargetChar;
	PostLockedOnChanged();
}

//====================================================================================
// Parry Functions

void AJFCharacter::Parry_Implementation()
{
	if(AbilitySystemComponent->HasMatchingGameplayTag(UJFGameInstance::DoingSomethingTag) ||
		AbilitySystemComponent->HasMatchingGameplayTag(UJFGameInstance::GAHitStunTag) ||
		AbilitySystemComponent->HasMatchingGameplayTag(UJFGameInstance::GrabbedTag)) return;

	//Cannot Move for First Few Frames of Parry
	AbilitySystemComponent->AddReplicatedLooseGameplayTag(UJFGameInstance::DoingSomethingTag);
	AbilitySystemComponent->AddReplicatedLooseGameplayTag(UJFGameInstance::CantMoveTag);
	
	AbilitySystemComponent->AddLooseGameplayTag(UJFGameInstance::DoingSomethingTag);
	AbilitySystemComponent->AddLooseGameplayTag(UJFGameInstance::CantMoveTag);

	//Parry Timer
	ParryTime = PARRY_PRE_LAG + PARRY_WINDOW + PARRY_POST_LAG;

	//Reset All
	bStartParryWindow = false;
	bEndParryWindow = false;
	bIsParrying = true;

	CallParryEvent();
	
	GEngine->AddOnScreenDebugMessage(-1,25,FColor::Red, "Pre-Parry Window Started");
}

void AJFCharacter::TickParry(float DeltaSeconds)
{
	ParryTime -= DeltaSeconds;
	if(!bIsParrying || !HasAuthority()) return;

	if(ParryTime <= (PARRY_WINDOW + PARRY_POST_LAG) && !bStartParryWindow)
	{
		//Start Parry Time
		GEngine->AddOnScreenDebugMessage(-1,25,FColor::Green, "Parry Window Started");
		AbilitySystemComponent->AddReplicatedLooseGameplayTag(UJFGameInstance::ParryTag);
		AbilitySystemComponent->AddLooseGameplayTag(UJFGameInstance::ParryTag);
		bStartParryWindow = true;
	}

	if(ParryTime <= PARRY_POST_LAG && !bEndParryWindow)
	{
		//Parry Post Lag - End Parry
		GEngine->AddOnScreenDebugMessage(-1,25,FColor::Yellow, "Parry Window Ended");
		AbilitySystemComponent->RemoveReplicatedLooseGameplayTag(UJFGameInstance::ParryTag);
		AbilitySystemComponent->RemoveLooseGameplayTag(UJFGameInstance::ParryTag);
		bEndParryWindow = true;
	}

	if(ParryTime <= 0)
	{
		//End Parry (DoingSomething)
		GEngine->AddOnScreenDebugMessage(-1,25,FColor::Red, "Parry is Over");
		AbilitySystemComponent->RemoveReplicatedLooseGameplayTag(UJFGameInstance::DoingSomethingTag);
		AbilitySystemComponent->RemoveReplicatedLooseGameplayTag(UJFGameInstance::CantMoveTag);
		AbilitySystemComponent->RemoveLooseGameplayTag(UJFGameInstance::DoingSomethingTag);
		AbilitySystemComponent->RemoveLooseGameplayTag(UJFGameInstance::CantMoveTag);
		ParryEndEvent();
		bIsParrying = false;
	}
}

void AJFCharacter::onParried_Implementation(float Damage, AJFCharacter* Character)
{
	//Only Stunned Once
	if(isStunned) return;
	
	GEngine->AddOnScreenDebugMessage(-1,25,FColor::Red,
		GetName() + " Has Been Stunned due to Parry.");
	
	//When Character Parries US -> Cannot Do Anything for Duration (Stunned)
	AbilitySystemComponent->AddReplicatedLooseGameplayTag(UJFGameInstance::CantMoveTag);
	AbilitySystemComponent->AddLooseGameplayTag(UJFGameInstance::CantMoveTag);
	
	AbilitySystemComponent->AddReplicatedLooseGameplayTag(UJFGameInstance::DoingSomethingTag);
	AbilitySystemComponent->AddLooseGameplayTag(UJFGameInstance::DoingSomethingTag);
	
	AbilitySystemComponent->AddGameplayCue(UJFGameInstance::ParryStunTag);

	StunTime = PARRY_STUN_TIME;
	isStunned = true;

	//Effects
	CallStunEvent();

	//End Ability
	AbilitySystemComponent->CancelAllAbilities();
	UE_LOG(LogTemp, Warning, TEXT("CancelAllAbilities called. Active: %d"), AbilitySystemComponent->GetActivatableAbilities().Num());
}

void AJFCharacter::TickStun(float DeltaSeconds, bool ForceEnd)
{
	if(!isStunned || !HasAuthority()) return;

	StunTime -= DeltaSeconds;

	if(StunTime <= 0 || ForceEnd)
	{
		isStunned = false;

		AbilitySystemComponent->RemoveReplicatedLooseGameplayTag(UJFGameInstance::CantMoveTag);
		AbilitySystemComponent->RemoveLooseGameplayTag(UJFGameInstance::CantMoveTag);
	
		AbilitySystemComponent->RemoveReplicatedLooseGameplayTag(UJFGameInstance::DoingSomethingTag);
		AbilitySystemComponent->RemoveLooseGameplayTag(UJFGameInstance::DoingSomethingTag);
		
		AbilitySystemComponent->RemoveGameplayCue(UJFGameInstance::ParryStunTag);
	}
}

void AJFCharacter::TickHitStun(float DeltaSeconds)
{
	if(!HasAuthority() ||
		!AbilitySystemComponent->HasMatchingGameplayTag(UJFGameInstance::GAHitStunTag))
	{
		HitStunTime = 0.f;
		return;
	}

	HitStunTime -= DeltaSeconds;

	if(HitStunTime <= 0)
	{
		AbilitySystemComponent->RemoveReplicatedLooseGameplayTag(UJFGameInstance::GAHitStunTag);
		AbilitySystemComponent->RemoveLooseGameplayTag(UJFGameInstance::GAHitStunTag);
		AbilitySystemComponent->RemoveGameplayCue(UJFGameInstance::HitStunTag);
	}
}

void AJFCharacter::CallParryEvent_Implementation()
{
	ParryAnimationEvent.Broadcast();
	LParryEyes->SetVisibility(true);
	RParryEyes->SetVisibility(true);
}

void AJFCharacter::ParryEndEvent_Implementation()
{
	LParryEyes->SetVisibility(false);
	RParryEyes->SetVisibility(false);
}

void AJFCharacter::CallStunEvent_Implementation()
{
	StunAnimationEvent.Broadcast();
}

//====================================================================================
// Sound Functions

void AJFCharacter::PlaySoundMulti_Implementation(USoundWave* Sound, bool ForcePlay)
{
	if(Sound == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("Cannot Play Sound -- Sound Wave is Invalid|Null"));
		return;
	}

	if(VoicePlayer->IsPlaying() && !ForcePlay)
	{
		UE_LOG(LogTemp, Warning, TEXT("Cannot Play Sound -- Sound is already playing."));
		return;
	}

	VoicePlayer->SetSound(Sound);
	VoicePlayer->Play();
}

void AJFCharacter::PlaySoundMultiLocation_Implementation(USoundWave* SoundWave, FVector Location, FVector2D VolumeRange, FVector2D PitchRange)
{
	const float RandVolume = FMath::FRandRange(VolumeRange.X, VolumeRange.Y);
	const float RandPitch = FMath::FRandRange(PitchRange.X, PitchRange.Y);
	
	UGameplayStatics::PlaySoundAtLocation(GetWorld(), SoundWave,
		Location, FRotator::ZeroRotator,
		RandVolume, RandPitch, 0,
		VoicePlayer->AttenuationSettings, nullptr, this, nullptr);
}

void AJFCharacter::StopSoundMulti_Implementation()
{
	VoicePlayer->Stop();
}

void AJFCharacter::PlaySoundByWave(USoundWave* Sound, bool ForcePlay)
{
	if(!HasAuthority())
	{
		UE_LOG(LogTemp, Warning, TEXT("Cannot Play Sound without Server Authority"));
		return;
	}

	PlaySoundMulti(Sound, ForcePlay);
}

void AJFCharacter::PlaySoundByType(ESoundType Sound, bool ForcePlay)
{
	switch (Sound)
	{
		case ESoundType::Grunt:
			PlaySoundByWave(GetAttackGruntSound(), ForcePlay);
			break;
		case ESoundType::LightHit:
			PlaySoundByWave(GetLightAttackSound(), ForcePlay);
			break;
		case ESoundType::HeavyHit:
			PlaySoundByWave(GetHeavyAttackSound(), ForcePlay);
			break;
	}
}

void AJFCharacter::PlaySoundByWaveAtLocation(USoundWave* Sound, FVector WorldLocation, FVector2D VolumeRange, FVector2D PitchRange)
{
	PlaySoundMultiLocation(Sound, WorldLocation, VolumeRange, PitchRange);
}

void AJFCharacter::PlaySoundByTypeAtLocation(ESoundType Sound, FVector WorldLocation, FVector2D VolumeRange, FVector2D PitchRange)
{
	switch (Sound)
	{
		case ESoundType::Grunt:
			PlaySoundByWaveAtLocation(GetAttackGruntSound(), WorldLocation, VolumeRange, PitchRange);
			break;
		case ESoundType::LightHit:
			PlaySoundByWaveAtLocation(GetLightAttackSound(), WorldLocation, VolumeRange, PitchRange);
			break;
		case ESoundType::HeavyHit:
			PlaySoundByWaveAtLocation(GetHeavyAttackSound(), WorldLocation, VolumeRange, PitchRange);
			break;
	}
}
