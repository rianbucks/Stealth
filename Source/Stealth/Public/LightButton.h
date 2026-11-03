#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "LightButton.generated.h"

class UStaticMeshComponent;
class UMaterialInstanceDynamic;
class ALightButton;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnButtonActivated, ALightButton*, Button);

UCLASS()
class STEALTH_API ALightButton : public AActor
{
	GENERATED_BODY()

public:
	ALightButton();

	UPROPERTY(BlueprintAssignable, Category = "Button")
	FOnButtonActivated OnActivated;

	bool IsActivated() const { return bActivated; }
	float GetProgress() const { return Progress; }

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, Category = "Button")
	TObjectPtr<USceneComponent> Root;

	UPROPERTY(VisibleAnywhere, Category = "Button")
	TObjectPtr<UStaticMeshComponent> Mesh;

	UPROPERTY(VisibleAnywhere, Category = "Button")
	TObjectPtr<USceneComponent> LightSample;

	UPROPERTY(EditDefaultsOnly, Category = "Button")
	float Button_LightThreshold = 0.35f;

	UPROPERTY(EditDefaultsOnly, Category = "Button")
	float Button_RequiredExposure = 1.f;

	UPROPERTY(EditDefaultsOnly, Category = "Button")
	float Button_DecayDelay = 2.f;

	UPROPERTY(EditDefaultsOnly, Category = "Button")
	float Button_DecayRate = 0.5f;

	UPROPERTY(EditDefaultsOnly, Category = "Button")
	bool bButton_ShowProgress = true;

	UPROPERTY(EditDefaultsOnly, Category = "Button")
	float Button_ActivateNoiseRadius = 0.f;

	UPROPERTY(EditAnywhere, Category = "Debug")
	bool bShowDebug = true;

	UPROPERTY(VisibleInstanceOnly, Category = "Button")
	float Progress = 0.f;

	UPROPERTY(VisibleInstanceOnly, Category = "Button")
	bool bActivated = false;

	UPROPERTY()
	TObjectPtr<UMaterialInstanceDynamic> ButtonMaterial;

	float LastLitTime = -100.f;
	float LastCheckTime = 0.f;
	FTimerHandle CheckTimer;

	static constexpr float CheckInterval = 0.1f;

	void CheckLight();
	void Activate();
	void UpdateColor();
};