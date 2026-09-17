// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemyAIController.h"
#include "EnemySearcher.h"
#include "TimerManager.h"
#include "Navigation/PathFollowingComponent.h"

void AEnemyAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	UE_LOG(LogTemp, Warning, TEXT("AIController possessed: %s"),
		*GetNameSafe(InPawn));
}

void AEnemyAIController::BeginPlay()
{
	Super::BeginPlay();

	// Short delay so the pawn is possessed and NavMesh is ready.
	FTimerHandle Handle;
	GetWorldTimerManager().SetTimer(
		Handle, this, &AEnemyAIController::StartFirstMove, 0.5f, false);
}

void AEnemyAIController::StartFirstMove()
{
	AEnemySearcher* Enemy = Cast<AEnemySearcher>(GetPawn());
	if (!Enemy)
	{
		UE_LOG(LogTemp, Warning, TEXT("StartFirstMove: no pawn"));
		return;
	}

	if (!Enemy->FirstTarget)
	{
		UE_LOG(LogTemp, Warning, TEXT("StartFirstMove: %s has no FirstTarget"),
			*Enemy->GetName());
		return;
	}

	EPathFollowingRequestResult::Type Result = MoveToActor(Enemy->FirstTarget, 50.f);

	UE_LOG(LogTemp, Warning, TEXT("%s MoveToActor result: %d"), *Enemy->GetName(), (int32)Result);
}