// Copyright ChezyName. All Rights Reserved.


#include "GameplayAbility_JFAttack.h"

#include "HitboxTask.h"
#include "Engine/OverlapResult.h"
#include "DrawDebugHelpers.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "JF/JFCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "Components/AudioComponent.h"
#include "JF/Game/JFGameInstance.h"
#include "Kismet/KismetSystemLibrary.h"

UHitbox::UHitbox() : Super() {}
UHitbox::UHitbox(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer) {}

UGameplayAbility_JFAttack::UGameplayAbility_JFAttack() : Super()
{
	//Default Modes
	ReplicationPolicy = EGameplayAbilityReplicationPolicy::ReplicateYes;
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	bReplicateInputDirectly = true;
	bIsCancelable = true;

	ActivationBlockedTags.AddTag(FGameplayTag::RequestGameplayTag(FName("Character.Status.DoingSomething")));
	ActivationBlockedTags.AddTag(FGameplayTag::RequestGameplayTag(FName("Character.HitStun")));
	ActivationBlockedTags.AddTag(FGameplayTag::RequestGameplayTag(FName("Character.Status.TimeStopped")));
	ActivationBlockedTags.AddTag(FGameplayTag::RequestGameplayTag(FName("GameplayCue.ParryStun")));
	ActivationBlockedTags.AddTag(FGameplayTag::RequestGameplayTag(FName("Character.Status.Grabbed")));
}

void UGameplayAbility_JFAttack::PostInitProperties()
{
	/*
	if (HasAnyFlags(RF_ClassDefaultObject))
	{
		ActivationBlockedTags.AddTag(FGameplayTag::RequestGameplayTag(FName("Character.Status.DoingSomething")));
		ActivationBlockedTags.AddTag(FGameplayTag::RequestGameplayTag(FName("Character.HitStun")));
		ActivationBlockedTags.AddTag(FGameplayTag::RequestGameplayTag(FName("Character.Status.TimeStopped")));
		ActivationBlockedTags.AddTag(FGameplayTag::RequestGameplayTag(FName("GameplayCue.ParryStun")));
		ActivationBlockedTags.AddTag(FGameplayTag::RequestGameplayTag(FName("Character.Status.Grabbed")));
	}
	*/
	
	Super::PostInitProperties();
}

void UGameplayAbility_JFAttack::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                                const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
                                                const FGameplayEventData* TriggerEventData)
{
	HitboxTask = UHitboxTask::CreateHitboxTicker(this);
	HitboxTask->OnTick.AddDynamic(this, &UGameplayAbility_JFAttack::onTick);
	HitboxTask->ReadyForActivation();
	LastTime = GetWorld()->GetTimeSeconds();
	ResetActorsHit();

	if(GI == nullptr)
	{
		GI = Cast<UJFGameInstance>(GetWorld()->GetGameInstance());
	}

	//Play 'GRUNT' Sound
	if(bPlayGruntSound)
	{
		if(AJFCharacter* Char = Cast<AJFCharacter>(ActorInfo->AvatarActor))
		{
			Char->PlaySoundByType(ESoundType::Grunt, true);
		}
	}
	
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

	const float CurrentTime = GetWorld()->GetTimeSeconds();
	DeltaTime = CurrentTime - LastTime;
	LastTime = CurrentTime;
	if(callTickEvent) OnTickEvent(DeltaTime);
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
	if(Hitbox->HitboxType == Box)
	{
		DrawDebugBox(GetWorld(), Position, Size, Rotation.Quaternion(),
		Color, Display, (1.f/60.f), 0, 2.5f);
	}
	else
	{
		DrawDebugCapsule(GetWorld(), Position, Size.Y, Size.X,  Rotation.Quaternion(),
		Color, Display, (1.f/60.f), 0, 2.5f);
	}
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
	else if(Hitbox->HitboxType == Capsule)
	{
		GetWorld()->OverlapMultiByChannel(OverlapResults, Position, Rotation.Quaternion(),
	ECC_Pawn, FCollisionShape::MakeCapsule(Size.X, Size.Y),
			QueryParams);
	}

	//Filter Hits
	for(FOverlapResult Overlap : OverlapResults)
	{
		//Only Allow if not HB Owner && JFChar
		if(Overlap.GetActor() &&
			Overlap.GetActor()->IsA(AJFCharacter::StaticClass())
			&& Overlap.GetActor() != Hitbox->Owner)
		{
			Actors.Add(Overlap.GetActor());
		}
	}
}

void UGameplayAbility_JFAttack::TickHitbox(UHitbox* Hitbox)
{
	if(Hitbox == nullptr) return;
	//Showcase if Debug
	if(GI->bDebugHitbox) DebugHitbox(Hitbox, Hitbox->bActive ? FColor::Green : FColor::Red);
	
	//Tick this Hitbox
	if(Hitbox->bActive)
	{
		//Deal Damage if Targets Hit
		TArray<AActor*> ActorsOverlapping;
		GetHitboxOverlap(Hitbox, ActorsOverlapping);

		//UKismetSystemLibrary::PrintString(GetWorld(),"Hitbox Active & Hit " + FString::SanitizeFloat(ActorsOverlapping.Num()));

		for(AActor* Actor : ActorsOverlapping)
		{
			if(Actor == nullptr) continue; //Ignore if Invalid
			
			if(ActorsHit.Contains(Actor)) continue; // Cannot Target Prev Targeted
			if(Actor == GetAvatarActorFromActorInfo()) continue; // Cannot Target Self

			if(AJFCharacter* Char = Cast<AJFCharacter>(Actor))
			{
				//Damage Char
				/* DEBUG HIT
				if(GetAvatarActorFromActorInfo() && Char)
				{
					UKismetSystemLibrary::PrintString(GetWorld(),
						GetAvatarActorFromActorInfo()->GetName() + " Has Hit: " +
						Char->GetName(), true, true, FLinearColor::Blue,
						30);
				}
				*/

				if(AJFCharacter* SelfChar = Cast<AJFCharacter>(GetAvatarActorFromActorInfo()))
				{
					Char->TakeDamage(Damage, SelfChar, false, false, Hitbox->Owner);
					if(bPlayHitSound) SelfChar->PlaySoundByTypeAtLocation(AttackSound, Hitbox->GetWorldTransform().GetLocation());
				}
				
				OnHitboxHit(Hitbox, Actor);
				ActorsHit.Add(Actor);
			}
		}
	}
}

UHitbox* UGameplayAbility_JFAttack::CreateHitbox(TEnumAsByte<EHitboxType> Type,
                                                 FVector Position, FRotator Rotation, FVector Size,
                                                 AActor* Owner,
                                                 UPrimitiveComponent* AttachTo, FName AttachToBoneName,
                                                 float Lifetime)
{
	UHitbox* BuildingHitbox = NewObject<UHitbox>(this);
	BuildingHitbox->HitboxID = HitboxIDGenerator();
	BuildingHitbox->Lifetime = Lifetime;
	BuildingHitbox->HitboxType = Type;
	
	BuildingHitbox->Owner = GetAvatarActorFromActorInfo();
	BuildingHitbox->AttachedTo = AttachTo;
	BuildingHitbox->AttachToBoneName = AttachToBoneName;
	
	BuildingHitbox->Location = Position;
	BuildingHitbox->Rotation = Rotation;
	BuildingHitbox->Size = Size;
	//Will Default to Ability Owner Upon Hit
	BuildingHitbox->Owner = Owner;

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
