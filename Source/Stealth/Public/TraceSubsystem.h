#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "TraceMarker.h"
#include "TraceSubsystem.generated.h"

UCLASS()
class STEALTH_API UTraceSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	void SpawnTrail(AActor* Owner, ETraceType Type, const FVector& Start, const FVector& End,
		float Lifetime, bool bDrawDebug);

	void SpawnNoise(AActor* Owner, const FVector& Location, float Radius, float Lifetime, bool bDrawDebug);

	const TArray<TObjectPtr<ATraceMarker>>& GetMarkers() const { return Markers; }

protected:
	ATraceMarker* SpawnMarker(AActor* Owner, const FVector& Location);

	UPROPERTY()
	TArray<TObjectPtr<ATraceMarker>> Markers;

	static constexpr int32 MaxMarkers = 120;
};