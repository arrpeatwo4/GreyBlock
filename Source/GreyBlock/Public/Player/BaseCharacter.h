// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Character.h"
#include "BaseCharacter.generated.h"

UCLASS()
class GREYBLOCK_API ABaseCharacter : public ACharacter
{
	GENERATED_BODY()

	/* Tracks noise data used by the pawn sensing component*/
	UPawnNoiseEmitterComponent * NoiseEmitterComp;

public:
	ABaseCharacter();

	ABaseCharacter(const class FObjectInitializer & ObjectInitializer);

	UFUNCTION(BlueprintCallable, Category = "PlayerCondition")
	float GetMaxHealth() const;

	UFUNCTION(BlueprintCallable, Category = "PlayerCondition")
	float GetHealth() const;

	UFUNCTION(BlueprintCallable, Category = "PlayerCondition")
	bool IsAlive() const;

	UFUNCTION(BlueprintCallable, Category = "Movement")
	virtual bool IsSprinting() const;

	UFUNCTION(BlueprintCallable, Category = "Movement")
	virtual void SetSprinting(bool NewSprinting);
	
	UFUNCTION(BlueprintCallable, Category = "Targeting")
	bool IsTargeting() const;	

	UFUNCTION(BlueprintCallable, Category = "Targeting")
	FRotator GetAimOffsets() const;

	UFUNCTION(BlueprintCallable, Category = "Targeting")
	bool IsFiring() const;

	UPROPERTY(EditDefaultsOnly, Category = "Inventory")
		TArray<TSubclassOf<class ABaseWeapon>> DefaultInventoryClasses;

	class ABaseWeapon * PreviousWeapon;

	float GetSprintingSpeedModifier() const;
	float GetTargetingSpeedModifier() const;

	bool CanFire() const;

	bool CanReload() const;

protected:

	UPROPERTY(EditDefaultsOnly, Category = "Movement")
	float SprintingSpeedModifier;

	UPROPERTY(EditDefaultsOnly, Category = "Movement")
	float DefaultSpeed;

	UPROPERTY(EditDefaultsOnly, Category = "Movement")
	float SprintSpeed;

	UPROPERTY(EditDefaultsOnly, Category = "Movement")
	float CrouchSpeed;

	UPROPERTY(EditDefaultsOnly, Category = "Movement")
	float TargetingSpeed;

	UPROPERTY(EditDefaultsOnly, Category = "Movement")
	bool bWantsToRun;

	void SetTargeting(bool NewTargeting);

	UPROPERTY(Transient)
	bool bIsTargeting;

	UPROPERTY(Transient)
	bool bIsFiring;

	UPROPERTY(EditDefaultsOnly, Category = "Targeting")
	float TargetingSpeedModifier;

	UPROPERTY(EditDefaultsOnly, Category = "Damage")
	UAnimMontage * BeingAssassinatedMontage;

	UPROPERTY(EditDefaultsOnly, Category = "Damage")
	UAnimMontage * TaserDeathMontage;

protected:

	UPROPERTY(EditDefaultsOnly, Category = "PlayerCondition")
	float Health;

	/* Incur damage*/
	float TakeDamage(float, struct FDamageEvent const&, class AController*, class AActor*);

	virtual bool CanDie(float KillingDamage, FDamageEvent const& DamageEvent, AController* Killer, AActor* DamageCauser) const;

	virtual bool Die(float KillingDamage, FDamageEvent const & DamageEvent, AController * Killer, AActor * DamageCauser);

	void SetRagdollPhysics();

	virtual void PlayHit(float DamageTaken, struct FDamageEvent const& DamageEvent, APawn* PawnInstigator, AActor* DamageCauser, bool bKilled);

	void DoHit(float DamageTaken, struct FDamageEvent const& DamageEvent, APawn* PawnInstigator, AActor* DamageCauser, bool bKilled);

	virtual void FellOutOfWorld(const class UDamageType& DmgType) override;

	virtual void OnDeath(float KillingDamage, FDamageEvent const& DamageEvent, APawn* PawnInstigator, AActor* DamageCauser);

	bool bIsDying;
};
