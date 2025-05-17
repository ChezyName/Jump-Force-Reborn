// Copyright Epic Games, Inc. All Rights Reserved.

#include "JFCharacter.h"
#include "Engine/LocalPlayer.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
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

DEFINE_LOG_CATEGORY(LogTemplateCharacter);

/** TODO
 * Combo-Combo System
 * Light Combo & Heavy Combo Share Combo Number & Combo Time
 * For Example Light -> Heavy -> Light will select the 2nd combo value for Heavy
 * God of War (2018 / Ragnorok) Style System
**/

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
	CameraBoom->TargetArmLength = 350.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller
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

	MeterChargeFX = CreateDefaultSubobject<UNiagaraComponent>(TEXT("MeterChargeFX"));
	MeterChargeFX->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale);
	MeterChargeFX->SetAutoActivate(false);

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

	// Load Niagara System asset - Meter Charge FX
	static ConstructorHelpers::FObjectFinder<UNiagaraSystem>
		MeterChargeFXFinder(TEXT("NiagaraSystem'/Game/Characters/_Core/Meter/MeterCharge.MeterCharge'"));
	if (MeterChargeFX && MeterChargeFXFinder.Succeeded())
	{
		MeterChargeFX->SetAsset(MeterChargeFXFinder.Object.Get());
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

		//Light Attack
		EnhancedInputComponent->BindAction(LightAttackAction, ETriggerEvent::Triggered,
			this, &AJFCharacter::LightAttack, true);

		//Heavy Attack
		EnhancedInputComponent->BindAction(HeavyAttackAction, ETriggerEvent::Triggered,
			this, &AJFCharacter::HeavyAttack, true);

		//Meter Charge
		EnhancedInputComponent->BindAction(MeterChargeAction, ETriggerEvent::Triggered, this, &AJFCharacter::SetMeter, true);
		EnhancedInputComponent->BindAction(MeterChargeAction, ETriggerEvent::Completed, this, &AJFCharacter::SetMeter, false);
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
		&& !AbilitySystemComponent->HasMatchingGameplayTag(DoingSomethingTag))
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	
		// get right vector 
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		// add movement 
		AddMovementInput(ForwardDirection, MovementVector.Y);
		AddMovementInput(RightDirection, MovementVector.X);
	}
}

void AJFCharacter::Look(const FInputActionValue& Value)
{
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

void AJFCharacter::BeginPlay()
{
	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->InitAbilityActorInfo(this, this);
	}

	//InitAbilities();

	//Init Base Attributes
	if(HasAuthority())
	{
		SetNumericAttribute(UJFAttributeSet::GetHealthAttribute(), MaxHealth);
		SetNumericAttribute(UJFAttributeSet::GetMaxHealthAttribute(), MaxHealth);

		SetNumericAttribute(UJFAttributeSet::GetMovementSpeedAttribute(), MovementSpeed);
	}
	
	Super::BeginPlay();
}

void AJFCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	if(IsLocallyControlled()) TickQueuedAttack(DeltaSeconds);

	//VFX & Cant Move During Meter Charge
	MeterChargeFX->SetActive(isChargingMeter());
	GetMovementComponent()->SetActive(!isChargingMeter());

	if(HasAuthority())
	{
		TickMeter(DeltaSeconds);
		
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
	}
}

void AJFCharacter::SetMeter_Implementation(bool isActive)
{
	isTryingMeterCharge = isActive;
}

bool AJFCharacter::isChargingMeter()
{
	if(!GetAbilitySystemComponent()) return false;
	
	return !AbilitySystemComponent->HasMatchingGameplayTag(DoingSomethingTag)
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