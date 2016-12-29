// Fill out your copyright notice in the Description page of Project Settings.

#include "GreyBlock.h"
#include "GBImpactEffect.h"
#include "GBDamageType.h"
#include "GreyBlockEnemyCharacter.h"
#include "WeaponInstant.h"


AWeaponInstant::AWeaponInstant(const class FObjectInitializer& PCIP)
	: Super(PCIP)
{
	HitDamage = 26;
	WeaponRange = 10;

	AllowedViewDotHitDir = -1.0f;
	ClientSideHitLeeway = 200.0f;
	MinimumProjectileSpawnDistance = 800;
	TracerRoundInterval = 3;
}


void AWeaponInstant::FireWeapon()
{
	const FVector AimDir = GetAdjustedAim();
	const FVector CameraPos = GetCameraDamageStartLocation(AimDir);
	const FVector EndPos = CameraPos + (AimDir * WeaponRange);

	/* Check for impact by tracing from the camera position */
	FHitResult Impact = WeaponTrace(CameraPos, EndPos);

	const FVector MuzzleOrigin = GetMuzzleLocation();
	
	FVector AdjustedAimDir = AimDir;

	if ((WeaponType == EWeaponType::Pistol) || (WeaponType == EWeaponType::Taser))
	{
		if (Impact.bBlockingHit)
		{
			/* Adjust the shoot direction to hit at the crosshair. */
			// Need to set the crosshair up, otherwise, this always points at the middle
			// of the world
			AdjustedAimDir = (Impact.ImpactPoint - MuzzleOrigin).GetSafeNormal();

			/* Re-trace with the new aim direction coming out of the weapon muzzle */
			Impact = WeaponTrace(MuzzleOrigin, MuzzleOrigin + (AdjustedAimDir * WeaponRange));
			
			DrawDebugLine(GetWorld(),
				MuzzleOrigin,
				MuzzleOrigin + (AdjustedAimDir * WeaponRange),
				FColor(0, 255, 0),
				false, 1.0, 0, 1.0f);
			
		}
		else
		{
			/* Use the maximum distance as the adjust direction */
			Impact.ImpactPoint = FVector_NetQuantize(EndPos);		
		}
	}

	/* Use the maximum distance as the adjust direction */
	Impact.ImpactPoint = FVector_NetQuantize(EndPos);

	ProcessInstantHit(Impact, MuzzleOrigin, AdjustedAimDir);
}


/*
*	A little cluttered here, due to the nature of the assassin animation, it has to check
*	if it hits before firing.
*/
void AWeaponInstant::ProcessInstantHit(const FHitResult& Impact, const FVector& Origin, const FVector& ShootDir)
{
	// Handle damage
	if (ShouldDealDamage(Impact.GetActor()))
	{
		if (WeaponType == EWeaponType::Unarmed)
		{
			AGreyBlockEnemyCharacter * EnemyCharacter = Impact.GetActor() ? Cast<AGreyBlockEnemyCharacter>(Impact.GetActor()) : nullptr;
			if (EnemyCharacter)
			{
				StartMelee(EnemyCharacter->GetLocation(), EnemyCharacter->GetRotation(), EnemyCharacter->GetForwardVec());
			}			
		}

		DealDamage(Impact, ShootDir);
	}

	// Play FX
	SimulateInstantHit(Impact.ImpactPoint);
}


bool AWeaponInstant::ShouldDealDamage(AActor* TestActor) const
{
	// If the actor is an enemy character proceed with damaging
	AGreyBlockEnemyCharacter * EnemyCharacter = TestActor ? Cast<AGreyBlockEnemyCharacter>(TestActor) : nullptr;

	if (EnemyCharacter)
	{
		return true;	
	}

	return false;
}


