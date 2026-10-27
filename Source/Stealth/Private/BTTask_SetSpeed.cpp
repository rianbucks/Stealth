#include "BTTask_SetSpeed.h"
#include "AIController.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

UBTTask_SetSpeed::UBTTask_SetSpeed()
{
	NodeName = TEXT("Set Speed");
}

EBTNodeResult::Type UBTTask_SetSpeed::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	const AAIController* AI = OwnerComp.GetAIOwner();
	const ACharacter* Character = AI ? Cast<ACharacter>(AI->GetPawn()) : nullptr;
	if (!Character) { return EBTNodeResult::Failed; }

	Character->GetCharacterMovement()->MaxWalkSpeed = Speed;
	return EBTNodeResult::Succeeded;
}

FString UBTTask_SetSpeed::GetStaticDescription() const
{
	return FString::Printf(TEXT("Speed %.0f"), Speed);
}