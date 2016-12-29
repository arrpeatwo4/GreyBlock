// Fill out your copyright notice in the Description page of Project Settings.

#include "GreyBlock.h"
#include "GBSpectatorPawn.h"




AGBSpectatorPawn::AGBSpectatorPawn(const class FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bAddDefaultMovementBindings = true;
}