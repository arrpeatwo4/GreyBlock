// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Actor.h"
#include "GreyBlockHeroCharacter.h"
#include "STypes.h"
#include "BaseWeapon.generated.h"

UENUM()
enum class EWeaponState
{
	Idle,
	Firing,
	Equipping,
	Melee,
	Reloading
};

UENUM(BlueprintType)
enum class EWeaponType : uint8
{
	Pistol	UMETA(DisplayName= "Pistol"),
	Taser	UMETA(DisplayName = "Taser"),
	Knife	UMETA(DisplayName = "Knife"),
	Club	UMETA(DisplayName = "Club"),
	Unarmed UMETA(DisplayName = "Unarmed")
};

UCLASS(ABSTRACT, Blueprintable)
class GREYBLOCK_API ABaseWeapon : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ABaseWeapon();

	virtual void PostInitializeComponents() override;

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	float GetEquipStartedTime() const;

	float GetEquipDuration() const;

	/** last time when this weapon was switched to */
	float EquipStartedTime;

	/** how much time weapon needs to be equipped */
	float EquipDuration;

	bool bIsEquipped;

	bool bPendingEquip;

	FTimerHandle TimerHandle_HandleFiring;

	FTimerHandle EquipFinishedTimerHandle;

	UPROPERTY(EditDefaultsOnly)
	float ShotsPerMinute;

protected:

	ABaseWeapon(const FObjectInitializer& ObjectInitializer);

	/* The character socket to store this item at. */
	EInventorySlot StorageSlot;

	/** pawn owner */
	UPROPERTY(Transient)
	class AGreyBlockHeroCharacter * MyPawn;

	/** weapon mesh: 3rd person view */
	UPROPERTY(VisibleDefaultsOnly, Category = Mesh)
	USkeletalMeshComponent* Mesh;

	//UFUNCTION()
	//void OnRep_MyPawn();

	/** detaches weapon mesh from pawn */
	void DetachMeshFromPawn();

	virtual void OnEquipFinished();

	bool IsEquipped() const;

	bool IsAttachedToPawn() const;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "WeaponType")
	EWeaponType WeaponType;	

public:

	/** get weapon mesh (needs pawn owner to determine variant) */
	UFUNCTION(BlueprintCallable, Category = "Game|Weapon")
	USkeletalMeshComponent* GetWeaponMesh() const;

	virtual void OnUnEquip();

	void OnEquip(bool bPlayAnimation);

	/* Set the weapon's owning pawn */
	void SetOwningPawn(AGreyBlockHeroCharacter* NewOwner);

	/* Get pawn owner */
	UFUNCTION(BlueprintCallable, Category = "Game|Weapon")
	class ABaseCharacter* GetPawnOwner() const;

	virtual void OnEnterInventory(AGreyBlockHeroCharacter* NewOwner);

	virtual void OnLeaveInventory();

	FORCEINLINE EInventorySlot GetStorageSlot()
	{
		return StorageSlot;
	}

	FORCEINLINE EWeaponType GetWeaponType() { return WeaponType; };
	
	/* The class to spawn in the level when dropped */
	//UPROPERTY(EditDefaultsOnly, Category = "Game|Weapon")
	//TSubclassOf<class ASWeaponPickup> WeaponPickupClass;

	/************************************************************************/
	/* Fire & Damage Handling                                               */
	/************************************************************************/

public:

	void StartFire();

	void StopFire();	

	EWeaponState GetCurrentState() const;

	/* You can assign default values to function parameters, these are then optional to specify/override when calling the function. */
	void AttachMeshToPawn(EInventorySlot Slot = EInventorySlot::Hands);

protected:

	void StartMelee(FVector, FRotator, FVector);

	void StopMelee();

	bool CanFire() const;

	FVector GetAdjustedAim() const;

	FVector GetCameraDamageStartLocation(const FVector& AimDir) const;

	FHitResult WeaponTrace(const FVector& TraceFrom, const FVector& TraceTo) const;

	/* With PURE_VIRTUAL we skip implementing the function in SWeapon.cpp and can do this in SWeaponInstant.cpp / SFlashlight.cpp instead */
	virtual void FireWeapon() PURE_VIRTUAL(ASWeapon::FireWeapon, );

