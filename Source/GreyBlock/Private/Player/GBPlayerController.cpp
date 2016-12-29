// Fill out your copyright notice in the Description page of Project Settings.

#include "GreyBlock.h"
#include "GBPlayerCameraManager.h"
#include "GBPlayerController.h"


AGBPlayerController::AGBPlayerController(const class FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	/* Assign the class types we wish to use */
	PlayerCameraManagerClass = AGBPlayerCameraManager::StaticClass();

	/* Example - Can be set to true for debugging, generally a value like this would exist in the GameMode instead */
	bRespawnImmediately = false;
}


