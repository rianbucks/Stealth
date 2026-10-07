#include "LightingSubsystem.h"
#include "EngineUtils.h"
#include "Engine/PointLight.h"
#include "Components/PointLightComponent.h"
#include "Components/SpotLightComponent.h"
#include "DrawDebugHelpers.h"

void ULightingSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
	Super::OnWorldBeginPlay(InWorld);

	CachedLights.Reset();

	for (TActorIterator<APointLight> It(&InWorld); It; ++It)
	{
		APointLight* Light = *It;
		CachedLights.Add(Light);

		UPointLightComponent* Comp = Cast<UPointLightComponent>(Light->GetLightComponent());
		if (Comp)
		{
			UE_LOG(LogTemp, Warning, TEXT("  %s  loc=%s  radius=%.0f  intensity=%.1f"),
				*Light->GetActorNameOrLabel(),
				*Light->GetActorLocation().ToString(),
				Comp->AttenuationRadius,
				Comp->Intensity);
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("LightingSubsystem: cached %d point lights"),
		CachedLights.Num());
}

float ULightingSubsystem::GetLightIntensityAtLocation(const FVector& Location, bool bDrawDebug) const
{
	const UWorld* World = GetWorld();
	if (!World)
	{
		return 0.f;
	}

	FCollisionObjectQueryParams ObjectParams;
	ObjectParams.AddObjectTypesToQuery(ECC_WorldStatic);

	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(LightOcclusion), false);

	float Total = 0.f;

	for (const TObjectPtr<APointLight>& Light : CachedLights)
	{
		if (!IsValid(Light))
		{
			continue;
		}

		const UPointLightComponent* Comp =
			Cast<UPointLightComponent>(Light->GetLightComponent());
		if (!Comp || !Comp->IsVisible())
		{
			continue;
		}

		const float Radius = Comp->AttenuationRadius;
		if (Radius <= 0.f)
		{
			continue;
		}

		const FVector LightLocation = Light->GetActorLocation();
		const float Distance = FVector::Dist(LightLocation, Location);

		if (Distance >= Radius)
		{
			continue;
		}

		FHitResult Hit;
		const bool bBlocked = World->LineTraceSingleByObjectType(
			Hit, LightLocation, Location, ObjectParams, QueryParams);

		if (bDrawDebug)
		{
			DrawDebugLine(World, LightLocation, Location,
				bBlocked ? FColor::Red : FColor::Green,
				false, 0.15f, 0, 2.f);
		}

		if (bBlocked)
		{
			continue;
		}

		Total += FMath::Pow(1.f - Distance / Radius, AttenuationExponent);
	}

	for (const TObjectPtr<USpotLightComponent>& Spot : SpotLights)
	{
		Total += GetSpotLightIntensity(Spot, Location, bDrawDebug);
	}

	return FMath::Clamp(Total, 0.f, 1.f);
}

void ULightingSubsystem::RegisterSpotLight(USpotLightComponent* Spot)
{
	if (Spot)
	{
		SpotLights.AddUnique(Spot);
	}
}

float ULightingSubsystem::GetSpotLightIntensity(const USpotLightComponent* Spot, const FVector& Location, bool bDrawDebug) const
{
	const UWorld* World = GetWorld();
	if (!World || !IsValid(Spot) || !Spot->IsVisible()) { return 0.f; }

	const float Radius = Spot->AttenuationRadius;
	if (Radius <= 0.f) { return 0.f; }

	const FVector LightLocation = Spot->GetComponentLocation();
	const FVector ToPoint = Location - LightLocation;
	const float Distance = ToPoint.Size();
	if (Distance >= Radius || Distance < 1.f) { return 0.f; }

	const float CosAngle = FVector::DotProduct(Spot->GetForwardVector(), ToPoint / Distance);
	const float CosOuter = FMath::Cos(FMath::DegreesToRadians(Spot->OuterConeAngle));
	if (CosAngle <= CosOuter) { return 0.f; }

	const float CosInner = FMath::Cos(FMath::DegreesToRadians(Spot->InnerConeAngle));
	const float Cone = CosInner > CosOuter
		? FMath::Clamp((CosAngle - CosOuter) / (CosInner - CosOuter), 0.f, 1.f)
		: 1.f;

	FHitResult Hit;
	const bool bBlocked = World->LineTraceSingleByObjectType(
		Hit, LightLocation, Location, FCollisionObjectQueryParams(ECC_WorldStatic));

	if (bDrawDebug)
	{
		DrawDebugLine(World, LightLocation, Location,
			bBlocked ? FColor::Red : FColor::Cyan, false, 0.15f, 0, 2.f);
	}

	if (bBlocked) { return 0.f; }

	return FMath::Pow(1.f - Distance / Radius, AttenuationExponent) * Cone;
}