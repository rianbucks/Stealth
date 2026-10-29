#include "EnemyAIController.h"
#include "EnemySearcher.h"
#include "Stealth/StealthCharacter.h"
#include "TimerManager.h"
#include "TraceSubsystem.h"
#include "TraceMarker.h"
#include "LightingSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "Components/SphereComponent.h"
#include "Components/CapsuleComponent.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
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

	if (Enemy && Enemy->BehaviorTree)
	{
		RunBehaviorTree(Enemy->BehaviorTree);
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

	GetWorldTimerManager().SetTimer(LightCheckTimer, this,
		&AEnemyAIController::CheckLight, DetectionInterval, true);

	GetWorldTimerManager().SetTimer(NoiseCheckTimer, this,
		&AEnemyAIController::CheckNoise, DetectionInterval, true);

	GetWorldTimerManager().SetTimer(TraceCheckTimer, this,
		&AEnemyAIController::CheckTrace, DetectionInterval, true);
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

	if (SightTarget || NearbyPlayer || bLightNoticed || bNoiseHeard || bTraceSeen || Alert > 0.f)
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

	const AEnemySearcher* Searcher = Cast<AEnemySearcher>(GetPawn());

	const bool bSeen = SightTarget && SightTarget->IsExposed();

	bool bNear = NearbyPlayer != nullptr;
	if (bNear && Searcher && NearbyPlayer->GetStance() == EMovementStance::Crouch)
	{
		const float Distance = FVector::Dist(GetPawn()->GetActorLocation(), NearbyPlayer->GetActorLocation());
		bNear = Distance <= Searcher->Proximity_CrouchRadius;
	}

	if (bNear)
	{
		NoticedLocation = NearbyPlayer->GetActorLocation();
	}

	const bool bOtherSense = bNear || bLightNoticed || bNoiseHeard || bTraceSeen;

	const float Now = GetWorld()->GetTimeSeconds();
	const float DeltaTime = FMath::Min(Now - LastAlertTime, DetectionInterval);
	LastAlertTime = Now;

	if (bSeen && Searcher)
	{
		const float Distance = FVector::Dist(GetPawn()->GetActorLocation(), SightTarget->GetActorLocation());
		const float Ratio = FMath::Clamp(Distance / SightConfig->SightRadius, 0.f, 1.f);
		const float FillTime = FMath::Max(FMath::Lerp(Searcher->Alert_FillTimeNear, Searcher->Alert_FillTimeFar, Ratio), 0.01f);

		Alert = FMath::Min(Alert + DeltaTime / FillTime, 1.f);
		NoticedLocation = SightTarget->GetActorLocation();
	}
	else if (Searcher)
	{
		Alert = FMath::Max(Alert - DeltaTime / FMath::Max(Searcher->Alert_DecayTime, 0.01f), 0.f);

		if (bOtherSense)
		{
			Alert = FMath::Max(Alert, Searcher->Alert_SuspicionLevel);
		}
	}

	const bool bDetected = Alert >= 1.f;
	const bool bSuspect = !bDetected && Alert > 0.f;

	if (bDetected != bPlayerDetected)
	{
		bPlayerDetected = bDetected;

		if (bPlayerDetected)
		{
			UE_LOG(LogTemp, Warning, TEXT("%s: DETECTED (light %.2f)"),
				*GetPawn()->GetActorNameOrLabel(), SightTarget ? SightTarget->GetCachedIlluminance() : 0.f);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("%s: HIDDEN"), *GetPawn()->GetActorNameOrLabel());
		}
	}

	if (bSuspect != bSuspicious)
	{
		bSuspicious = bSuspect;

		if (bSuspicious)
		{
			const TCHAR* Reason = bSeen ? TEXT("sight")
				: (bNear ? TEXT("near")
					: (bLightNoticed ? TEXT("light")
						: (bNoiseHeard ? TEXT("noise")
							: (bTraceSeen ? TEXT("trace") : TEXT("memory")))));
			UE_LOG(LogTemp, Warning, TEXT("%s: SUSPICIOUS (%s)"), *GetPawn()->GetActorNameOrLabel(), Reason);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("%s: CALM"), *GetPawn()->GetActorNameOrLabel());
		}
	}

	if (Searcher && Searcher->bShowDebug && (bPlayerDetected || bSuspicious))
	{
		DrawDebugString(GetWorld(), FVector(0.f, 0.f, 120.f),
			FString::Printf(TEXT("%s %d"), bPlayerDetected ? TEXT("!") : TEXT("?"), FMath::RoundToInt(Alert * 100.f)),
			GetPawn(), bPlayerDetected ? FColor::Red : FColor::Yellow,
			DetectionInterval * 1.5f, true, 2.f);
	}

	if (bPlayerDetected && SightTarget)
	{
		RequestInvestigation(SightTarget->GetActorLocation());
	}
	else if (bSuspicious)
	{
		RequestInvestigation(NoticedLocation);
	}

	if (Alert <= 0.f && !SightTarget && !NearbyPlayer && !bLightNoticed && !bNoiseHeard && !bTraceSeen)
	{
		GetWorldTimerManager().ClearTimer(DetectionTimer);
	}
}

