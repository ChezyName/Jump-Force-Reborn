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

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f); // ...at this rotation rate

	// Note: For faster iteration times these variables, and many more, can be tweaked in the Character Blueprint
	// instead of recompiling to adjust them
	GetCharacterMovement()->JumpZVelocity = 700.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;

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

	//Load Dash Ability
	static ConstructorHelpers::FObjectFinder<UAbilityData>
		DataAssetRef(TEXT("/Script/JF.AbilityData'/Game/Characters/_Core/_Dash/DashAbilityData.DashAbilityData'"));
	if (DataAssetRef.Succeeded())
	{
		DashAbility = DataAssetRef.Object;
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
		UKismetSystemLibrary::PrintString(GetWorld(), "Input System [HANDSHAKE] Ability System @ SetupPlayerInputComp",
			true,true,FLinearColor::Green,30);
		AbilitySystemComponent->SetInputComponent(EnhancedInputComponent);
		
		InitAbilities();
		
		// Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AJFCharacter::Move);

		// Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AJFCharacter::Look);
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

void AJFCharacter::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();
	UpdatePlayerMovementVector(MovementVector);

	if (Controller != nullptr)
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
}

void AJFCharacter::InitAbilities()
{
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(InputComponent))
	{
		UKismetSystemLibrary::PrintString(GetWorld(), "Input System [HANDSHAKE] Ability System",
			true,true,FLinearColor::Green,30);
		AbilitySystemComponent->SetInputComponent(EnhancedInputComponent);
	}
	
	//Init Dash Ability (DEFAULT ABILITY)

	if(DashAbility != nullptr && DashAbility->Ability != nullptr)
	{
		UKismetSystemLibrary::PrintString(GetWorld(), "Initing Ability: " + DashAbility->AbilityName + " on Key [" +
			DashAbility->AbilityAction->GetName() + "] for " + GetName(), true,true,FLinearColor::Green,30);
		
		FGameplayAbilitySpecHandle Handle = AbilitySystemComponent->GiveAbility(DashAbility->Ability);
		AbilitySystemComponent->SetInputBinding(DashAbility->AbilityAction, Handle);
	}
	
	//Init All Other Abilities
	for(int i = 0; i < CharacterAbilities.Num(); i++)
	{
		UAbilityData* Ability = CharacterAbilities[i];
		if(Ability == nullptr || Ability->Ability == nullptr) continue;
		UKismetSystemLibrary::PrintString(GetWorld(), "Initing Ability: " + Ability->AbilityName + " on Key [" +
			Ability->AbilityAction->GetName() + "] for " + GetName(), true,true,FLinearColor::Green,30);
		
		FGameplayAbilitySpecHandle Handle = AbilitySystemComponent->GiveAbility(Ability->Ability);
		AbilitySystemComponent->SetInputBinding(Ability->AbilityAction, Handle);
	}
	
	UKismetSystemLibrary::PrintString(GetWorld(), "==================================================================",
		true,true,FLinearColor::Green,30);
	
	//UKismetSystemLibrary::PrintString(GetWorld(), "Initing Abilities | COMPLETE", true,true,FLinearColor::Red,30);
}

void AJFCharacter::BeginPlay()
{
	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->InitAbilityActorInfo(this, this);
	}
	
	Super::BeginPlay();
}
