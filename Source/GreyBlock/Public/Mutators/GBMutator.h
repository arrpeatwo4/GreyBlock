// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Info.h"
#include "GBMutator.generated.h"

/**
 * 
 */
UCLASS()
class GREYBLOCK_API AGBMutator : public AInfo
{
	GENERATED_BODY()
	
public:

	UFUNCTION(BlueprintAuthorityOnly)
	void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage);

	

	/* Next mutator in the chain */
	AGBMutator* NextMutator;

	/** From UT: entry point for mutators modifying, replacing, or destroying Actors
	* return false to destroy Other
	* note that certain critical Actors such as PlayerControllers can't be destroyed, but we'll still call this code path to allow mutators
	* to change properties on them
	* MAKE SURE TO CALL SUPER TO PROCESS ADDITIONAL MUTATORS
	*/
	UFUNCTION(BlueprintAuthorityOnly)
		bool CheckRelevance(AActor* Other);
	
};
