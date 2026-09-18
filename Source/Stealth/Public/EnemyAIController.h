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

		/** Called by the engine when a move request finishes. */
		virtual void OnMoveCompleted(FAIRequestID RequestID,
			const FPathFollowingResult& Result) override;

		/** Issues a move command toward the current patrol point. */
		void MoveToCurrentPoint();

		/** Advances to the next point and moves. Called after the wait timer. */
		void GoToNextPoint();

		/** Index into the pawn's PatrolPoints array. */
		int32 CurrentIndex = 0;

		FTimerHandle WaitTimer;

};
