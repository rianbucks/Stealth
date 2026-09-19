#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "EnemySearcher.generated.h"

UCLASS()
class STEALTH_API AEnemySearcher : public ACharacter
{
	GENERATED_BODY()

public:
	AEnemySearcher();

protected:
	virtual void BeginPlay() override;

	// Patrolling speed
	UPROPERTY(EditDefaultsOnly, Category = "Movement")
	float Speed_Patrol = 110.f;

	// Tracing speed
	UPROPERTY(EditDefaultsOnly, Category = "Movement")
	float Speed_Investigate = 180.f;

	// Chasing speed
	UPROPERTY(EditDefaultsOnly, Category = "Movement")
	float Speed_Chase = 350.f;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

public:
	UPROPERTY(EditInstanceOnly, Category = "Patrol")
	TArray<TObjectPtr<AActor>> PatrolPoints;

	UPROPERTY(EditDefaultsOnly, Category = "Patrol")
	float PatrolWaitTime = 3.0f;

	UPROPERTY(EditAnywhere, Category = "Debug")
	bool bShowDebug = true;

};
