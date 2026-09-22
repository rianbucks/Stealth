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