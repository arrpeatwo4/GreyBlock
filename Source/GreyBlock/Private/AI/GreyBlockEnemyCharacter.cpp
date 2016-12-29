// Fill out your copyright notice in the Description page of Project Settings.

#include "GreyBlock.h"
#include "AGBEnemyAIController.h"
#include "GBBotWayPoint.h"
#include "GreyBlockHeroCharacter.h"
#include "GBPlayerState.h"
#include "BaseWeapon.h"
#include "GreyBlockEnemyCharacter.h"

/* AI include */
#include "Perception/PawnSensingComponent.h"

/* Default constructor */
AGreyBlockEnemyCharacter::AGreyBlockEnemyCharacter()
{

}


AGreyBlockEnemyCharacter::AGreyBlockEnemyCharacter(const class FObjectInitializer & ObjectInitializer) : Super(ObjectInitializer)
{
	/* Assign the controller class in the bleprint extension of this class */
	PawnSensingComp = CreateDefaultSubobject<UPawnSensingComponent>(TEXT("PawnSensingComp"));
	PawnSensingComp->SetPeripheralVisionAngle(60.0f);
	PawnSensingComp->SightRadius = 2000;
	PawnSensingComp->HearingThreshold = 600;
	PawnSensingComp->LOSHearingThreshold = 1200;

	/* Ignore this channel or it will absorb the trace impacts instead of the skeletal mesh */
	GetCapsuleComponent()->SetCollisionResponseToChannel(COLLISION_WEAPON, ECR_Ignore);
	GetCapsuleComponent()->SetCapsuleHalfHeight(96.0f, false);
	GetCapsuleComponent()->SetCapsuleRadius(42.0f);

	/* These values are mathced up to the capsulecomponent above and are used to find nav paths */
	GetMovementComponent()->NavAgentProps.AgentRadius = 42;
	GetMovementComponent()->NavAgentProps.AgentHeight = 192;

	/*
	MeleeCollisionComp = CreateDefaultSubobject<UCapsuleComponent>(TEXT("MeleeCollision"));
	MeleeCollisionComp->SetRelativeLocation(FVector(45, 0, 25));
	MeleeCollisionComp->SetCapsuleHalfHeight(60);
	MeleeCollisionComp->SetCapsuleRadius(35, false);
	MeleeCollisionComp->SetCollisionResponseToAllChannels(ECR_Ignore);
	MeleeCollisionComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	MeleeCollisionComp->SetupAttachment(GetCapsuleComponent());
	*/

	/* audio stuff */

	Health = 100;
	MeleeDamage = 24.0f;
	MeleeStrikeCooldown = 1.0f;
	SprintingSpeedModifier = 3.0f;

	/* By default do not let the AI patrol we can override this value per instance */
	//BotType = EBotBehaviorType::Passive;
	BotType = EBotBehaviorType::Patrolling;
	SenseTimeOut = 2.5f;
	
}


void AGreyBlockEnemyCharacter::BeginPlay()
{
	Super::BeginPlay();

	/* This is the earliest moment we can bind our delegates to the component */
	if (PawnSensingComp)
	{
		PawnSensingComp->OnSeePawn.AddDynamic(this, &AGreyBlockEnemyCharacter::OnSeePlayer);
		PawnSensingComp->OnHearNoise.AddDynamic(this, &AGreyBlockEnemyCharacter::OnHearNoise);
	}
	if (MeleeCollisionComp)
	{
		MeleeCollisionComp->OnComponentBeginOverlap.AddDynamic(this, &AGreyBlockEnemyCharacter::OnMeleeCompBeginOverlap);
	}

	// Broadcast update audio loop stuff
	//BroadcastUpdateAudioLoop(bSensedTarget);

	/* Assign a basic name to identify the bots in the HUD */
	AGBPlayerState * PS = Cast<AGBPlayerState>(PlayerState);
	if (PS)
	{
		PS->SetPlayerName("Bot");
		PS->bIsABot = true;
	}
}


void AGreyBlockEnemyCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	/* Check if the last time we sensed a player is beyond the time out value to prevent bot from endlessly following a player */
	if (bSensedTarget && (GetWorld()->TimeSeconds - LastSeenTime) > SenseTimeOut
		&& (GetWorld()->TimeSeconds - LastHeardTime) > SenseTimeOut)
	{
		
		AAGBEnemyAIController * AIController = Cast<AAGBEnemyAIController>(GetController());
		if (AIController)
		{
			bSensedTarget = false;
						
			AIController->SetTargetEnemy(nullptr);

			/* Stop playing hunting sound */
			//BroadcastUpdateAudioLoop(false);
		}
		
	}
}


void AGreyBlockEnemyCharacter::OnSeePlayer(APawn * Pawn)
{
	if (!IsAlive())
	{
		return;
	}

	if (!bSensedTarget)
	{
		// BroadcastUpdateAudioLoop(true);
	}

	/* keep track of the time the player was last sensed in order to clear target */
	LastSeenTime = GetWorld()->GetTimeSeconds();
	bSensedTarget = true;

	
	AAGBEnemyAIController * AIController = Cast<AAGBEnemyAIController>(GetController());
	AGreyBlockHeroCharacter * SensedPawn = Cast<AGreyBlockHeroCharacter>(Pawn);
	if (AIController && SensedPawn->IsAlive())
	{
		AIController->SetTargetEnemy(SensedPawn);
	}
}


void AGreyBlockEnemyCharacter::OnHearNoise(APawn * PawnInstigator, const FVector & Location, float Volume)
{
	if (!IsAlive())
	{
		return;
	}

	if (!bSensedTarget)
	{
		//BroadcastUpdateAudioLoop(true);
	}

	bSensedTarget = true;
	LastHeardTime = GetWorld()->GetTimeSeconds();

	
	AAGBEnemyAIController * AIController = Cast<AAGBEnemyAIController>(GetController());
	if (AIController)
	{
		AIController->SetTargetEnemy(PawnInstigator);
	}
}


void AGreyBlockEnemyCharacter::PerformMeleeStrike(AActor * HitActor)
{
	if (LastMeleeAttackTime > GetWorld()->GetTimeSeconds() - MeleeStrikeCooldown)
	{
		/* Set timer to start attacking as soon as the cooldown elapses */
		if (!TimerHandle_MeleeAttack.IsValid())
		{
			//TODO: Set Time
		}

		/* Attacked before cooldown required */
		return;
	}

	if (HitActor && HitActor != this && IsAlive())
	{
		ACharacter * OtherPawn = Cast<ACharacter>(HitActor);
		if (OtherPawn)
		{
			AGBPlayerState * MyPS = Cast<AGBPlayerState>(PlayerState);
			AGBPlayerState * OtherPS = Cast<AGBPlayerState>(OtherPawn->PlayerState);

			if (MyPS && OtherPS)
			{
				
				if (MyPS->GetTeamNumber() == OtherPS->GetTeamNumber())
				{
					/* Do not attack other enemies */
					return;
				}
		

				/* Set to prevent a zombie attack multiple times in a very short time */
				LastMeleeAttackTime = GetWorld()->GetTimeSeconds();

				FPointDamageEvent DmgEvent;
				DmgEvent.DamageTypeClass = PunchDamageType;
				DmgEvent.Damage = MeleeDamage;

				HitActor->TakeDamage(DmgEvent.Damage, DmgEvent, GetController(), this);

				SimulateMeleeStrike();
			}
		}
	}
}


void AGreyBlockEnemyCharacter::SetBotType(EBotBehaviorType NewType)
{
	BotType = NewType;

	AAGBEnemyAIController * AIController = Cast<AAGBEnemyAIController>(GetController());
	if (AIController)
	{
		AIController->SetBlackboardBotType(NewType);
	}

	// audio
	// BroadcastUpdateAudioLoop(bSensedTarget);
}



bool AGreyBlockEnemyCharacter::IsSprinting() const
{
	/* Allow a zombie to sprint when he has seen a player */
	return bSensedTarget && !GetVelocity().IsZero();
}


