#include "EnemySearcher.h"
#include "EnemyAIController.h"
#include "Components/CapsuleComponent.h"
#include "Components/SphereComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
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

	ProximitySphere = CreateDefaultSubobject<USphereComponent>(TEXT("ProximitySphere"));
	ProximitySphere->SetupAttachment(GetCapsuleComponent());
	ProximitySphere->SetSphereRadius(250.f);
	ProximitySphere->SetCollisionProfileName(TEXT("Trigger"));
}

void AEnemySearcher::BeginPlay()
{
	Super::BeginPlay();

	if (GetMesh())
	{
		const int32 MaterialCount = GetMesh()->GetNumMaterials();
		for (int32 Index = 0; Index < MaterialCount; ++Index)
		{
			if (UMaterialInstanceDynamic* Dynamic = GetMesh()->CreateAndSetMaterialInstanceDynamic(Index))
			{
				BodyMaterials.Add(Dynamic);
			}
		}
	}

	if (ProximitySphere)
	{
		ProximitySphere->SetHiddenInGame(!bShowDebug);
	}

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
	if (!Lighting) { return; }

	const float Light = Lighting->GetLightIntensityAtLocation(GetActorLocation());

	const float Range = Visibility_FullLight - Visibility_MinLight;
	const float TargetExposure = Range > 0.f
		? FMath::Clamp((Light - Visibility_MinLight) / Range, 0.f, 1.f)
		: 0.f;

	const float Now = GetWorld()->GetTimeSeconds();
	const float DeltaTime = Now - LastVisibilityTime;
	LastVisibilityTime = Now;

	if (TargetExposure >= ShownExposure || Visibility_FadeOutTime <= 0.f)
	{
		ShownExposure = TargetExposure;
	}
	else
	{
		ShownExposure = FMath::Max(TargetExposure, ShownExposure - DeltaTime / Visibility_FadeOutTime);
	}

	for (const TObjectPtr<UMaterialInstanceDynamic>& Material : BodyMaterials)
	{
		if (Material)
		{
			Material->SetScalarParameterValue(TEXT("Exposure"), ShownExposure);
		}
	}
}