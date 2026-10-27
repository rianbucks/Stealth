#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Tasks/BTTask_BlackboardBase.h"
#include "BTTask_FinishInvestigation.generated.h"

UCLASS()
class STEALTH_API UBTTask_FinishInvestigation : public UBTTask_BlackboardBase
{
	GENERATED_BODY()

public:
	UBTTask_FinishInvestigation();

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
};