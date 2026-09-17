// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "EnemyAIController.generated.h"

/**
 * 
 */
UCLASS()
class STEALTH_API AEnemyAIController : public AAIController
{
	GENERATED_BODY()
	
	protected:
		virtual void BeginPlay() override;
		virtual void OnPossess(APawn* InPawn) override;

		/** Issues the first move command. Called on a short delay. */
		void StartFirstMove();

};
