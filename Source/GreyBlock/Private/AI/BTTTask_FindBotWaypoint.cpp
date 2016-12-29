// Fill out your copyright notice in the Description page of Project Settings.

#include "GreyBlock.h"
#include "BTTTask_FindBotWaypoint.h"
#include "GBBotWayPoint.h"
#include "AGBEnemyAIController.h"

/* AI Module Includes */
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/Blackboard/BlackboardKeyAllTypes.h"

EBTNodeResult::Type UBTTTask_FindBotWaypoint::ExecuteTask(UBehaviorTreeComponent & OwnerComp, uint8 * NodeMemory)
{
	AAGBEnemyAIController * MyController = Cast<AAGBEnemyAIController>(OwnerComp.GetAIOwner());
	if (MyController == nullptr)
	{
		return EBTNodeResult::Failed;
	}

	AGBBotWayPoint * CurrentWaypoint = MyController->GetWayPoint();
	AActor * NewWaypoint = nullptr;

	/* Iterate over all the bot waypoints and find a new random one as destination */
	TArray<AActor*> AllWaypoints;
	UGameplayStatics::GetAllActorsOfClass(MyController, AGBBotWayPoint::StaticClass(), AllWaypoints);

	/* Find a new waypoint randomly by index (can include current waypoint */
	NewWaypoint = AllWaypoints[FMath::RandRange(0, AllWaypoints.Num() - 1)];

	/* Assign new waypoint to the blackboard */
	if (NewWaypoint)
	{
		/* Selected key should be "CurrentWaypoint in the BT setup */
		OwnerComp.GetBlackboardComponent()->SetValue<UBlackboardKeyType_Object>(BlackboardKey.GetSelectedKeyID(), NewWaypoint);
		return EBTNodeResult::Succeeded;
	}

	return EBTNodeResult::Failed;
}




