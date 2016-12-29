// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "AIController.h"
#include "BaseCharacter.h"
#include "GBBotWayPoint.h"
#include "STypes.h"
#include "AGBEnemyAIController.generated.h"

/**
 * 
 */
UCLASS()
class GREYBLOCK_API AAGBEnemyAIController : public AAIController
{
	GENERATED_BODY()

	AAGBEnemyAIController();

	AAGBEnemyAIController(const class FObjectInitializer & ObjectInitializer);


	/* Called whenever the controller possesses a character bot */
	virtual void Possess(class APawn * InPawn) override;

	virtual void UnPossess() override;

	UBehaviorTreeComponent * BehaviorComp;

	UBlackboardComponent * BlackboardComp;

	UPROPERTY(EditDefaultsOnly, Category = "AI")
		FName TargetEnemyKeyName;

	UPROPERTY(EditDefaultsOnly, Category = "AI")
		FName PatrolLocationKeyName;

	UPROPERTY(EditDefaultsOnly, Category = "AI")
		FName CurrentWaypointKeyName;

	UPROPERTY(EditDefaultsOnly, Category = "AI")
		FName BotTypeKeyName;

public:

	AGBBotWayPoint * GetWayPoint();

	ABaseCharacter * GetTargetEnemy();

	void SetWaypoint(AGBBotWayPoint * NewWayPoint);

	void SetTargetEnemy(APawn * NewTarget);

	void SetBlackboardBotType(EBotBehaviorType NewType);

	/* returns behaviorcom subobject */
	FORCEINLINE UBehaviorTreeComponent * GetBehaviorComp() const { return BehaviorComp; }

	FORCEINLINE UBlackboardComponent * GetBlackboardComp() const { return BlackboardComp; }

};
