#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_LookAround.generated.h"

UCLASS()
class STEALTH_API UBTTask_LookAround : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_LookAround();

protected:
	UPROPERTY(EditAnywhere, Category = "Look Around")
	float LookAngle = 90.f;

	UPROPERTY(EditAnywhere, Category = "Look Around")
	float HoldTime = 1.f;

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	virtual EBTNodeResult::Type AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual FString GetStaticDescription() const override;

private:
	float BaseYaw = 0.f;
	int32 Step = 0;
	float Held = 0.f;

	float StepOffset(int32 InStep) const;
	float StepHold(int32 InStep) const;
};