// Fill out your copyright notice in the Description page of Project Settings.

#include "GreyBlock.h"
#include "GBDamageType.h"

UGBDamageType::UGBDamageType(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	/* We apply this modifier based on the physics material setup to the head of the enemy PhysAsset */
	HeadDmgModifier = 2.0f;
	LimbDmgModifier = 0.5f;

	bCanDieFrom = true;
}


bool UGBDamageType::GetCanDieFrom()
{
	return bCanDieFrom;
}


float UGBDamageType::GetHeadDamageModifier()
{
	return HeadDmgModifier;
}

float UGBDamageType::GetLimbDamageModifier()
{
	return LimbDmgModifier;
}