void AEnemyAIController::CheckLight()
{
	APawn* MyPawn = GetPawn();
	const AStealthCharacter* Player = Cast<AStealthCharacter>(UGameplayStatics::GetPlayerCharacter(this, 0));
	if (!MyPawn || !Player) { return; }

	const AEnemySearcher* Enemy = Cast<AEnemySearcher>(MyPawn);

	bool bNoticed = false;
	bool bOnMe = false;
	FVector Where = FVector::ZeroVector;

	if (Player->IsFlashlightOn())
	{
		const float Threshold = Player->GetExposureThreshold();
		FVector LitPoint = FVector::ZeroVector;

		if (Player->GetFlashlightLightAt(MyPawn->GetActorLocation()) >= Threshold)
		{
			bNoticed = true;
			bOnMe = true;
			Where = Player->GetActorLocation();
		}
		else if (Player->GetFlashlightLitPoint(LitPoint)
			&& Player->GetFlashlightLightAt(LitPoint) >= Threshold
			&& CanSeeLocation(LitPoint, Enemy ? Enemy->LightNotice_HalfAngle : SightConfig->PeripheralVisionAngleDegrees))
		{
			bNoticed = true;
			Where = LitPoint;
		}
	}

	if (bNoticed)
	{
		NoticedLocation = Where;
	}

	if (bNoticed != bLightNoticed)
	{
		bLightNoticed = bNoticed;

		RefreshWatch();

		if (bLightNoticed)
		{
			UE_LOG(LogTemp, Warning, TEXT("%s: LIGHT NOTICED (%s)"), *MyPawn->GetActorNameOrLabel(),
				bOnMe ? TEXT("on me") : TEXT("lit point"));
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("%s: LIGHT GONE"), *MyPawn->GetActorNameOrLabel());
		}
	}

	if (Enemy && Enemy->bShowDebug && bLightNoticed)
	{
		DrawDebugLine(GetWorld(), MyPawn->GetPawnViewLocation(), NoticedLocation,
			FColor::Orange, false, DetectionInterval * 1.5f, 0, 2.f);
		DrawDebugSphere(GetWorld(), NoticedLocation, 25.f, 8, FColor::Orange,
			false, DetectionInterval * 1.5f, 0, 2.f);
	}
}

bool AEnemyAIController::CanSeeLocation(const FVector& Target, float HalfAngle) const
{
	if (!GetPawn() || !SightConfig) { return false; }

	FVector EyeLocation;
	FRotator EyeRotation;
	GetActorEyesViewPoint(EyeLocation, EyeRotation);

	const FVector ToTarget = Target - EyeLocation;
	const float Distance = ToTarget.Size();
	if (Distance > SightConfig->SightRadius || Distance < 1.f) { return false; }

	const float CosAngle = FVector::DotProduct(EyeRotation.Vector(), ToTarget / Distance);
	if (CosAngle < FMath::Cos(FMath::DegreesToRadians(HalfAngle))) { return false; }

	FCollisionQueryParams Params;
	Params.AddIgnoredActor(GetPawn());

	FHitResult Hit;
	return !GetWorld()->LineTraceSingleByChannel(Hit, EyeLocation, Target, ECC_Visibility, Params);
}

