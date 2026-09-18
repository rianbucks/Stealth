// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemyAIController.h"
#include "EnemySearcher.h"
#include "TimerManager.h"
#include "DrawDebugHelpers.h"
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
	FTimerHandle StartTimer;
	GetWorldTimerManager().SetTimer(
		StartTimer, this, &AEnemyAIController::MoveToCurrentPoint, 0.5f, false);
}

void AEnemyAIController::MoveToCurrentPoint()
{
	AEnemySearcher* Enemy = Cast<AEnemySearcher>(GetPawn());
	if (!Enemy) { return; }

	if (!Enemy->PatrolPoints.IsValidIndex(CurrentIndex))
	{
		UE_LOG(LogTemp, Warning, TEXT("%s: no patrol point at index %d"),
			*Enemy->GetName(), CurrentIndex);
		return;
	}

	AActor* Target = Enemy->PatrolPoints[CurrentIndex];
	if (!Target)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s: patrol point %d is empty"),
			*Enemy->GetName(), CurrentIndex);
		return;
	}

	if (Enemy->bShowDebug)
	{
		DrawDebugSphere(GetWorld(), Target->GetActorLocation(),
			60.f, 12, FColor::Yellow, false, 6.f);
	}

	MoveToActor(Target, 50.f);
}


void AEnemyAIController::OnMoveCompleted(FAIRequestID RequestID,
	const FPathFollowingResult& Result)
{
	Super::OnMoveCompleted(RequestID, Result);

	AEnemySearcher* Enemy = Cast<AEnemySearcher>(GetPawn());
	if (!Enemy) { return; }

	// Wait, then head to the next point.
	GetWorldTimerManager().SetTimer(
		WaitTimer, this, &AEnemyAIController::GoToNextPoint,
		Enemy->PatrolWaitTime, false);
}


void AEnemyAIController::GoToNextPoint()
{
	AEnemySearcher* Enemy = Cast<AEnemySearcher>(GetPawn());
	if (!Enemy || Enemy->PatrolPoints.Num() == 0) { return; }

	// Wraps back to 0 after the last point.
	CurrentIndex = (CurrentIndex + 1) % Enemy->PatrolPoints.Num();

	MoveToCurrentPoint();
}