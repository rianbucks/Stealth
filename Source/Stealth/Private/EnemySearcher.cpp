// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemySearcher.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

// Sets default values
AEnemySearcher::AEnemySearcher()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Same size as the player so the NavMesh agent settings apply to both.
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// Body faces movement direction, not controller rotation.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 300.0f, 0.0f);
	GetCharacterMovement()->MaxWalkSpeed = Speed_Patrol;
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