void AEnemyAIController::CheckNoise()
{
	const APawn* MyPawn = GetPawn();
	const UTraceSubsystem* Traces = GetWorld()->GetSubsystem<UTraceSubsystem>();
	if (!MyPawn || !Traces) { return; }

	bool bHeard = false;
	float NewestTime = -1.f;

	for (const TObjectPtr<ATraceMarker>& Marker : Traces->GetMarkers())
	{
		if (!IsValid(Marker) || Marker->GetTraceType() != ETraceType::Noise) { continue; }

		if (!IsNoiseAudible(Marker)) { continue; }

		if (Marker->GetSpawnTime() > NewestTime)
		{
			NewestTime = Marker->GetSpawnTime();
			NoticedLocation = Marker->GetActorLocation();
			bHeard = true;
		}
	}

	if (bHeard != bNoiseHeard)
	{
		bNoiseHeard = bHeard;
		RefreshWatch();
	}
}

void AEnemyAIController::RequestInvestigation(const FVector& Location)
{
	UBlackboardComponent* Board = GetBlackboardComponent();
	const AEnemySearcher* Searcher = Cast<AEnemySearcher>(GetPawn());
	if (!Board || !Searcher) { return; }

	if (Board->GetValueAsBool(TEXT("bInvestigating")))
	{
		if (!Searcher->bInvestigate_Retarget) { return; }

		const FVector Current = Board->GetValueAsVector(TEXT("InvestigateLocation"));
		if (FVector::Dist(Current, Location) < Searcher->Investigate_RetargetDistance) { return; }
	}

	Board->SetValueAsVector(TEXT("InvestigateLocation"), Location);
	Board->SetValueAsBool(TEXT("bInvestigating"), true);
}

bool AEnemyAIController::IsNoiseAudible(ATraceMarker* Noise) const
{
	const APawn* MyPawn = GetPawn();
	const AEnemySearcher* Searcher = Cast<AEnemySearcher>(MyPawn);
	if (!Noise || !Searcher) { return false; }

	const float Radius = Noise->GetRadius();
	const FVector Ear = MyPawn->GetActorLocation();
	const float Dist = FVector::Dist(Noise->GetActorLocation(), Ear);
	if (Dist > Radius) { return false; }
	if (Dist <= Radius * Searcher->Noise_WallFactor) { return true; }

	const FVector From = Noise->GetActorLocation() + FVector(0.f, 0.f, 50.f);
	const FCollisionObjectQueryParams Params(ECC_WorldStatic);
	return !GetWorld()->LineTraceTestByObjectType(From, Ear, Params);
}


void AEnemyAIController::CheckTrace()
{
	const AEnemySearcher* Searcher = Cast<AEnemySearcher>(GetPawn());
	const UTraceSubsystem* Traces = GetWorld()->GetSubsystem<UTraceSubsystem>();
	const ULightingSubsystem* Lighting = GetWorld()->GetSubsystem<ULightingSubsystem>();
	if (!Searcher || !Traces || !Lighting || !SightConfig) { return; }

	ATraceMarker* Found = nullptr;
	const TArray<TObjectPtr<ATraceMarker>>& Markers = Traces->GetMarkers();

	for (int32 i = Markers.Num() - 1; i >= 0; --i)
	{
		ATraceMarker* Marker = Markers[i];
		if (!IsValid(Marker)) { continue; }
		if (Marker == LastSeenTrace.Get()) { break; }
		if (Marker->GetTraceType() == ETraceType::Noise) { continue; }

		const FVector Point = Marker->GetActorLocation();
		if (FVector::Dist(Point, Searcher->GetActorLocation()) > Searcher->TraceDetect_Range) { continue; }
		if (!CanSeeLocation(Point, SightConfig->PeripheralVisionAngleDegrees)) { continue; }
		if (Lighting->GetLightIntensityAtLocation(Point) < Searcher->TraceDetect_LightThreshold) { continue; }

		Found = Marker;
		break;
	}

	if (Found)
	{
		LastSeenTrace = Found;
		NoticedLocation = Found->GetActorLocation();

		UE_LOG(LogTemp, Warning, TEXT("%s: TRACE SEEN (%s)"), *Searcher->GetActorNameOrLabel(),
			*UEnum::GetValueAsString(Found->GetTraceType()));
	}

	const bool bSeenNow = LastSeenTrace.IsValid();
	if (bSeenNow != bTraceSeen)
	{
		bTraceSeen = bSeenNow;
		RefreshWatch();
	}

	if (Searcher->bShowDebug && bTraceSeen)
	{
		DrawDebugLine(GetWorld(), Searcher->GetPawnViewLocation(), LastSeenTrace->GetActorLocation(),
			FColor::Green, false, DetectionInterval * 1.5f, 0, 2.f);
	}
}