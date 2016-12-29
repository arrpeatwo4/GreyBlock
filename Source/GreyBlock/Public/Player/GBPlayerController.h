// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/PlayerController.h"
#include "GBPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class GREYBLOCK_API AGBPlayerController : public APlayerController
{
	GENERATED_BODY()
	
	AGBPlayerController(const FObjectInitializer& ObjectInitializer);	
	
	/* Flag to respawn or start spectating upon death */
	UPROPERTY(EditDefaultsOnly, Category = "Spawning")
	bool bRespawnImmediately;
};
