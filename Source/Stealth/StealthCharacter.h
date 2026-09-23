#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Logging/LogMacros.h"
#include "StealthCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UInputMappingContext;
class UInputAction;
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

	// Temporary debug. Remove after W4.
	void UpdateLightDebug();

	FTimerHandle LightDebugTimer;

	

protected:

	virtual void NotifyControllerChanged() override;

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	virtual void BeginPlay() override;

public:
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }
};

