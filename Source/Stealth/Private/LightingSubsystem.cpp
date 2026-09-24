#include "LightingSubsystem.h"
#include "EngineUtils.h"
#include "Engine/PointLight.h"
#include "Components/PointLightComponent.h"
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

	return FMath::Clamp(Total, 0.f, 1.f);
}