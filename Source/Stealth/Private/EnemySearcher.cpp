#include "EnemySearcher.h"
#include "EnemyAIController.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

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

// Called when the game starts or when spawned
void AEnemySearcher::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AEnemySearcher::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

