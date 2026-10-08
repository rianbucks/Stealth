#include "StealthCharacter.h"
#include "Engine/LocalPlayer.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SpotLightComponent.h"
#include "LightingSubsystem.h"
#include "TimerManager.h"
#include "TraceMarker.h"
#include "Blueprint/UserWidget.h"
#include "Engine/Engine.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Controller.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"

DEFINE_LOG_CATEGORY(LogTemplateCharacter);

AStealthCharacter::AStealthCharacter()
{
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
		
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	GetCharacterMovement()->bOrientRotationToMovement = true; 
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f); 

	GetCharacterMovement()->JumpZVelocity = 700.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = Speed_Walk;
	GetCharacterMovement()->MaxWalkSpeedCrouched = Speed_Crouch;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f; 
	CameraBoom->bUsePawnControlRotation = true;

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false; 

	GetCharacterMovement()->GetNavAgentPropertiesRef().bCanCrouch = true;

	// Flashlight
	Flashlight = CreateDefaultSubobject<USpotLightComponent>(TEXT("Flashlight"));
	Flashlight->SetupAttachment(GetCapsuleComponent());
	Flashlight->SetRelativeLocation(FVector(30.f, 0.f, 40.f));
	Flashlight->SetMobility(EComponentMobility::Movable);
	Flashlight->SetVisibility(false);

	Flashlight->AttenuationRadius = 1500.f;
	Flashlight->InnerConeAngle = 15.f;
	Flashlight->OuterConeAngle = 30.f;
}

void AStealthCharacter::NotifyControllerChanged()
{
	Super::NotifyControllerChanged();

	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}
}

void AStealthCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent)) {
		
		// Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		// Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AStealthCharacter::Move);

		// Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AStealthCharacter::Look);

		// Crouch
		if (IA_Crouch)
		{
			EnhancedInputComponent->BindAction(
				IA_Crouch, ETriggerEvent::Started,
				this, &AStealthCharacter::OnCrouchToggle);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("IA_Crouch is not assigned in BP"));
		}

		// Sprint
		if (IA_Sprint)
		{
			EnhancedInputComponent->BindAction(
				IA_Sprint, ETriggerEvent::Started,
				this, &AStealthCharacter::OnSprintStart);

			EnhancedInputComponent->BindAction(
				IA_Sprint, ETriggerEvent::Completed,
				this, &AStealthCharacter::OnSprintEnd);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("IA_Sprint is not assigned in BP"));
		}

		if (IA_Flashlight)
		{
			EnhancedInputComponent->BindAction(
				IA_Flashlight, ETriggerEvent::Started,
				this, &AStealthCharacter::OnFlashlightStart);

			EnhancedInputComponent->BindAction(
				IA_Flashlight, ETriggerEvent::Completed,
				this, &AStealthCharacter::OnFlashlightEnd);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("IA_Flashlight is not assigned in BP"));
		}
	}
	else
	{
		UE_LOG(LogTemplateCharacter, Error, TEXT("'%s' Failed to find an Enhanced Input component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}
}

void AStealthCharacter::Move(const FInputActionValue& Value)
{
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		AddMovementInput(ForwardDirection, MovementVector.Y);
		AddMovementInput(RightDirection, MovementVector.X);
	}
}

void AStealthCharacter::Look(const FInputActionValue& Value)
{
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}

void AStealthCharacter::BeginPlay()
{
	Super::BeginPlay();

	GetWorldTimerManager().SetTimer(IlluminanceCacheTimer, this,
		&AStealthCharacter::UpdateIlluminanceCache, BatteryUpdateInterval, true);

	GetWorldTimerManager().SetTimer(TrailTimer, this,
		&AStealthCharacter::UpdateTrail, TrailCheckInterval, true);

	if (HUDWidgetClass)
	{
		HUDWidget = CreateWidget<UUserWidget>(GetWorld(), HUDWidgetClass);
		if (HUDWidget)
		{
			HUDWidget->AddToViewport();
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("HUDWidgetClass is not assigned in BP"));
	}

	if (ULightingSubsystem* Lighting = GetWorld()->GetSubsystem<ULightingSubsystem>())
	{
		Lighting->RegisterSpotLight(Flashlight);
	}

	GetWorldTimerManager().SetTimer(BatteryTimer, this,
		&AStealthCharacter::UpdateBattery, BatteryUpdateInterval, true);

	SetStance(EMovementStance::Sprint);
	SetStance(EMovementStance::Walk);
	SetStance(EMovementStance::Walk);
}

void AStealthCharacter::ApplyStanceSpeed()
{
	UCharacterMovementComponent* Move = GetCharacterMovement();
	if (!Move) { return; }

	switch (CurrentStance)
	{
	case EMovementStance::Crouch:
		break;

	case EMovementStance::Walk:
		Move->MaxWalkSpeed = Speed_Walk;
		break;

	case EMovementStance::Sprint:
		Move->MaxWalkSpeed = Speed_Sprint;
		break;
	}
}

void AStealthCharacter::SetStance(EMovementStance NewStance)
{
	if (CurrentStance == NewStance) { return; }

	CurrentStance = NewStance;
	
	if (NewStance == EMovementStance::Crouch)
	{
		Crouch();
	}
	else if (bIsCrouched)
	{
		UnCrouch();
	}

	ApplyStanceSpeed();

	UE_LOG(LogTemp, Warning, TEXT("Stance -> %s"),
		*UEnum::GetValueAsString(CurrentStance));
}

void AStealthCharacter::OnCrouchToggle()
{
	SetStance(CurrentStance == EMovementStance::Crouch
		? EMovementStance::Walk
		: EMovementStance::Crouch);
}

void AStealthCharacter::OnSprintStart()
{
	SetStance(EMovementStance::Sprint);
}

void AStealthCharacter::OnSprintEnd()
{
	if (CurrentStance == EMovementStance::Sprint)
	{
		SetStance(EMovementStance::Walk);
	}
}

void AStealthCharacter::UpdateIlluminanceCache()
{
	CachedIlluminance = GetEffectiveIlluminance(bShowLightDebug);

	if (bShowLightDebug && GEngine)
	{
		GEngine->AddOnScreenDebugMessage(
			1, 0.2f, FColor::Yellow,
			FString::Printf(TEXT("Light: %.2f  Conceal: %.0f%%  Battery: %.1f"),
				CachedIlluminance, GetConcealmentRatio() * 100.f, Battery));
	}
}

float AStealthCharacter::GetConcealmentRatio() const
{
	if (Concealment_FullExposure <= 0.f) { return 0.f; }
	return FMath::Clamp(1.f - CachedIlluminance / Concealment_FullExposure, 0.f, 1.f);
}

void AStealthCharacter::OnFlashlightStart()
{
	if (!Flashlight)
	{
		return;
	}

	if (bBatteryDepleted)
	{
		UE_LOG(LogTemp, Warning, TEXT("Flashlight: recharging (%.1f / %.0f)"),
			Battery, Battery_Max);
		return;
	}

	Flashlight->SetVisibility(true);
	SetAimMode(true);

	UE_LOG(LogTemp, Warning, TEXT("Flashlight -> ON (battery %.1f)"), Battery);
}

void AStealthCharacter::OnFlashlightEnd()
{
	if (!Flashlight)
	{
		return;
	}

	Flashlight->SetVisibility(false);
	SetAimMode(false);

	UE_LOG(LogTemp, Warning, TEXT("Flashlight -> OFF (battery %.1f)"), Battery);
}

void AStealthCharacter::UpdateBattery()
{
	if (!Flashlight)
	{
		return;
	}

	if (bBatteryDepleted)
	{
		Battery = FMath::Min(Battery + Battery_RechargeRate * BatteryUpdateInterval,
			Battery_Max);

		if (Battery >= Battery_Max)
		{
			Battery = Battery_Max;
			bBatteryDepleted = false;
			UE_LOG(LogTemp, Warning, TEXT("Flashlight: recharged"));
		}
		return;
	}

	if (Flashlight->IsVisible())
	{
		Battery -= Battery_DrainRate * BatteryUpdateInterval;

		if (Battery <= 0.f)
		{
			Battery = 0.f;
			bBatteryDepleted = true;
			Flashlight->SetVisibility(false);
			SetAimMode(false);

			UE_LOG(LogTemp, Warning, TEXT("Flashlight -> OFF (depleted)"));
		}
	}
}

float AStealthCharacter::GetEffectiveIlluminance(bool bDrawDebug) const
{
	const ULightingSubsystem* Lighting = GetWorld()->GetSubsystem<ULightingSubsystem>();

	float Value = Lighting
		? Lighting->GetLightIntensityAtLocation(GetActorLocation(), bDrawDebug)
		: 0.f;

	if (Flashlight && Flashlight->IsVisible())
	{
		Value += Flashlight_SelfGlow;
	}

	return FMath::Clamp(Value, 0.f, 1.f);
}

void AStealthCharacter::SetAimMode(bool bAiming)
{
	// Face the camera while aiming, face movement direction otherwise
	bUseControllerRotationYaw = bAiming;
	GetCharacterMovement()->bOrientRotationToMovement = !bAiming;
}

bool AStealthCharacter::IsFlashlightOn() const
{
	return Flashlight && Flashlight->IsVisible();
}

float AStealthCharacter::GetFlashlightLightAt(const FVector& Location) const
{
	const ULightingSubsystem* Lighting = GetWorld()->GetSubsystem<ULightingSubsystem>();
	return Lighting ? Lighting->GetSpotLightIntensity(Flashlight, Location) : 0.f;
}

bool AStealthCharacter::GetFlashlightLitPoint(FVector& OutPoint) const
{
	if (!IsFlashlightOn()) { return false; }

	FRotator Direction = Flashlight->GetComponentRotation();
	Direction.Pitch -= Flashlight->InnerConeAngle;

	const FVector Start = Flashlight->GetComponentLocation();
	const FVector End = Start + Direction.Vector() * Flashlight->AttenuationRadius;

	FHitResult Hit;
	if (!GetWorld()->LineTraceSingleByObjectType(Hit, Start, End, FCollisionObjectQueryParams(ECC_WorldStatic)))
	{
		return false;
	}

	OutPoint = Hit.ImpactPoint + Hit.ImpactNormal * 10.f;
	return true;
}

void AStealthCharacter::UpdateTrail()
{
	const bool bLeavesTrail = CurrentStance != EMovementStance::Crouch
		&& GetCharacterMovement()->IsMovingOnGround();

	if (!bLeavesTrail)
	{
		bTrailActive = false;
		return;
	}

	const float HalfHeight = GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
	const FVector Foot = GetActorLocation() - FVector(0.f, 0.f, HalfHeight - 5.f);

	if (!bTrailActive)
	{
		TrailPoint = Foot;
		bTrailActive = true;
		return;
	}

	if (FVector::Dist2D(Foot, TrailPoint) < Trail_SegmentLength) { return; }

	const bool bSprinting = CurrentStance == EMovementStance::Sprint;

	FActorSpawnParameters Params;
	Params.Owner = this;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	if (ATraceMarker* Marker = GetWorld()->SpawnActor<ATraceMarker>(Foot, FRotator::ZeroRotator, Params))
	{
		Marker->Init(bSprinting ? ETraceType::Sprint : ETraceType::Walk, TrailPoint,
			bSprinting ? Trail_SprintLifetime : Trail_WalkLifetime, bShowTraceDebug);
	}

	TrailPoint = Foot;
}