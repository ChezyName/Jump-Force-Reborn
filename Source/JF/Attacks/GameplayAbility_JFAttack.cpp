// Copyright ChezyName. All Rights Reserved.


#include "GameplayAbility_JFAttack.h"

#include "HitboxTask.h"
#include "Engine/OverlapResult.h"
#include "DrawDebugHelpers.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
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

void UGameplayAbility_JFAttack::EndAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility, bool bWasCancelled)
{
	DestroyAllHitboxs();
	FlushPersistentDebugLines(GetWorld());
	
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UGameplayAbility_JFAttack::onTick()
{
	FlushPersistentDebugLines(GetWorld());
	
	for(int i = 0; i < Hitboxes.Num(); i++)
	{
		TickHitbox(Hitboxes[i]);
	}
}

void UGameplayAbility_JFAttack::DebugHitbox(UHitbox* Hitbox, FColor Color, bool Display)
{
	FVector Position = Hitbox->Location;
	FRotator Rotation = Hitbox->Rotation;
	FVector Size = Hitbox->Size;

	if (Hitbox->AttachedTo_SKEL && Hitbox->AttachToBoneName != NAME_None)
	{
		// Attached to a Skeletal Mesh Bone
		const FTransform SocketTransform = Hitbox->AttachedTo_SKEL->GetSocketTransform(Hitbox->AttachToBoneName);
		Position = SocketTransform.TransformPosition(Hitbox->Location);
		Rotation = SocketTransform.TransformRotation(Hitbox->Rotation.Quaternion()).Rotator();
	}
	else if (Hitbox->AttachedTo)
	{
		// Attached to a regular Component
		const FTransform BaseTransform = Hitbox->AttachedTo->GetComponentTransform();
		Position = BaseTransform.TransformPosition(Hitbox->Location);
		Rotation = BaseTransform.TransformRotation(Hitbox->Rotation.Quaternion()).Rotator();
	}

	//Display as Box
	DrawDebugBox(GetWorld(), Position, Size, Rotation.Quaternion(),
		Color, Display, (1.f/60.f), 0, 2.5f);
}

void UGameplayAbility_JFAttack::GetHitboxOverlap(UHitbox* Hitbox, TArray<AActor*>& Actors)
{
	Actors.Empty();
	
	FVector Position = Hitbox->Location;
	FRotator Rotation = Hitbox->Rotation;
	FVector Size = Hitbox->Size;

	TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
	ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECollisionChannel::ECC_Pawn));

	TArray<FOverlapResult> OverlapResults = {};
	
	FCollisionQueryParams QueryParams;
	TArray<AActor*> IgnoredActors = { Hitbox->Owner };
	QueryParams.AddIgnoredActors(IgnoredActors);

	if (Hitbox->AttachedTo_SKEL && Hitbox->AttachToBoneName != NAME_None)
	{
		// Attached to a Skeletal Mesh Bone
		const FTransform SocketTransform = Hitbox->AttachedTo_SKEL->GetSocketTransform(Hitbox->AttachToBoneName);
		Position = SocketTransform.TransformPosition(Hitbox->Location);
		Rotation = SocketTransform.TransformRotation(Hitbox->Rotation.Quaternion()).Rotator();
	}
	else if (Hitbox->AttachedTo)
	{
		// Attached to a regular Component
		const FTransform BaseTransform = Hitbox->AttachedTo->GetComponentTransform();
		Position = BaseTransform.TransformPosition(Hitbox->Location);
		Rotation = BaseTransform.TransformRotation(Hitbox->Rotation.Quaternion()).Rotator();
	}
	
	if (Hitbox->HitboxType == Box)
	{
		GetWorld()->OverlapMultiByChannel(OverlapResults, Position, Rotation.Quaternion(),
			ECC_Pawn, FCollisionShape::MakeBox(Size),
			QueryParams);
	}

	//Filter Hits
}

void UGameplayAbility_JFAttack::TickHitbox(UHitbox* Hitbox)
{
	if(Hitbox == nullptr) return;
	//Showcase if Debug
	if(Hitbox->bDebug) DebugHitbox(Hitbox, Hitbox->bActive ? FColor::Green : FColor::Red);
	
	//Tick this Hitbox
	if(Hitbox->bActive)
	{
		//Deal Damage if Targets Hit
		TArray<AActor*> ActorsOverlapping;
		GetHitboxOverlap(Hitbox, ActorsOverlapping);

		for(AActor* Actor : ActorsOverlapping)
		{
			if(Actor == nullptr) continue; //Ignore if Invalid
			
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
}

UHitbox* UGameplayAbility_JFAttack::CreateHitbox(TEnumAsByte<EHitboxType> Type,
                                                 FVector Position, FRotator Rotation, FVector Size,
                                                 UPrimitiveComponent* AttachTo, FName AttachToBoneName, 
                                                 float Lifetime, bool Debug)
{
	UHitbox* BuildingHitbox = NewObject<UHitbox>(this);
	BuildingHitbox->HitboxID = HitboxIDGenerator();
	BuildingHitbox->Lifetime = Lifetime;
	BuildingHitbox->HitboxType = Type;
	BuildingHitbox->bDebug = Debug;
	
	BuildingHitbox->Owner = GetAvatarActorFromActorInfo();
	BuildingHitbox->AttachedTo = AttachTo;
	BuildingHitbox->AttachToBoneName = AttachToBoneName;
	
	BuildingHitbox->Location = Position;
	BuildingHitbox->Rotation = Rotation;
	BuildingHitbox->Size = Size;

	if(USkeletalMeshComponent* Comp = Cast<USkeletalMeshComponent>(AttachTo))
	{
		BuildingHitbox->AttachedTo_SKEL = Comp;
	}

	Hitboxes.Add(BuildingHitbox);
	return BuildingHitbox;
}

void UGameplayAbility_JFAttack::DestroyHitbox(UHitbox* Hitbox)
{
	Hitboxes.Remove(Hitbox);
	DebugHitbox(Hitbox, FColor::Red, false);
	Hitbox = nullptr;
}

void UGameplayAbility_JFAttack::DestroyAllHitboxs()
{
	for(UHitbox* Hitbox : Hitboxes) DebugHitbox(Hitbox, FColor::Red, false);
	Hitboxes.Empty();
}