void AWeaponInstant::DealDamage(const FHitResult& Impact, const FVector& ShootDir)
{
	float ActualHitDamage = HitDamage;

	/* Handle special damage location on the zombie body (types are setup in the Physics Asset of the zombie */	
	//UGBDamageType* DmgType = Cast<UGBDamageType>(DamageType->GetDefaultObject());
	//UPhysicalMaterial * PhysMat = Impact.PhysMaterial.Get();
	//if (PhysMat && DmgType)
	//{
	//	if (PhysMat->SurfaceType == SURFACE_ENEMYHEAD)
	//	{
	//		ActualHitDamage *= DmgType->GetHeadDamageModifier();
	//	}
	//	else if (PhysMat->SurfaceType == SURFACE_ENEMYLIMB)
	//	{
	//		ActualHitDamage *= DmgType->GetLimbDamageModifier();
	//	}
	//}
		
	FPointDamageEvent PointDmg;
	PointDmg.DamageTypeClass = DamageType;
	PointDmg.HitInfo = Impact;
	PointDmg.ShotDirection = ShootDir;
	PointDmg.Damage = ActualHitDamage;

	Impact.GetActor()->TakeDamage(PointDmg.Damage, PointDmg, MyPawn->Controller, this);
}


void AWeaponInstant::SimulateInstantHit(const FVector& ImpactPoint)
{
	const FVector MuzzleOrigin = GetMuzzleLocation();

	/* Adjust direction based on desired crosshair impact point and muzzle location */
	const FVector AimDir = (ImpactPoint - MuzzleOrigin).GetSafeNormal();

	const FVector EndTrace = MuzzleOrigin + (AimDir * WeaponRange);
	const FHitResult Impact = WeaponTrace(MuzzleOrigin, EndTrace);

	if (Impact.bBlockingHit)
	{
		SpawnImpactEffects(Impact);
		SpawnTrailEffects(Impact.ImpactPoint);
	}
	else
	{
		SpawnTrailEffects(EndTrace);
	}
}


void AWeaponInstant::SpawnImpactEffects(const FHitResult& Impact)
{
	if (ImpactTemplate && Impact.bBlockingHit)
	{
		// TODO: Possible re-trace to get hit component that is lost during replication.

		/* This function prepares an actor to spawn, but requires another call to finish the actual spawn progress. This allows manipulation of properties before entering into the level */
		AGBImpactEffect* EffectActor = GetWorld()->SpawnActorDeferred<AGBImpactEffect>(ImpactTemplate, FTransform(Impact.ImpactPoint.Rotation(), Impact.ImpactPoint));
		if (EffectActor)
		{
			EffectActor->SurfaceHit = Impact;
			UGameplayStatics::FinishSpawningActor(EffectActor, FTransform(Impact.ImpactNormal.Rotation(), Impact.ImpactPoint));
		}
	}

}


void AWeaponInstant::SpawnTrailEffects(const FVector& EndPoint)
{
	// Keep local count for effects
	BulletsShotCount++;

	const FVector Origin = GetMuzzleLocation();
	FVector ShootDir = EndPoint - Origin;

	// Only spawn if a minimum distance is satisfied.
	if (ShootDir.Size() < MinimumProjectileSpawnDistance)
	{
		return;
	}

	if (BulletsShotCount % TracerRoundInterval == 0)
	{
		if (TracerFX)
		{
			ShootDir.Normalize();
			UGameplayStatics::SpawnEmitterAtLocation(this, TracerFX, Origin, ShootDir.Rotation());
		}
	}
	else
	{
		// Only create trails FX by other players.
		ABaseCharacter* OwningPawn = GetPawnOwner();
		if (OwningPawn && OwningPawn->IsLocallyControlled())
		{
			return;
		}

		if (TrailFX)
		{
			UParticleSystemComponent* TrailPSC = UGameplayStatics::SpawnEmitterAtLocation(this, TrailFX, Origin);
			if (TrailPSC)
			{
				TrailPSC->SetVectorParameter(TrailTargetParam, EndPoint);
			}
		}
	}
}
