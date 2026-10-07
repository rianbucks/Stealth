#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "EnemySearcher.generated.h"

class UMaterialInstanceDynamic;
class USphereComponent;

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

	void UpdateVisibility();

	FTimerHandle VisibilityTimer;

	UPROPERTY(EditDefaultsOnly, Category = "Visibility")
	float Visibility_MinLight = 0.08f;

	UPROPERTY(EditDefaultsOnly, Category = "Visibility")
	float Visibility_FullLight = 0.35f;

	UPROPERTY()
	TArray<TObjectPtr<UMaterialInstanceDynamic>> BodyMaterials; 

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

	UPROPERTY(VisibleAnywhere, Category = "Perception")
	TObjectPtr<USphereComponent> ProximitySphere;

	UPROPERTY(EditDefaultsOnly, Category = "Perception")
	float Proximity_CrouchRadius = 0;
};
