// Fill out your copyright notice in the Description page of Project Settings.

#include "STypes.generated.h"
#pragma once

/**
 * 
 */

UENUM()
enum class EInventorySlot : uint8
{
	/* For currently equipped weapon */
	Hands,

	/* For primary weapon on spine bone */
	Primary,

	/* Storage for small items like flashlight on pelvis */
	Secondary,
};


UENUM()
enum class EBotBehaviorType : uint8
{
	/* Does not move, remains in place until a player is spotted */
	Passive,

	/* Patrols a region until a player is spotted */
	Patrolling,
};