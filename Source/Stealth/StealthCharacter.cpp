#include "StealthCharacter.h"
#include "Engine/LocalPlayer.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
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