#include "TraceSubsystem.h"
#include "Engine/World.h"

ATraceMarker* UTraceSubsystem::SpawnMarker(AActor* Owner, const FVector& Location)
{
	UWorld* World = GetWorld();
	if (!World) { return nullptr; }

	Markers.RemoveAll([](const TObjectPtr<ATraceMarker>& Marker) { return !IsValid(Marker); });

	while (Markers.Num() >= MaxMarkers)
	{
		Markers[0]->Destroy();
		Markers.RemoveAt(0);
		UE_LOG(LogTemp, Warning, TEXT("Trace cap %d reached: oldest removed"), MaxMarkers);
	}

	FActorSpawnParameters Params;
	Params.Owner = Owner;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	ATraceMarker* Marker = World->SpawnActor<ATraceMarker>(Location, FRotator::ZeroRotator, Params);
	if (Marker)
	{
		Markers.Add(Marker);
	}
	return Marker;
}

void UTraceSubsystem::SpawnTrail(AActor* Owner, ETraceType Type, const FVector& Start, const FVector& End,
	float Lifetime, bool bDrawDebug)
{
	if (ATraceMarker* Marker = SpawnMarker(Owner, End))
	{
		Marker->Init(Type, Start, Lifetime, bDrawDebug);
	}
}

void UTraceSubsystem::SpawnNoise(AActor* Owner, const FVector& Location, float Radius, float Lifetime, bool bDrawDebug)
{
	if (ATraceMarker* Marker = SpawnMarker(Owner, Location))
	{
		Marker->InitNoise(Radius, Lifetime, bDrawDebug);
	}
}