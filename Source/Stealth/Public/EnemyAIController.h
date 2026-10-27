#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "EnemyAIController.generated.h"

class UAIPerceptionComponent;
class UAISenseConfig_Sight;
class UPrimitiveComponent;
class AStealthCharacter;
struct FAIStimulus;

UCLASS()
class STEALTH_API AEnemyAIController : public AAIController
{
	GENERATED_BODY()
	
	protected:
		virtual void BeginPlay() override;
		virtual void OnPossess(APawn* InPawn) override;

		UPROPERTY(VisibleAnywhere, Category = "Perception")
		TObjectPtr<UAIPerceptionComponent> AIPerception;

		UPROPERTY()
		TObjectPtr<UAISenseConfig_Sight> SightConfig;

		UPROPERTY(EditDefaultsOnly, Category = "Perception")
		float Sight_Radius = 1500.f;

		UPROPERTY(EditDefaultsOnly, Category = "Perception")
		float Sight_LoseRadius = 1600.f;

		UPROPERTY(EditDefaultsOnly, Category = "Perception")
		float Sight_HalfAngle = 50.f;

		UPROPERTY(EditDefaultsOnly, Category = "Perception")
		float Sight_MaxAge = 5.f;

		UFUNCTION()
		void OnPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);

		UPROPERTY()
		TObjectPtr<AStealthCharacter> SightTarget;

		UPROPERTY()
		TObjectPtr<AStealthCharacter> NearbyPlayer;

		UPROPERTY(VisibleInstanceOnly, Category = "Perception")
		bool bPlayerDetected = false;

		UPROPERTY(VisibleInstanceOnly, Category = "Perception")
		bool bSuspicious = false;

		UPROPERTY(VisibleInstanceOnly, Category = "Perception")
		bool bNoiseHeard = false;

		FTimerHandle NoiseCheckTimer;

		void CheckNoise();

		void RequestInvestigation(const FVector& Location);

		FTimerHandle DetectionTimer;

		static constexpr float DetectionInterval = 0.1f;

		void RefreshWatch();
		void UpdateDetection();

		UFUNCTION()
		void OnProximityBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
			UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

		UFUNCTION()
		void OnProximityEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
			UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

		UPROPERTY(VisibleInstanceOnly, Category = "Perception")
		bool bLightNoticed = false;

		UPROPERTY(VisibleInstanceOnly, Category = "Perception")
		FVector NoticedLocation = FVector::ZeroVector;

		FTimerHandle LightCheckTimer;

		void CheckLight();
		bool CanSeeLocation(const FVector& Target, float HalfAngle) const;

	public:
		AEnemyAIController();
};
