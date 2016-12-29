// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/PlayerState.h"
#include "GBPlayerState.generated.h"

/**
 * 
 */
UCLASS()
class GREYBLOCK_API AGBPlayerState : public APlayerState
{
	GENERATED_BODY()
	
	AGBPlayerState(const FObjectInitializer& ObjectInitializer);

	UPROPERTY(Transient)
		int32 NumKills;

	UPROPERTY(Transient)
		int32 NumDeaths;

	/* Team number assigned to player */
	UPROPERTY(Transient)
		int32 TeamNumber;

	virtual void Reset() override;

public:

	void AddKill();

	void AddDeath();

	void ScorePoints(int32 Points);

	void SetTeamNumber(int32 NewTeamNumber);

	UFUNCTION(BlueprintCallable, Category = "Teams")
		int32 GetTeamNumber() const;

	UFUNCTION(BlueprintCallable, Category = "Score")
		int32 GetKills() const;

	UFUNCTION(BlueprintCallable, Category = "Score")
		int32 GetDeaths() const;

	UFUNCTION(BlueprintCallable, Category = "Score")
		float GetScore() const;
	
	
};
