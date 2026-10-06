#include "EnemyAIController.h"
#include "EnemySearcher.h"
#include "Stealth/StealthCharacter.h"
#include "TimerManager.h"
#include "DrawDebugHelpers.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Navigation/PathFollowingComponent.h"

AEnemyAIController::AEnemyAIController()
{
	AIPerception = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerception"));
	SetPerceptionComponent(*AIPerception);

	SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
	SightConfig->SightRadius = Sight_Radius;
	SightConfig->LoseSightRadius = Sight_LoseRadius;
	SightConfig->PeripheralVisionAngleDegrees = Sight_HalfAngle;
	SightConfig->SetMaxAge(Sight_MaxAge);

	SightConfig->DetectionByAffiliation.bDetectEnemies = true;
	SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
	SightConfig->DetectionByAffiliation.bDetectFriendlies = true;

	AIPerception->ConfigureSense(*SightConfig);
	AIPerception->SetDominantSense(SightConfig->GetSenseImplementation());
}

void AEnemyAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	UE_LOG(LogTemp, Warning, TEXT("AIController possessed: %s"),
		*GetNameSafe(InPawn));
}

void AEnemyAIController::BeginPlay()
{
	Super::BeginPlay();

	if (AIPerception)
	{
		AIPerception->OnTargetPerceptionUpdated.AddDynamic(
			this, &AEnemyAIController::OnPerceptionUpdated);
	}

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

	GetWorldTimerManager().SetTimer(
		WaitTimer, this, &AEnemyAIController::GoToNextPoint,
		Enemy->PatrolWaitTime, false);
}


void AEnemyAIController::GoToNextPoint()
{
	AEnemySearcher* Enemy = Cast<AEnemySearcher>(GetPawn());
	if (!Enemy || Enemy->PatrolPoints.Num() == 0) { return; }

	CurrentIndex = (CurrentIndex + 1) % Enemy->PatrolPoints.Num();

	MoveToCurrentPoint();
}

void AEnemyAIController::OnPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
	AStealthCharacter* Player = Cast<AStealthCharacter>(Actor);
	if (!Player || !GetPawn()) { return; }

	if (Stimulus.WasSuccessfullySensed())
	{
		UE_LOG(LogTemp, Warning, TEXT("%s: IN SIGHT"), *GetPawn()->GetActorNameOrLabel());

		SightTarget = Player;
		GetWorldTimerManager().SetTimer(SightCheckTimer, this,
			&AEnemyAIController::CheckSightTarget, SightCheckInterval, true);
		CheckSightTarget();
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("%s: OUT OF SIGHT"), *GetPawn()->GetActorNameOrLabel());

		GetWorldTimerManager().ClearTimer(SightCheckTimer);
		SightTarget = nullptr;

		if (bCanSeePlayer)
		{
			bCanSeePlayer = false;
			UE_LOG(LogTemp, Warning, TEXT("%s: LOST"), *GetPawn()->GetActorNameOrLabel());
		}
	}
}

void AEnemyAIController::CheckSightTarget()
{
	if (!SightTarget || !GetPawn()) { return; }

	const bool bExposed = SightTarget->IsExposed();
	if (bExposed != bCanSeePlayer)
	{
		bCanSeePlayer = bExposed;
		UE_LOG(LogTemp, Warning, TEXT("%s: %s (light %.2f)"),
			*GetPawn()->GetActorNameOrLabel(),
			bCanSeePlayer ? TEXT("DETECTED") : TEXT("HIDDEN"),
			SightTarget->GetCachedIlluminance());
	}

	const AEnemySearcher* Enemy = Cast<AEnemySearcher>(GetPawn());
	if (Enemy && Enemy->bShowDebug)
	{
		DrawDebugString(GetWorld(), FVector(0.f, 0.f, 120.f),
			bCanSeePlayer ? TEXT("!") : TEXT("?"), GetPawn(),
			bCanSeePlayer ? FColor::Red : FColor::Yellow,
			SightCheckInterval * 1.5f, true, 2.f);
	}
}