private:

	void SetWeaponState(EWeaponState NewState);

	void DetermineWeaponState();

	virtual void HandleFiring();

	void OnBurstStarted();

	void OnBurstFinished();

	bool bWantsToFire;

	bool bPlayingMeleeMontage;

	EWeaponState CurrentState;

	bool bRefiring;

	float LastFireTime;

	/* Time between shots for repeating fire */
	float TimeBetweenShots;

	/************************************************************************/
	/* Simulation & FX                                                      */
	/************************************************************************/

private:

	//UFUNCTION()
	//	void OnRep_BurstCounter();

	UPROPERTY(EditDefaultsOnly, Category = "Sounds")
		USoundCue* FireSound;

	UPROPERTY(EditDefaultsOnly, Category = "Sounds")
		USoundCue* EquipSound;

	UPROPERTY(EditDefaultsOnly)
		UParticleSystem* MuzzleFX;

	UPROPERTY(EditDefaultsOnly)
		UAnimMontage* EquipAnim;

	UPROPERTY(EditDefaultsOnly)
		UAnimMontage* FireAnim;

	UPROPERTY(EditDefaultsOnly)
	UAnimMontage* MeleeMontage;

	UPROPERTY(Transient)
		UParticleSystemComponent* MuzzlePSC;

	UPROPERTY(EditDefaultsOnly)
		FName MuzzleAttachPoint;

	bool bPlayingFireAnim;

	UPROPERTY(Transient)
		int32 BurstCounter;

protected:

	virtual void SimulateWeaponFire();

	virtual void StopSimulatingWeaponFire();

	FVector GetMuzzleLocation() const;

	FVector GetMuzzleDirection() const;

	UAudioComponent* PlayWeaponSound(USoundCue* SoundToPlay);

	float PlayWeaponAnimation(UAnimMontage* Animation, float InPlayRate = 1.f, FName StartSectionName = NAME_None);

	void StopWeaponAnimation(UAnimMontage* Animation);

	/************************************************************************/
	/* Ammo & Reloading                                                     */
	/************************************************************************/

private:

	UPROPERTY(EditDefaultsOnly, Category = "Sounds")
		USoundCue* OutOfAmmoSound;

	FTimerHandle TimerHandle_ReloadWeapon;

	FTimerHandle TimerHandle_StopReload;

	FTimerHandle TimerHandle_Melee;

	FTimerHandle TimerHandle_StopMelee;

protected:

	/* Time to assign on reload when no animation is found */
	UPROPERTY(EditDefaultsOnly, Category = "Animation")
	float NoAnimReloadDuration;

	/* Time to assign on equip when no animation is found */
	UPROPERTY(EditDefaultsOnly, Category = "Animation")
	float NoEquipAnimDuration;

	/* Time to assign on equip when no animation is found */
	UPROPERTY(EditDefaultsOnly, Category = "Animation")
	float NoMeleeAnimDuration;

	UPROPERTY(Transient)
	bool bPendingReload;

	void UseAmmo();

	UPROPERTY(Transient)
		int32 CurrentAmmo;

	UPROPERTY(Transient)
		int32 CurrentAmmoInClip;

	/* Weapon ammo on spawn */
	UPROPERTY(EditDefaultsOnly)
		int32 StartAmmo;

	UPROPERTY(EditDefaultsOnly)
		int32 MaxAmmo;

	UPROPERTY(EditDefaultsOnly)
		int32 MaxAmmoPerClip;

	UPROPERTY(EditDefaultsOnly, Category = "Sounds")
		USoundCue* ReloadSound;

	UPROPERTY(EditDefaultsOnly, Category = "Animation")
		UAnimMontage* ReloadAnim;

	virtual void ReloadWeapon();

	/* Is weapon and character currently capable of starting a reload */
	bool CanReload();

	//UFUNCTION()
	//void OnRep_Reload();

public:

	virtual void StartReload(bool bFromReplication = false);

	virtual void StopSimulateReload();

	/* Give ammo to weapon and return the amount that was not 'consumed' beyond the max count */
	int32 GiveAmmo(int32 AddAmount);

	/* Set a new total amount of ammo of weapon */
	void SetAmmoCount(int32 NewTotalAmount);

	UFUNCTION(BlueprintCallable, Category = "Ammo")
		int32 GetCurrentAmmo() const;

	UFUNCTION(BlueprintCallable, Category = "Ammo")
		int32 GetCurrentAmmoInClip() const;

	UFUNCTION(BlueprintCallable, Category = "Ammo")
		int32 GetMaxAmmoPerClip() const;

	UFUNCTION(BlueprintCallable, Category = "Ammo")
		int32 GetMaxAmmo() const;	
};
