// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "BaseCharacter.h"
#include "STypes.h"
#include "GreyBlockHeroCharacter.generated.h"

/**
 * 
 */
UCLASS()
class GREYBLOCK_API AGreyBlockHeroCharacter : public ABaseCharacter
{
	GENERATED_BODY()

public:

	AGreyBlockHeroCharacter();
	
	AGreyBlockHeroCharacter(const FObjectInitializer & ObjectInitializer);

	virtual void BeginPlay() override;

	virtual void Tick(float DeltaSeconds) override;

	virtual void SetupPlayerInputComponent(class UInputComponent * PlayerInputComponent) override;

	void StopAllAnimMontages();

	float LastNoiseLoudness;

	float LastMakeNoiseTime;

	void OverrideCharacterPosition(FVector, FRotator, FVector);

	void UnsetMeleeCollision();

private:

	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, Category = "Camera")
	class USpringArmComponent* CameraBoomComp;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, Category = "Camera")
	class UCameraComponent* CameraComp;

public:

	UFUNCTION(BlueprintCallable, Category = "AI")
	float GetLastNoiseLoudness();

	UFUNCTION(BlueprintCallable, Category = "AI")
	float GetLastMakeNoiseTime();

	FORCEINLINE UCameraComponent * GetCameraComponent()
	{
		return CameraComp;
	}

	UFUNCTION(BlueprintCallable, Category = "AI")
	void MakePawnNoise(float Loudness);

	/** Base turn rate, in deg/sec. Other scaling may affect final turn rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera)
	float BaseTurnRate;

	/** Base look up/down rate, in deg/sec. Other scaling may affect final rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera)
	float BaseLookUpRate;

	/**************************************************************/
	/* Movement                                                   */
	/**************************************************************/

	void MoveForward(float Value);

	void MoveRight(float Value);

	/** Handle the crouch input request */
	void EnterCrouch();	

	/** Handles Sprint input requests */
	void OnSprint();
	void OnStopSprinting();
	virtual void SetSprinting(bool NewSprinting) override;

	/**************************************************************/
	/* Targeting                                                  */
	/**************************************************************/

	/** Handles Aim input requests */
	UFUNCTION(BlueprintCallable, Category = "Targeting")
	bool GetIsTargeting() const;
	
	void AimPistol();
	void StopAimingPistol();

	/**************************************************************/
	/* Interaction                                                */
	/**************************************************************/

	UPROPERTY(EditDefaultsOnly, Category = "ObjectInteraction")
	float MaxUseDistance;
	
	void Interact();
	void Lure();

	class AUsableActor * GetUsableInView();
	
	bool bHasNewFocus;

	/**************************************************************/
	/* Camera Control                                             */
	/**************************************************************/
	void SetCameraControl(bool);

protected:

	/**
	* Called via input to turn at a given rate.
	* @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	*/
	void TurnAtRate(float Rate);

	/**
	* Called via input to turn look up/down at a given rate.
	* @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	*/
	void LookUpAtRate(float Rate);

	/**************************************************/
	/* Weapons & Inventory							  */
	/**************************************************/
private:
	/* Attach point for active weapons/item in hands */
	UPROPERTY(EditDefaultsOnly, Category = "Sockets")
	FName WeaponAttachPoint;

	/* Attach point for items carried on the belt */
	UPROPERTY(EditDefaultsOnly, Category = "Sockets")
	FName PelvisAttachPoint;

	/* Attach point for items on the back */
	UPROPERTY(EditDefaultsOnly, Category = "Sockets")
	FName SpineAttachPoint;

	UPROPERTY(EditDefaultsOnly, Category = "Inventory")
	float DropWeaponMaxDistance;

	bool bWantsToFire;

	/** Mapped to input */
	void OnReload();

	/** Mapped to input */
	void OnAttack();

	/** Mapped to input */
	void OnEndAttack();

	/** Mapped to input */
	void OnNextWeapon();

	/** Mapped to input */
	void OnPreviousWeapon();

	void StartWeaponFire();

	void StopWeaponFire();


public:
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	ABaseWeapon * GetCurrentWeapon() const;

	/* Check if weapon slot available */
	bool WeaponSlotAvailable(EInventorySlot CheckSlot);

	/* Check if pawn can fire weapon */
	bool CanFire() const;

	bool CanReload() const;

	UFUNCTION(BlueprintCallable, Category = "Weapon")
	bool IsFiring() const;

	UFUNCTION(BlueprintCallable, Category = "Weapon")
	bool IsMelee() const;

	/* Return socket name for attachments */
	FName GetInventoryAttachPoint(EInventorySlot Slot) const;

	UPROPERTY(Transient)
	TArray<ABaseWeapon*> Inventory;

	// RJP Remove
	UPROPERTY(EditDefaultsOnly, Category = "Player")
	TArray<TSubclassOf<class ABaseWeapon>> DefaultInventory;

	void SetCurrentWeapon(class ABaseWeapon * newWeapon, class ABaseWeapon * LastWeapon = nullptr);

	void EquipWeapon(ABaseWeapon * Weapon);

	/* OnRep functions can use a parameter to hold the previous value of the variable. Very useful when you need to handle UnEquip etc. */
	//UFUNCTION()
	//void OnRep_CurrentWeapon(ABaseWeapon * LastWeapon);

	void AddWeapon(class ABaseWeapon * Weapon);

	void RemoveWeapon(class ASWeapon * Weapon, bool bDestroy);

	UPROPERTY(Transient)
	class ABaseWeapon * CurrentWeapon;

	class ABaseWeapon * PreviousWeapon;

	UFUNCTION(BlueprintCallable, Category = "Animation")
	void SwapToNewWeaponMesh();

private:
	bool bIsCrouching, bIsSprinting, bIsAttacking, bIsDead;
	float CurrentHealth;
	int idx_CurrentWeapon;
};
