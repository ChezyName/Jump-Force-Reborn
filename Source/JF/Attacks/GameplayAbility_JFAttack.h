// Copyright ChezyName. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "HitboxTask.h"
#include "Abilities/GameplayAbility.h"
#include "GameplayAbility_JFAttack.generated.h"

UENUM(BlueprintType)
enum EHitboxType : uint8
{
	Box,
	Capsule,
};

USTRUCT(BlueprintType)
struct FHitbox
{
	GENERATED_BODY()
public:
	UPROPERTY(BlueprintReadOnly)
	TEnumAsByte<EHitboxType> HitboxType;
	UPROPERTY(BlueprintReadOnly)
	float Lifetime = -1;

	UPROPERTY(BlueprintReadOnly)
	uint8 HitboxID;

	UPROPERTY(BlueprintReadOnly)
	UPrimitiveComponent* Component = nullptr;
	
	UPROPERTY(BlueprintReadOnly)
	AActor* Owner;

	UPROPERTY(BlueprintReadWrite)
	bool bActive = false;

	UPROPERTY(BlueprintReadWrite)
	bool bDebug = false;
};

UCLASS()
class JF_API UGameplayAbility_JFAttack : public UGameplayAbility
{
	GENERATED_BODY()
private:
	UPROPERTY()
	TArray<FHitbox> Hitboxes;
	UPROPERTY()
	TArray<AActor*> ActorsHit;

	int HitboxIDGenerator()
	{
		if(Hitboxes.Num() == 0) return 0;
		return Hitboxes.Last().HitboxID + 1;
	}

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	UPROPERTY()
	UHitboxTask* HitboxTask;

	UFUNCTION()
	void onTick();
	UFUNCTION()
	void TickHitbox(FHitbox Hitbox);
	
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
	FHitbox CreateHitbox(TEnumAsByte<EHitboxType> Type = EHitboxType::Box,
		FVector Position = FVector::ZeroVector, FRotator Rotation = FRotator::ZeroRotator,
		FVector Size = FVector(100,100,100),
		UPrimitiveComponent* AttachTo = nullptr, FName AttachToBoneName = NAME_None,
		float Lifetime = -1, bool Debug = false);

	UFUNCTION(BlueprintCallable)
	void EnableHitbox(FHitbox Hitbox) { Hitbox.bActive = true; }
	UFUNCTION(BlueprintCallable)
	void DisableHitbox(FHitbox Hitbox) { Hitbox.bActive = false; }
	UFUNCTION(BlueprintCallable)
	void ResetActorsHit() {ActorsHit.Reset();}

	/*
	UFUNCTION(BlueprintCallable)
	void DestroyHitbox(uint8 HitboxID);
	UFUNCTION(BlueprintCallable)
	void DestroyHitbox(FHitbox Hitbox);
	*/
};
