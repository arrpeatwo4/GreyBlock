// Fill out your copyright notice in the Description page of Project Settings.

#include "GreyBlock.h"
#include "GBMutator.h"


bool AGBMutator::CheckRelevance(AActor* Other)
{
	if (NextMutator)
	{
		return NextMutator->CheckRelevance(Other);
	}

	return true;
}


void AGBMutator::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
	if (NextMutator)
	{
		//NextMutator->InitGame(MapName, Options, ErrorMessage);
	}
}

