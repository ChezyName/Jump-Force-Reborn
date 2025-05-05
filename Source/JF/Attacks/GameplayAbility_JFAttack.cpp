// Copyright ChezyName. All Rights Reserved.


#include "GameplayAbility_JFAttack.h"

#include "HitboxTask.h"
#include "Components/BoxComponent.h"
#include "Components/CapsuleComponent.h"
#include "JF/JFCharacter.h"
#include "Kismet/KismetSystemLibrary.h"

UHitbox::UHitbox() : Super() {}
UHitbox::UHitbox(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer) {}

void UGameplayAbility_JFAttack::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                                const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
                                                const FGameplayEventData* TriggerEventData)
{
	HitboxTask = UHitboxTask::CreateHitboxTicker(this);
	HitboxTask->OnTick.AddDynamic(this, &UGameplayAbility_JFAttack::onTick);
	HitboxTask->ReadyForActivation();
	
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
}

void UGameplayAbility_JFAttack::onTick()
{
	for(int i = 0; i < Hitboxes.Num(); i++)
	{
		TickHitbox(Hitboxes[i]);
	}
}

void UGameplayAbility_JFAttack::TickHitbox(UHitbox* Hitbox)
{
	if(Hitbox == nullptr) return;
	//Tick this Hitbox
	if(Hitbox->bActive && Hitbox->Component != nullptr)
	{
		UKismetSystemLibrary::PrintString(GetWorld(), "Hitbox: " + FString::Printf(TEXT("%u"), Hitbox->HitboxID)
		+ " is Active", true, true, FLinearColor::Green,
			30);
		
		//Deal Damage if Targets Hit
		TArray<AActor*> ActorsOverlapping;
		Hitbox->Component->GetOverlappingActors(ActorsOverlapping, AJFCharacter::StaticClass());

		for(AActor* Actor : ActorsOverlapping)
		{
			if(Actor == nullptr) continue; //Ignore if Invalid
			
			UKismetSystemLibrary::PrintString(GetWorld(),"Actor was Hit: " +
				Actor->GetName(), true, true, FLinearColor::Yellow,
				30);
			
			if(ActorsHit.Contains(Actor)) continue; // Cannot Target Prev Targeted
			if(Actor == GetAvatarActorFromActorInfo()) continue; // Cannot Target Self

			if(AJFCharacter* Char = Cast<AJFCharacter>(Actor))
			{
				//Damage Char
				if(GetAvatarActorFromActorInfo() && Char)
				{
					UKismetSystemLibrary::PrintString(GetWorld(),
						GetAvatarActorFromActorInfo()->GetName() + " Has Hit: " +
						Char->GetName(), true, true, FLinearColor::Blue,
						30);
				}

				ActorsHit.Add(Actor);
			}
		}
	}
	else
	{
		UKismetSystemLibrary::PrintString(GetWorld(), "Hitbox: " + FString::Printf(TEXT("%u"), Hitbox->HitboxID)
		+ " is NOT Active for the following: " +
		"     Is Active: " + (Hitbox->bActive ? "true" : "false") +
		"     Is Component Valid: " + (Hitbox->Component != nullptr ? "true" : "false"),
		true, true, FLinearColor::Red,
			30);
	}
}

UHitbox* UGameplayAbility_JFAttack::CreateHitbox(TEnumAsByte<EHitboxType> Type,
                                                FVector Position, FRotator Rotation, FVector Size,
                                                UPrimitiveComponent* AttachTo, FName AttachToBoneName, float Lifetime, bool Debug)
{
	UHitbox* BuildingHitbox = NewObject<UHitbox>(this);
	BuildingHitbox->HitboxID = HitboxIDGenerator();
	BuildingHitbox->Lifetime = Lifetime;
	BuildingHitbox->Component = nullptr;
	BuildingHitbox->HitboxType = Type;
	BuildingHitbox->bDebug = Debug;
	BuildingHitbox->Owner = GetAvatarActorFromActorInfo();

	if(BuildingHitbox->Owner == nullptr)
	{
		Hitboxes.Add(BuildingHitbox);
		return BuildingHitbox;
	}

	//Create Hitbox
	if(Type == Box)
	{
		UBoxComponent* Box = NewObject<UBoxComponent>(BuildingHitbox->Owner);
		Box->RegisterComponent();
		Box->SetBoxExtent(Size);
		Box->bHiddenInGame = !Debug;
		BuildingHitbox->Component = Box;
	}
	else if(Type == Capsule)
	{
		UCapsuleComponent* Capsule = NewObject<UCapsuleComponent>(BuildingHitbox->Owner);
		Capsule->RegisterComponent();
		Capsule->SetCapsuleSize(Size.X, Size.Y, true);
		Capsule->bHiddenInGame = !Debug;
		BuildingHitbox->Component = Capsule;
	}
	
	if(AttachTo != nullptr && BuildingHitbox->Component != nullptr)
	{
		BuildingHitbox->Component->AttachToComponent(AttachTo,
			FAttachmentTransformRules::SnapToTargetNotIncludingScale, AttachToBoneName);
			
		BuildingHitbox->Component->SetRelativeLocation(Position);
		BuildingHitbox->Component->SetRelativeRotation(Rotation);
	}
	else {
		BuildingHitbox->Component->SetWorldLocation(Position);
		BuildingHitbox->Component->SetWorldRotation(Rotation);
	}

	Hitboxes.Add(BuildingHitbox);
	return BuildingHitbox;
}
