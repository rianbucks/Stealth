// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "EnemySearcher.generated.h"

UCLASS()
class STEALTH_API AEnemySearcher : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AEnemySearcher();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	/** Patrolling speed*/
	UPROPERTY(EditDefaultsOnly, Category = "Movement")
	float Speed_Patrol = 110.f;

	/** Tracing speed */
	UPROPERTY(EditDefaultsOnly, Category = "Movement")
	float Speed_Investigate = 180.f;

	/** Chasing speed */
	UPROPERTY(EditDefaultsOnly, Category = "Movement")
	float Speed_Chase = 350.f;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
