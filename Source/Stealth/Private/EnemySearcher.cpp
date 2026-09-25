#include "EnemySearcher.h"
#include "EnemyAIController.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "LightingSubsystem.h"
#include "TimerManager.h"
#include "Components/SkeletalMeshComponent.h"

AEnemySearcher::AEnemySearcher()
{
	PrimaryActorTick.bCanEverTick = true;

	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 300.0f, 0.0f);
	GetCharacterMovement()->MaxWalkSpeed = Speed_Patrol;

	GetCharacterMovement()->GetNavMovementProperties()->bUseAccelerationForPaths = true;

	AIControllerClass = AEnemyAIController::StaticClass();

	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
}

void AEnemySearcher::BeginPlay()
{
	Super::BeginPlay();

	GetWorldTimerManager().SetTimer(VisibilityTimer, this,
		&AEnemySearcher::UpdateVisibility, 0.1f, true);
}

void AEnemySearcher::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AEnemySearcher::UpdateVisibility()
{
	const ULightingSubsystem* Lighting = GetWorld()->GetSubsystem<ULightingSubsystem>();
	if (!Lighting)
	{
		return;
	}

	const float Value = Lighting->GetLightIntensityAtLocation(GetActorLocation());

	const float ShowAt = VisibilityThreshold + VisibilityHysteresis;
	const float HideAt = VisibilityThreshold - VisibilityHysteresis;

	if (bCurrentlyVisible && Value < HideAt)
	{
		bCurrentlyVisible = false;
		GetMesh()->SetVisibility(false);

		UE_LOG(LogTemp, Warning, TEXT("%s -> HIDDEN (light %.2f)"),
			*GetActorNameOrLabel(), Value);
	}
	else if (!bCurrentlyVisible && Value > ShowAt)
	{
		bCurrentlyVisible = true;
		GetMesh()->SetVisibility(true);

		UE_LOG(LogTemp, Warning, TEXT("%s -> VISIBLE (light %.2f)"),
			*GetActorNameOrLabel(), Value);
	}
}