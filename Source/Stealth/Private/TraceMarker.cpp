#include "TraceMarker.h"
#include "DrawDebugHelpers.h"

static FColor GetTraceColor(ETraceType Type)
{
	switch (Type)
	{
	case ETraceType::Sprint: return FColor::Orange;
	case ETraceType::Blood:  return FColor::Red;
	case ETraceType::Noise:  return FColor::Cyan;
	default:                 return FColor::Yellow;
	}
}

ATraceMarker::ATraceMarker()
{
	PrimaryActorTick.bCanEverTick = false;
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
}

void ATraceMarker::Init(ETraceType InType, const FVector& InStart, float InLifetime, bool bDrawDebug)
{
	Type = InType;
	Start = InStart;
	Lifetime = InLifetime;
	SpawnTime = GetWorld()->GetTimeSeconds();
	SetLifeSpan(Lifetime);

	if (bDrawDebug)
	{
		DrawDebugLine(GetWorld(), Start, GetActorLocation(), GetTraceColor(Type),
			false, Lifetime, 0, 4.f);
	}
}