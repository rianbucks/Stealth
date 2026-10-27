#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_SetSpeed.generated.h"

UCLASS()
class STEALTH_API UBTTask_SetSpeed : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_SetSpeed();

protected:
	UPROPERTY(EditAnywhere, Category = "Movement")
	float Speed = 150.f;

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual FString GetStaticDescription() const override;
};