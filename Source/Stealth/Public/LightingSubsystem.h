#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "LightingSubsystem.generated.h"

class APointLight;

UCLASS()
class STEALTH_API ULightingSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void OnWorldBeginPlay(UWorld& InWorld) override;

	int32 GetCachedLightCount() const { return CachedLights.Num(); }

	float GetLightIntensityAtLocation(const FVector& Location, bool bDrawDebug = false) const;

private:
	float AttenuationExponent = 2.f;

	UPROPERTY()
	TArray<TObjectPtr<APointLight>> CachedLights;
};