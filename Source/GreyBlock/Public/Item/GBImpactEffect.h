// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Actor.h"
#include "GBImpactEffect.generated.h"

UCLASS(ABSTRACT, Blueprintable)
class GREYBLOCK_API AGBImpactEffect : public AActor
{
	GENERATED_BODY()

protected:

	UParticleSystem* GetImpactFX(EPhysicalSurface SurfaceType) const;

	USoundCue* GetImpactSound(EPhysicalSurface SurfaceType) const;
	
public:	
	// Sets default values for this actor's properties
	AGBImpactEffect();

	virtual void PostInitializeComponents() override;

	/* FX spawned on standard materials */
	UPROPERTY(EditDefaultsOnly)
	UParticleSystem* DefaultFX;

	UPROPERTY(EditDefaultsOnly)
	UParticleSystem* PlayerFleshFX;

	UPROPERTY(EditDefaultsOnly)
	UParticleSystem* EnemyFleshFX;

	UPROPERTY(EditDefaultsOnly)
	USoundCue* DefaultSound;

	UPROPERTY(EditDefaultsOnly)
	USoundCue* PlayerFleshSound;

	UPROPERTY(EditDefaultsOnly)
	USoundCue* EnemyFleshSound;

	UPROPERTY(EditDefaultsOnly, Category = "Decal")
	UMaterial* DecalMaterial;

	UPROPERTY(EditDefaultsOnly, Category = "Decal")
	float DecalSize;

	UPROPERTY(EditDefaultsOnly, Category = "Decal")
	float DecalLifeSpan;

	FHitResult SurfaceHit;
};
