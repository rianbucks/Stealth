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

private:
	UPROPERTY()
	TArray<TObjectPtr<APointLight>> CachedLights;
};