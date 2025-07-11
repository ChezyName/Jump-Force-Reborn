﻿// Copyright ChezyName. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "HitboxTask.h"
#include "Abilities/GameplayAbility.h"
#include "JF/JFCharacter.h"
#include "JF/Game/JFGameInstance.h"
#include "GameplayAbility_JFAttack.generated.h"

UENUM(BlueprintType)
enum EHitboxType : uint8
{
	Box,
	Capsule,
};

UCLASS(BlueprintType)
class UHitbox : public UObject
{
	GENERATED_BODY()
public:
	UHitbox();
	UHitbox(const FObjectInitializer& ObjectInitializer);
	
	UPROPERTY(BlueprintReadOnly)
	TEnumAsByte<EHitboxType> HitboxType;
	UPROPERTY(BlueprintReadOnly)
	float Lifetime = -1;

	UPROPERTY(BlueprintReadOnly)
	uint8 HitboxID;

	UPROPERTY(BlueprintReadOnly)
	FVector Location;

	UPROPERTY(BlueprintReadOnly)
	FRotator Rotation;
	
	UPROPERTY(BlueprintReadOnly)
	FVector Size;
	
	UPROPERTY(BlueprintReadOnly)
	AActor* Owner;

	UPROPERTY(BlueprintReadOnly)
	UPrimitiveComponent* AttachedTo;

	UPROPERTY(BlueprintReadOnly)
	USkeletalMeshComponent* AttachedTo_SKEL;

	UPROPERTY(BlueprintReadOnly)
	FName AttachToBoneName;

	UPROPERTY(BlueprintReadWrite)
	bool bActive = false;

	UPROPERTY()
	AActor* HitboxOwner = nullptr;

	UFUNCTION(BlueprintPure)
	const FTransform GetWorldTransform()
	{
		FVector _Position = Location;
		FRotator _Rotation = Rotation;
		FVector _Size = Size;

		if (AttachedTo_SKEL && AttachToBoneName != NAME_None)
		{
			// Attached to a Skeletal Mesh Bone
			const FTransform SocketTransform = AttachedTo_SKEL->GetSocketTransform(AttachToBoneName);
			_Position = SocketTransform.TransformPosition(Location);
			_Rotation = SocketTransform.TransformRotation(Rotation.Quaternion()).Rotator();
		}
		else if (AttachedTo)
		{
			// Attached to a regular Component
			const FTransform BaseTransform = AttachedTo->GetComponentTransform();
			_Position = BaseTransform.TransformPosition(Location);
			_Rotation = BaseTransform.TransformRotation(Rotation.Quaternion()).Rotator();
		}

		return FTransform(_Rotation, _Position, _Size);
	}

	const FCollisionShape GetHitboxShape()
	{
		switch(HitboxType)
		{
		case Capsule:
			return FCollisionShape::MakeCapsule(Size.X, Size.Y);
		case Box:
		default:
			return FCollisionShape::MakeBox(Size);
		}
	}
};

UCLASS()
class JF_API UGameplayAbility_JFAttack : public UGameplayAbility
{
	GENERATED_BODY()
	
public:
	UGameplayAbility_JFAttack();

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Ability")
	float Damage = 150.f;

	UFUNCTION(BlueprintCallable, BlueprintPure)
	TArray<AActor*> GetActorsHit()
	{
		return ActorsHit;
	}

	UFUNCTION(BlueprintCallable, BlueprintPure)
	AActor* GetFirstActorHit()
	{
		if(ActorsHit.Num() > 0) return ActorsHit[0];
		return nullptr;
	}
	
private:
	UPROPERTY()
	TArray<UHitbox*> Hitboxes;
	UPROPERTY()
	TArray<AActor*> ActorsHit;

	int HitboxIDGenerator()
	{
		if(Hitboxes.Num() == 0) return 0;
		return Hitboxes.Last()->HitboxID + 1;
	}
protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

	UFUNCTION()
	virtual void onTick();

	bool callTickEvent = true;

	UJFGameInstance* GI;
private:
	UPROPERTY()
	UHitboxTask* HitboxTask;
	
	UFUNCTION()
	void TickHitbox(UHitbox* Hitbox);
	
	UFUNCTION()
	void DebugHitbox(UHitbox* Hitbox, FColor Color, bool Display = true);

	UFUNCTION()
	void GetHitboxOverlap(UHitbox* Hitbox, TArray<AActor*>& Actors);

	float DeltaTime;
	float LastTime;
public:
	
	UFUNCTION(BlueprintImplementableEvent, Category = Hitbox, DisplayName = "Hitbox Hit", meta=(ScriptName = "HitboxHit"))
	void OnHitboxHit(UHitbox* Hitbox, AActor* TargetHit);

	UFUNCTION(BlueprintImplementableEvent, Category = Tick, DisplayName = "On Tick", meta=(ScriptName = "OnTick"))
	void OnTickEvent(float DeltaSeconds);
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Ability|Sounds", DisplayName="Play Grunt Sound")
	bool bPlayGruntSound = true;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Ability|Sounds", DisplayName="Play Hit Sound")
	bool bPlayHitSound = true;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Ability|Sounds", DisplayName="Attack Sound Type [HIT]")
	TEnumAsByte<ESoundType> AttackSound = ESoundType::LightHit;
	
protected:
	/*
	UFUNCTION(BlueprintCallable)
	FHitbox GetHitboxByID(uint8 id);
	UFUNCTION(BlueprintCallable)
	FHitbox GetHitboxByIndex(uint8 index);
	*/

	/**
	 * Creates the Hitbox and returns the FHitbox Struct for Information regarding the Hitbox
	 * @param Type 
	 * @param Position 
	 * @param Rotation 
	 * @param Size For Box, Just Size, for Capsule, X is Radius, Y is Half Height
	 * @param AttachTo 
	 * @param AttachToBoneName 
	 * @param Lifetime 
	 * @return 
	 */
	UFUNCTION(BlueprintCallable)
	UHitbox* CreateHitbox(TEnumAsByte<EHitboxType> Type = EHitboxType::Box,
		FVector Position = FVector::ZeroVector, FRotator Rotation = FRotator::ZeroRotator,
		FVector Size = FVector(100,100,100), AActor* Owner = nullptr,
		UPrimitiveComponent* AttachTo = nullptr, FName AttachToBoneName = NAME_None,
		float Lifetime = -1);

	UFUNCTION(BlueprintCallable)
	void EnableHitbox(UHitbox* Hitbox) { if(Hitbox != nullptr) Hitbox->bActive = true; }
	UFUNCTION(BlueprintCallable)
	void DisableHitbox(UHitbox* Hitbox) { if(Hitbox != nullptr) Hitbox->bActive = false; }
	UFUNCTION(BlueprintCallable)
	void ResetActorsHit() {ActorsHit.Reset();}

	UFUNCTION(BlueprintCallable)
	void DestroyHitbox(UHitbox* Hitbox);

	UFUNCTION(BlueprintCallable)
	void DestroyAllHitboxs();

	/*
	UFUNCTION(BlueprintCallable)
	void DestroyHitbox(uint8 HitboxID);
	UFUNCTION(BlueprintCallable)
	void DestroyHitbox(FHitbox Hitbox);
	*/
};
