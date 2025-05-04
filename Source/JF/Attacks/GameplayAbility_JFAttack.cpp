// Copyright ChezyName. All Rights Reserved.


#include "GameplayAbility_JFAttack.h"

#include "HitboxTask.h"
#include "Components/BoxComponent.h"
#include "Components/CapsuleComponent.h"
#include "JF/JFCharacter.h"
#include "Kismet/KismetSystemLibrary.h"

void UGameplayAbility_JFAttack::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                                const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
                                                const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	UHitboxTask* HitboxTask = UHitboxTask::CreateHitboxTicker(this);
	HitboxTask->OnTick.AddDynamic(this, &UGameplayAbility_JFAttack::onTick);
	HitboxTask->ReadyForActivation();
}

void UGameplayAbility_JFAttack::onTick()
{
	for(int i = 0; i < Hitboxes.Num(); i++)
	{
		TickHitbox(Hitboxes[i]);
	}
}

void UGameplayAbility_JFAttack::TickHitbox(FHitbox Hitbox)
{
	//Tick this Hitbox
	if(Hitbox.bActive && Hitbox.Component != nullptr)
	{
		//Deal Damage if Targets Hit
		TArray<AActor*> ActorsOverlapping;
		Hitbox.Component->GetOverlappingActors(ActorsOverlapping, AJFCharacter::StaticClass());

		for(AActor* Actor : ActorsOverlapping)
		{
			if(ActorsHit.Contains(Actor)) continue; // Cannot Target Prev Targeted
			if(Actor == GetAvatarActorFromActorInfo()) continue; // Cannot Target Self

			if(AJFCharacter* Char = Cast<AJFCharacter>(Actor))
			{
				//Damage Char
				if(GetAvatarActorFromActorInfo() && Char)
				{
					UKismetSystemLibrary::PrintString(GetWorld(),
						GetAvatarActorFromActorInfo()->GetName() + " Has Hit: " +
						Char->GetName(), true, true, FLinearColor::Red,
						30);
				}

				ActorsHit.Add(Actor);
			}
		}
	}
}

FHitbox UGameplayAbility_JFAttack::CreateHitbox(TEnumAsByte<EHitboxType> Type,
                                                FVector Position, FRotator Rotation, FVector Size,
                                                UPrimitiveComponent* AttachTo, FName AttachToBoneName, float Lifetime, bool Debug)
{
	FHitbox BuildingHitbox;
	BuildingHitbox.HitboxID = HitboxIDGenerator();
	BuildingHitbox.Lifetime = Lifetime;
	BuildingHitbox.Component = nullptr;
	BuildingHitbox.HitboxType = Type;
	BuildingHitbox.bDebug = Debug;
	BuildingHitbox.Owner = GetAvatarActorFromActorInfo();

	if(BuildingHitbox.Owner == nullptr) return BuildingHitbox;

	//Create Hitbox
	if(Type == Box)
	{
		UBoxComponent* Box = NewObject<UBoxComponent>(BuildingHitbox.Owner);
		Box->RegisterComponent();
		Box->SetBoxExtent(Size);
		Box->bHiddenInGame = !Debug;
		BuildingHitbox.Component = Box;
	}
	else if(Type == Capsule)
	{
		UCapsuleComponent* Capsule = NewObject<UCapsuleComponent>(BuildingHitbox.Owner);
		Capsule->RegisterComponent();
		Capsule->SetCapsuleSize(Size.X, Size.Y, true);
		Capsule->bHiddenInGame = !Debug;
		BuildingHitbox.Component = Capsule;
	}
	
	if(AttachTo != nullptr && BuildingHitbox.Component != nullptr)
	{
		BuildingHitbox.Component->AttachToComponent(AttachTo,
			FAttachmentTransformRules::SnapToTargetNotIncludingScale, AttachToBoneName);
			
		BuildingHitbox.Component->SetRelativeLocation(Position);
		BuildingHitbox.Component->SetRelativeRotation(Rotation);
	}
	else {
		BuildingHitbox.Component->SetWorldLocation(Position);
		BuildingHitbox.Component->SetWorldRotation(Rotation);
	}

	return BuildingHitbox;
}