void AGreyBlockEnemyCharacter::PlayHit(float DamageTaken, struct FDamageEvent const& DamageEvent, APawn* PawnInstigator, AActor* DamageCauser, bool bKilled)
{
	Super::PlayHit(DamageTaken, DamageEvent, PawnInstigator, DamageCauser, bKilled);
		
	ABaseWeapon * WeaponUsed = DamageCauser ? Cast<ABaseWeapon>(DamageCauser) : nullptr;

	if (!WeaponUsed)
	{
		return;
	}

	//if (bKilled && SoundDeath)
	if (bKilled)
	{
		// Play Death Animation
		if (WeaponUsed->GetWeaponType() == EWeaponType::Unarmed)
		{
			float AnimDuration;
			if (BeingAssassinatedMontage)
			{
				AnimDuration = PlayAnimMontage(BeingAssassinatedMontage);
			}
			GetWorldTimerManager().SetTimer(TimerHandle_StopAssassination, this, &AGreyBlockEnemyCharacter::StopAssassination, AnimDuration, false);
		}
		else if (WeaponUsed->GetWeaponType() == EWeaponType::Pistol)
		{
			SetRagdollPhysics();
		}
		else if (WeaponUsed->GetWeaponType() == EWeaponType::Taser)
		{
			float AnimDuration;
			if (TaserDeathMontage)
			{
				AnimDuration = PlayAnimMontage(TaserDeathMontage);
			}

			// Change the time manager to call appropriate function.
			GetWorldTimerManager().SetTimer(TimerHandle_StopAssassination, this, &AGreyBlockEnemyCharacter::StopAssassination, AnimDuration, false);
		}

		//Uncomment Audio when available
		//UGameplayStatics::SpawnSoundAttached(SoundDeath, RootComponent, NAME_None, FVector::ZeroVector, EAttachLocation::SnapToTarget, true);
	}
	//else if (SoundTakeHit)
	//{
	//	UGameplayStatics::SpawnSoundAttached(SoundTakeHit, RootComponent, NAME_None, FVector::ZeroVector, EAttachLocation::SnapToTarget, true);
	//}
}


void AGreyBlockEnemyCharacter::StopAssassination()
{
	SetRagdollPhysics();
}


void AGreyBlockEnemyCharacter::SimulateMeleeStrike()
{
	//PlayAnimMontage(MeleeAnimMontage);
	//PlayCharacterSound(SoundAttackMelee);
}


void AGreyBlockEnemyCharacter::OnMeleeCompBeginOverlap(class UPrimitiveComponent * OverlappedComponent, class AActor * OtherActor, class UPrimitiveComponent * OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult)
{
	/* Stop any running attack timers */
	TimerHandle_MeleeAttack.Invalidate();

	PerformMeleeStrike(OtherActor);

	/* Set retrigger time to recheck overlapping pawns at melee attack rate interval */
	GetWorldTimerManager().SetTimer(TimerHandle_MeleeAttack, this, &AGreyBlockEnemyCharacter::OnRetriggerMeleeStrike, MeleeStrikeCooldown, true);
}


void AGreyBlockEnemyCharacter::OnRetriggerMeleeStrike()
{
	/* Apply damage to a single random parn in range */
	TArray<AActor*> Overlaps;
	MeleeCollisionComp->GetOverlappingActors(Overlaps, ABaseCharacter::StaticClass());

	for (int32 i = 0; i < Overlaps.Num(); i++)
	{
		ABaseCharacter * OverlappingPawn = Cast<ABaseCharacter>(Overlaps[i]);
		if (OverlappingPawn)
		{
			PerformMeleeStrike(OverlappingPawn);
		}
	}

	/* No pawns in range, cancle the retrigger timer */
	if (Overlaps.Num() == 0)
	{
		TimerHandle_MeleeAttack.Invalidate();
	}
}

void AGreyBlockEnemyCharacter::BeAssassinated()
{
	
}


FVector AGreyBlockEnemyCharacter::GetForwardVec()
{
	return GetActorForwardVector();
}


FVector AGreyBlockEnemyCharacter::GetLocation()
{
	return GetActorLocation();	
}


FRotator AGreyBlockEnemyCharacter::GetRotation()
{
	return GetActorRotation();
}

