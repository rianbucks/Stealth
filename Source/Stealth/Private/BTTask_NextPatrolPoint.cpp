#include "BTTask_NextPatrolPoint.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"
#include "Engine/TargetPoint.h"
#include "EnemySearcher.h"

UBTTask_NextPatrolPoint::UBTTask_NextPatrolPoint()
{
	NodeName = TEXT("Next Patrol Point");
	BlackboardKey.AddVectorFilter(this, GET_MEMBER_NAME_CHECKED(UBTTask_NextPatrolPoint, BlackboardKey));
}

EBTNodeResult::Type UBTTask_NextPatrolPoint::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	const AAIController* AI = OwnerComp.GetAIOwner();
	AEnemySearcher* Enemy = AI ? Cast<AEnemySearcher>(AI->GetPawn()) : nullptr;
	UBlackboardComponent* Blackboard = OwnerComp.GetBlackboardComponent();
	if (!Enemy || !Blackboard || Enemy->PatrolPoints.Num() == 0) { return EBTNodeResult::Failed; }

	Enemy->PatrolIndex = (Enemy->PatrolIndex + 1) % Enemy->PatrolPoints.Num();

	const AActor* Point = Enemy->PatrolPoints[Enemy->PatrolIndex];
	if (!Point) { return EBTNodeResult::Failed; }

	Blackboard->SetValueAsVector(BlackboardKey.SelectedKeyName, Point->GetActorLocation());
	return EBTNodeResult::Succeeded;
}