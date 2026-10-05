#include "CoreMinimal.h"
#include "AIController.h"
#include "EnemyAIController.generated.h"

class UAIPerceptionComponent;
class UAISenseConfig_Sight;
struct FAIStimulus;

UCLASS()
class STEALTH_API AEnemyAIController : public AAIController
{
	GENERATED_BODY()
	
	protected:
		virtual void BeginPlay() override;
		virtual void OnPossess(APawn* InPawn) override;

		virtual void OnMoveCompleted(FAIRequestID RequestID,
			const FPathFollowingResult& Result) override;

		void MoveToCurrentPoint();

		void GoToNextPoint();

		int32 CurrentIndex = 0;

		FTimerHandle WaitTimer;

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

	public:
		AEnemyAIController();
};
