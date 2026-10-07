#include "EnemyAIController.h"
#include "EnemySearcher.h"
#include "Stealth/StealthCharacter.h"
#include "TimerManager.h"
#include "DrawDebugHelpers.h"
#include "Components/SphereComponent.h"
#include "Components/CapsuleComponent.h"
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

	AEnemySearcher* Enemy = Cast<AEnemySearcher>(InPawn);
	if (Enemy && Enemy->ProximitySphere)
	{
		Enemy->ProximitySphere->OnComponentBeginOverlap.AddDynamic(
			this, &AEnemyAIController::OnProximityBegin);
		Enemy->ProximitySphere->OnComponentEndOverlap.AddDynamic(
			this, &AEnemyAIController::OnProximityEnd);
	}
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

	SightTarget = Stimulus.WasSuccessfullySensed() ? Player : nullptr;

	UE_LOG(LogTemp, Warning, TEXT("%s: %s"), *GetPawn()->GetActorNameOrLabel(),
		SightTarget ? TEXT("IN SIGHT") : TEXT("OUT OF SIGHT"));

	RefreshWatch();
}

void AEnemyAIController::OnProximityBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	AStealthCharacter* Player = Cast<AStealthCharacter>(OtherActor);
	if (!Player || OtherComp != Player->GetCapsuleComponent() || !GetPawn()) { return; }

	NearbyPlayer = Player;
	UE_LOG(LogTemp, Warning, TEXT("%s: NEARBY"), *GetPawn()->GetActorNameOrLabel());
	RefreshWatch();
}

void AEnemyAIController::OnProximityEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	AStealthCharacter* Player = Cast<AStealthCharacter>(OtherActor);
	if (!Player || OtherComp != Player->GetCapsuleComponent() || !GetPawn()) { return; }

	NearbyPlayer = nullptr;
	UE_LOG(LogTemp, Warning, TEXT("%s: LEFT NEARBY"), *GetPawn()->GetActorNameOrLabel());
	RefreshWatch();
}

void AEnemyAIController::RefreshWatch()
{
	FTimerManager& Timers = GetWorldTimerManager();

	if (SightTarget || NearbyPlayer)
	{
		if (!Timers.IsTimerActive(DetectionTimer))
		{
			Timers.SetTimer(DetectionTimer, this,
				&AEnemyAIController::UpdateDetection, DetectionInterval, true);
		}
	}
	else
	{
		Timers.ClearTimer(DetectionTimer);
	}

	UpdateDetection();
}

void AEnemyAIController::UpdateDetection()
{
	if (!GetPawn()) { return; }

	const bool bSeen = SightTarget && SightTarget->IsExposed();
	bool bNear = NearbyPlayer != nullptr;
	const AEnemySearcher* Searcher = Cast<AEnemySearcher>(GetPawn());
	if (bNear && Searcher && NearbyPlayer->GetStance() == EMovementStance::Crouch)
	{
		const float Distance = FVector::Dist(GetPawn()->GetActorLocation(), NearbyPlayer->GetActorLocation());
		bNear = Distance <= Searcher->Proximity_CrouchRadius;
	}
	const bool bDetected = bSeen || bNear;

	if (bDetected != bPlayerDetected)
	{
		bPlayerDetected = bDetected;

		if (!bPlayerDetected)
		{
			UE_LOG(LogTemp, Warning, TEXT("%s: HIDDEN"), *GetPawn()->GetActorNameOrLabel());
		}
		else if (bSeen)
		{
			UE_LOG(LogTemp, Warning, TEXT("%s: DETECTED (light %.2f)"),
				*GetPawn()->GetActorNameOrLabel(), SightTarget->GetCachedIlluminance());
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("%s: DETECTED (near)"), *GetPawn()->GetActorNameOrLabel());
		}
	}

	const AEnemySearcher* Enemy = Cast<AEnemySearcher>(GetPawn());
	if (Enemy && Enemy->bShowDebug && (SightTarget || NearbyPlayer))
	{
		DrawDebugString(GetWorld(), FVector(0.f, 0.f, 120.f),
			bPlayerDetected ? TEXT("!") : TEXT("?"), GetPawn(),
			bPlayerDetected ? FColor::Red : FColor::Yellow,
			DetectionInterval * 1.5f, true, 2.f);
	}
}