#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Tasks/BTTask_BlackboardBase.h"
#include "BTTask_Attack.generated.h"

UCLASS()
class STEALTH_API UBTTask_Attack : public UBTTask_BlackboardBase
{
	GENERATED_BODY()

public:
	UBTTask_Attack();

protected:
	UPROPERTY(EditAnywhere, Category = "Attack")
	float Range = 150.f;

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual FString GetStaticDescription() const override;
};