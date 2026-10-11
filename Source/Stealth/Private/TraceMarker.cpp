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

void ATraceMarker::InitNoise(float InRadius, float InLifetime, bool bDrawDebug)
{
	Init(ETraceType::Noise, GetActorLocation(), InLifetime, false);
	Radius = InRadius;

	if (bDrawDebug && Radius > 0.f)
	{
		DrawDebugCircle(GetWorld(), GetActorLocation(), Radius, 48, GetTraceColor(Type),
			false, Lifetime, 0, 2.f, FVector(1.f, 0.f, 0.f), FVector(0.f, 1.f, 0.f), false);
	}
}