#include "LightingSubsystem.h"
#include "EngineUtils.h"
#include "Engine/PointLight.h"
#include "Components/PointLightComponent.h"

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

float ULightingSubsystem::GetLightIntensityAtLocation(const FVector& Location) const
{
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

		const float Distance = FVector::Dist(Light->GetActorLocation(), Location);

		if (Distance >= Radius)
		{
			continue;
		}

		Total += FMath::Pow(1.f - Distance / Radius, AttenuationExponent);
	}

	return FMath::Clamp(Total, 0.f, 1.f);
}