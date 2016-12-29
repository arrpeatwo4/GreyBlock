// Fill out your copyright notice in the Description page of Project Settings.

#include "GreyBlock.h"
#include "GreyBlockEnemyCharacter.h"
#include "AGBEnemyAIController.h"

/* AI Specific Includes */
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"

AAGBEnemyAIController::AAGBEnemyAIController()
{

}


AAGBEnemyAIController::AAGBEnemyAIController(const class FObjectInitializer & ObjectInitializer)
{
	BehaviorComp = ObjectInitializer.CreateDefaultSubobject<UBehaviorTreeComponent>(this, TEXT("BehaviorComp"));
	BlackboardComp = ObjectInitializer.CreateDefaultSubobject<UBlackboardComponent>(this, TEXT("BlackboardComp"));

	/* Match with the AI/ZombieBlackboard */
	PatrolLocationKeyName = "PatrolLocation";
	CurrentWaypointKeyName = "CurrentWaypoint";
	BotTypeKeyName = "BotType";
	TargetEnemyKeyName = "TargetEnemy";

	/* Initializes PlayerState so we can assign a team index to AI */
	bWantsPlayerState = true;
}


void AAGBEnemyAIController::Possess(class APawn * InPawn)
{
	Super::Possess(InPawn);

	AGreyBlockEnemyCharacter* EnemyBot = Cast<AGreyBlockEnemyCharacter>(InPawn);
	if (EnemyBot)
	{
		if (EnemyBot->BehaviorTree->BlackboardAsset)
		{
			BlackboardComp->InitializeBlackboard(*EnemyBot->BehaviorTree->BlackboardAsset);

			/* Make sure the Blackboard has the type of bot we possessed */
			SetBlackboardBotType(EnemyBot->BotType);
		}

		BehaviorComp->StartTree(*EnemyBot->BehaviorTree);
	}
}


void AAGBEnemyAIController::UnPossess()
{
	Super::UnPossess();

	/* Stop any behavior running as we no longer have a pawn to control */
	BehaviorComp->StopTree();
}


void AAGBEnemyAIController::SetWaypoint(AGBBotWayPoint * NewWaypoint)
{
	if (BlackboardComp)
	{
		BlackboardComp->SetValueAsObject(CurrentWaypointKeyName, NewWaypoint);
	}
}


void AAGBEnemyAIController::SetTargetEnemy(APawn * NewTarget)
{
	if (BlackboardComp)
	{
		BlackboardComp->SetValueAsObject(TargetEnemyKeyName, NewTarget);
	}
}


AGBBotWayPoint * AAGBEnemyAIController::GetWayPoint()
{
	if (BlackboardComp)
	{
		return Cast<AGBBotWayPoint>(BlackboardComp->GetValueAsObject(CurrentWaypointKeyName));
	}

	return nullptr;
}


ABaseCharacter * AAGBEnemyAIController::GetTargetEnemy()
{
	if (BlackboardComp)
	{
		return Cast<ABaseCharacter>(BlackboardComp->GetValueAsObject(TargetEnemyKeyName));
	}

	return nullptr;
}


void AAGBEnemyAIController::SetBlackboardBotType(EBotBehaviorType NewType)
{
	if (BlackboardComp)
	{
		BlackboardComp->SetValueAsEnum(BotTypeKeyName, (uint8)NewType);
	}
}