// Fill out your copyright notice in the Description page of Project Settings.

#include "GreyBlock.h"
#include "UBTTask_FindPatrolLocation.h"
#include "GBBotWayPoint.h"
#include "AGBEnemyAIController.h"

/* AI Module Includes */
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/Blackboard/BlackboardKeyAllTypes.h"

EBTNodeResult::Type UUBTTask_FindPatrolLocation::ExecuteTask(UBehaviorTreeComponent & OwnerComp, uint8 * NodeMemory)
{
	AAGBEnemyAIController * MyController = Cast<AAGBEnemyAIController>(OwnerComp.GetAIOwner());
	if (MyController == nullptr)
	{
		return EBTNodeResult::Failed;
	}

	AGBBotWayPoint * MyWaypoint = MyController->GetWayPoint();
	if (MyWaypoint)
	{
		/* Find a position that is close to the waypoint, Add a small random to this position to give predictable patrol patterns */
		const float SearchRadius = 200.0f;
		const FVector SearchOrigin = MyWaypoint->GetActorLocation();
		const FVector Loc = UNavigationSystem::GetRandomPointInNavigableRadius(MyController, SearchOrigin, SearchRadius);	
		if (Loc != FVector::ZeroVector)
		{
			OwnerComp.GetBlackboardComponent()->SetValue<UBlackboardKeyType_Vector>(BlackboardKey.GetSelectedKeyID(), Loc);
			return EBTNodeResult::Succeeded;
		}
	}

	return EBTNodeResult::Failed;
}


