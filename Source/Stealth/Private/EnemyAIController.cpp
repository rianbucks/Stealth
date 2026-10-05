#include "EnemyAIController.h"
#include "EnemySearcher.h"
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
	if (!Actor || !GetPawn()) { return; }

	const float Distance = FVector::Dist(GetPawn()->GetActorLocation(), Actor->GetActorLocation());

	UE_LOG(LogTemp, Warning, TEXT("%s -> %s %s (dist %.0f)"),
		*GetPawn()->GetActorNameOrLabel(),
		*Actor->GetActorNameOrLabel(),
		Stimulus.WasSuccessfullySensed() ? TEXT("SEEN") : TEXT("LOST"),
		Distance);
}