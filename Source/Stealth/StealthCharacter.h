#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Logging/LogMacros.h"
#include "StealthCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UInputMappingContext;
class UInputAction;
class USpotLightComponent;
class UUserWidget;
struct FInputActionValue;

DECLARE_LOG_CATEGORY_EXTERN(LogTemplateCharacter, Log, All);

UENUM(BlueprintType)
enum class EMovementStance : uint8
{
	Crouch UMETA(DisplayName = "Crouch"),
	Walk   UMETA(DisplayName = "Walk"),
	Sprint UMETA(DisplayName = "Sprint")
};

UCLASS(config=Game)
class AStealthCharacter : public ACharacter
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FollowCamera;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputMappingContext* DefaultMappingContext;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* JumpAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* MoveAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* LookAction;

public:
	AStealthCharacter();

protected:
	// movement functions
	void Move(const FInputActionValue& Value);

	void Look(const FInputActionValue& Value);

	void OnCrouchToggle();

	void OnSprintStart();

	void OnSprintEnd();

	void SetStance(EMovementStance NewStance);

	void ApplyStanceSpeed();

	UPROPERTY(EditDefaultsOnly, Category = "Movement")
	float Speed_Crouch = 120.f;

	UPROPERTY(EditDefaultsOnly, Category = "Movement")
	float Speed_Walk = 200.f;

	UPROPERTY(EditDefaultsOnly, Category = "Movement")
	float Speed_Sprint = 400.f;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Movement")
	EMovementStance CurrentStance = EMovementStance::Walk;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input",
		meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> IA_Crouch;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input",
		meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> IA_Sprint;

	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<UInputAction> IA_Flashlight;

	// debug functions
	void UpdateIlluminanceCache();
	FTimerHandle IlluminanceCacheTimer;

	UPROPERTY(VisibleInstanceOnly, Category = "Flashlight")
	float CachedIlluminance = 0.f;

	UPROPERTY(EditAnywhere, Category = "Debug")
	bool bShowLightDebug = false;

	// HUD functions
	UPROPERTY(EditDefaultsOnly, Category = "HUD")
	TSubclassOf<UUserWidget> HUDWidgetClass;

	UPROPERTY()
	TObjectPtr<UUserWidget> HUDWidget;

	UPROPERTY(EditDefaultsOnly, Category = "HUD")
	float Concealment_FullExposure = 0.30f;

	UPROPERTY(VisibleAnywhere, Category = "Flashlight")
	TObjectPtr<USpotLightComponent> Flashlight;

	// flashlight functions
	void OnFlashlightStart();
	void OnFlashlightEnd();
	void UpdateBattery();

	UPROPERTY(VisibleInstanceOnly, Category = "Flashlight")
	float Battery = 5.f;

	UPROPERTY(EditDefaultsOnly, Category = "Flashlight")
	float Battery_Max = 5.f;

	UPROPERTY(EditDefaultsOnly, Category = "Flashlight")
	float Battery_DrainRate = 1.f;

	UPROPERTY(EditDefaultsOnly, Category = "Flashlight")
	float Battery_RechargeRate = 0.5f;

	UPROPERTY(VisibleInstanceOnly, Category = "Flashlight")
	bool bBatteryDepleted = false;

	UPROPERTY(EditDefaultsOnly, Category = "Flashlight")
	float Flashlight_SelfGlow = 0.35f;

	FTimerHandle BatteryTimer;

	static constexpr float BatteryUpdateInterval = 0.1f;

	// aiming functions
	void SetAimMode(bool bAiming);

protected:

	virtual void NotifyControllerChanged() override;

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	virtual void BeginPlay() override;

public:
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }

	float GetEffectiveIlluminance(bool bDrawDebug = false) const;

	UFUNCTION(BlueprintPure, Category = "HUD")
	float GetBatteryRatio() const { return Battery_Max > 0.f ? Battery / Battery_Max : 0.f; }

	UFUNCTION(BlueprintPure, Category = "HUD")
	float GetConcealmentRatio() const;

	UFUNCTION(BlueprintPure, Category = "HUD")
	bool IsBatteryDepleted() const { return bBatteryDepleted; }
};

