// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "BaseCharacter.h"
#include "STypes.h"
#include "GreyBlockEnemyCharacter.generated.h"

/**
 * 
 */
UCLASS()
class GREYBLOCK_API AGreyBlockEnemyCharacter : public ABaseCharacter
{
	GENERATED_BODY()
	
		/* Last time the player was seen */
	float LastSeenTime;

	/* Last time the player was heard */
	float LastHeardTime;

	/* Last time we attacked something */
	float LastMeleeAttackTime;

	/* Time-out value to clear the sensed position of the player. Should be > Sense interval in PawnSense component */
	UPROPERTY(EditDefaultsOnly, Category = "AI")
	float SenseTimeOut;

	/* Resets after sense time-out to avoid unnecessary clearing of target each tick */
	bool bSensedTarget;

	UPROPERTY(VisibleAnywhere, Category = "AI")
	class UPawnSensingComponent * PawnSensingComp;

	virtual void BeginPlay() override;

	virtual void Tick(float DeltaSeconds) override;

protected:

	virtual bool IsSprinting() const override;

	/* Triggered by pawn sensing component when a pawn is spotted */
	UFUNCTION()
	void OnSeePlayer(APawn * Pawn);

	UFUNCTION()
	void OnHearNoise(APawn * PawnInstigator, const FVector & Location, float Volume);

	UPROPERTY(VisibleAnywhere, Category = "Attacking")
	UCapsuleComponent * MeleeCollisionComp;

	/* A pawn is in melle range */
	UFUNCTION()
	void OnMeleeCompBeginOverlap(class UPrimitiveComponent * OverlappedComponent, class AActor * OtherActor, class UPrimitiveComponent * OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult);

	void OnRetriggerMeleeStrike();

	/* Deal damage to the Actor that was hit by punch */
	UFUNCTION(BlueprintCallable, Category = "Attacking")
	void PerformMeleeStrike(AActor * HitActor);

	UFUNCTION(BlueprintCallable, Category = "Attacking")
	void SimulateMeleeStrike();

	UPROPERTY(EditDefaultsOnly, Category = "Attacking")
	TSubclassOf<UDamageType> PunchDamageType;

	UPROPERTY(EditDefaultsOnly, Category = "Attacking")
	float MeleeDamage;

	UPROPERTY(EditDefaultsOnly, Category = "Attacking")
	UAnimMontage * MeleeAnimMontage;

	/*
	* All of the sound bytes go here, check the survival template
	*/

	FTimerHandle TimerHandle_MeleeAttack;

	float MeleeStrikeCooldown;

	//virtual void PlayHit(float DamageTaken, struct FDamageEvent const & DamageEvent, APawn * PawnInstigator, AActor * DamageCauser, bool bKilled) override;

	virtual void PlayHit(float DamageTaken, struct FDamageEvent const& DamageEvent, APawn* PawnInstigator, AActor* DamageCauser, bool bKilled);

	FTimerHandle TimerHandle_StopAssassination;

	void StopAssassination();

public:

	AGreyBlockEnemyCharacter();

	AGreyBlockEnemyCharacter(const class FObjectInitializer & ObjectInitializer);

	UPROPERTY(BlueprintReadWrite, Category = "Attacking")
	bool bIsPunching;

	/* The bot behavior we want this bot to execute, (passive/protocol) by specifiying EditAnywhere we can edit this value per-instance */
	UPROPERTY(EditAnywhere, Category = "AI")
	EBotBehaviorType BotType;

	/* The thinking part of the brain */
	UPROPERTY(EditDefaultsOnly, Category = "AI")
	class UBehaviorTree * BehaviorTree;

	/* Change default bot type during gameplay */
	void SetBotType(EBotBehaviorType NewType);

	FVector GetForwardVec();

	FVector GetLocation();

	FRotator GetRotation();

	void BeAssassinated();
};
