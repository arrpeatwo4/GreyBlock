// Fill out your copyright notice in the Description page of Project Settings.

#include "GreyBlock.h"
#include "GBGameState.h"
#include "GBPlayerState.h"


AGBPlayerState::AGBPlayerState(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	/* AI will remain in team 0, players are updated to team 1 through the GameMode::InitNewPlayer */
	TeamNumber = 0;
}


void AGBPlayerState::Reset()
{
	Super::Reset();

	NumKills = 0;
	NumDeaths = 0;
	Score = 0;
}

void AGBPlayerState::AddKill()
{
	NumKills++;
}

void AGBPlayerState::AddDeath()
{
	NumDeaths++;
}

void AGBPlayerState::ScorePoints(int32 Points)
{
	Score += Points;

	/* Add the score to the global score count */
	AGBGameState* GS = Cast<AGBGameState>(GetWorld()->GameState);
	if (GS)
	{
		GS->AddScore(Points);
	}
}


void AGBPlayerState::SetTeamNumber(int32 NewTeamNumber)
{
	TeamNumber = NewTeamNumber;
}


int32 AGBPlayerState::GetTeamNumber() const
{
	return TeamNumber;
}

int32 AGBPlayerState::GetKills() const
{
	return NumKills;
}

int32 AGBPlayerState::GetDeaths() const
{
	return NumDeaths;
}


float AGBPlayerState::GetScore() const
{
	return Score;
}
