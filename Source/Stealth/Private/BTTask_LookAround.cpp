#include "BTTask_LookAround.h"
#include "EnemyAIController.h"
#include "GameFramework/Pawn.h"

UBTTask_LookAround::UBTTask_LookAround()
{
	NodeName = TEXT("Look Around");
	bNotifyTick = true;
	bCreateNodeInstance = true;
}

EBTNodeResult::Type UBTTask_LookAround::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AEnemyAIController* AI = Cast<AEnemyAIController>(OwnerComp.GetAIOwner());
	const APawn* Pawn = AI ? AI->GetPawn() : nullptr;
	if (!AI || !Pawn) { return EBTNodeResult::Failed; }

	BaseYaw = Pawn->GetActorRotation().Yaw;
	Step = 0;
	Held = 0.f;

	AI->SetLookAround(true);
	AI->LookToward(BaseYaw + StepOffset(Step));
	UE_LOG(LogTemp, Warning, TEXT("%s: LOOK %+.0f"), *Pawn->GetActorNameOrLabel(), StepOffset(Step));
	return EBTNodeResult::InProgress;
}

void UBTTask_LookAround::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	AEnemyAIController* AI = Cast<AEnemyAIController>(OwnerComp.GetAIOwner());
	const APawn* Pawn = AI ? AI->GetPawn() : nullptr;
	if (!AI || !Pawn)
	{
		if (AI) { AI->SetLookAround(false); }
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}

	if (FMath::Abs(FMath::FindDeltaAngleDegrees(Pawn->GetActorRotation().Yaw, BaseYaw + StepOffset(Step))) > 5.f) { return; }

	Held += DeltaSeconds;
	if (Held < StepHold(Step)) { return; }

	if (Step < 2)
	{
		++Step;
		Held = 0.f;
		AI->LookToward(BaseYaw + StepOffset(Step));
		UE_LOG(LogTemp, Warning, TEXT("%s: LOOK %+.0f"), *Pawn->GetActorNameOrLabel(), StepOffset(Step));
		return;
	}

	AI->SetLookAround(false);
	UE_LOG(LogTemp, Warning, TEXT("%s: LOOK DONE"), *Pawn->GetActorNameOrLabel());
	FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
}

float UBTTask_LookAround::StepOffset(int32 InStep) const
{
	if (InStep == 0) { return LookAngle; }
	if (InStep == 1) { return 0.f; }
	return -LookAngle;
}

float UBTTask_LookAround::StepHold(int32 InStep) const
{
	return InStep == 1 ? 0.f : HoldTime;
}

EBTNodeResult::Type UBTTask_LookAround::AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	if (AEnemyAIController* AI = Cast<AEnemyAIController>(OwnerComp.GetAIOwner()))
	{
		AI->SetLookAround(false);
	}
	return EBTNodeResult::Aborted;
}

FString UBTTask_LookAround::GetStaticDescription() const
{
	return FString::Printf(TEXT("Look Around +/-%.0f, hold %.1fs"), LookAngle, HoldTime);
}