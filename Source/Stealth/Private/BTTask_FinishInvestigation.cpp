#include "BTTask_FinishInvestigation.h"
#include "BehaviorTree/BlackboardComponent.h"

UBTTask_FinishInvestigation::UBTTask_FinishInvestigation()
{
	NodeName = TEXT("Finish Investigation");
	BlackboardKey.AddBoolFilter(this, GET_MEMBER_NAME_CHECKED(UBTTask_FinishInvestigation, BlackboardKey));
}

EBTNodeResult::Type UBTTask_FinishInvestigation::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	UBlackboardComponent* Blackboard = OwnerComp.GetBlackboardComponent();
	if (!Blackboard) { return EBTNodeResult::Failed; }

	Blackboard->SetValueAsBool(BlackboardKey.SelectedKeyName, false);
	return EBTNodeResult::Succeeded;
}