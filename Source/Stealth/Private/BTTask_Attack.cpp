#include "BTTask_Attack.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Stealth/StealthCharacter.h"

UBTTask_Attack::UBTTask_Attack()
{
	NodeName = TEXT("Attack");
	BlackboardKey.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(UBTTask_Attack, BlackboardKey), AActor::StaticClass());
}

EBTNodeResult::Type UBTTask_Attack::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	const AAIController* AI = OwnerComp.GetAIOwner();
	APawn* Self = AI ? AI->GetPawn() : nullptr;
	const UBlackboardComponent* Board = OwnerComp.GetBlackboardComponent();
	AStealthCharacter* Target = Board ? Cast<AStealthCharacter>(Board->GetValueAsObject(BlackboardKey.SelectedKeyName)) : nullptr;
	if (!Self || !Target) { return EBTNodeResult::Failed; }

	if (FVector::Dist(Self->GetActorLocation(), Target->GetActorLocation()) > Range) { return EBTNodeResult::Failed; }

	Target->ApplyEnemyHit(Self);
	return EBTNodeResult::Succeeded;
}

FString UBTTask_Attack::GetStaticDescription() const
{
	return FString::Printf(TEXT("Attack %s (range %.0f)"), *BlackboardKey.SelectedKeyName.ToString(), Range);
}