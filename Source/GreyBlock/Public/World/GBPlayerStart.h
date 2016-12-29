// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/PlayerStart.h"
#include "GBPlayerStart.generated.h"

/**
 * 
 */
UCLASS()
class GREYBLOCK_API AGBPlayerStart : public APlayerStart
{
	GENERATED_BODY()

	AGBPlayerStart(const class FObjectInitializer & ObjectInitializer);

	UPROPERTY(EditAnywhere, Category = "PlayerStart")
		bool bPlayerOnly;

public:

	bool GetIsPlayerOnly() { return bPlayerOnly; }
	
	
};
