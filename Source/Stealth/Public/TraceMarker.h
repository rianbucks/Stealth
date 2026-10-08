#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TraceMarker.generated.h"

UENUM(BlueprintType)
enum class ETraceType : uint8
{
	Walk,
	Sprint,
	Blood,
	Noise
};

UCLASS()
class STEALTH_API ATraceMarker : public AActor
{
	GENERATED_BODY()

public:
	ATraceMarker();

	void Init(ETraceType InType, const FVector& InStart, float InLifetime, bool bDrawDebug);

	ETraceType GetTraceType() const { return Type; }
	FVector GetStart() const { return Start; }
	float GetSpawnTime() const { return SpawnTime; }

protected:
	UPROPERTY(VisibleInstanceOnly, Category = "Trace")
	ETraceType Type = ETraceType::Walk;

	UPROPERTY(VisibleInstanceOnly, Category = "Trace")
	FVector Start = FVector::ZeroVector;

	UPROPERTY(VisibleInstanceOnly, Category = "Trace")
	float Lifetime = 0.f;

	float SpawnTime = 0.f;
};