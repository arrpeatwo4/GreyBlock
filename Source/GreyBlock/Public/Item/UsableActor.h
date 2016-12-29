// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "EngineMinimal.h"
#include "GameFramework/Actor.h"
#include "UsableActor.generated.h"

UCLASS(ABSTRACT)
class GREYBLOCK_API AUsableActor : public AActor
{
	GENERATED_BODY()

protected:

	AUsableActor(const FObjectInitializer & ObjectInitializer);

	UPROPERTY(VisibleAnywhere, Category = "Mesh")
	UStaticMeshComponent * MeshComp;
	
public:	
	// Sets default values for this actor's properties
	AUsableActor();

	/* PLayer is looking at */
	virtual void OnBeginFocus();

	/* Player is no longer looking at */
	virtual void OnEndFocus();

	virtual void OnUsed(APawn * InstigatorPawn);

	/* Public accessor to mesh component */
	FORCEINLINE UStaticMeshComponent * GetMeshComponent() const { return MeshComp; }

};
