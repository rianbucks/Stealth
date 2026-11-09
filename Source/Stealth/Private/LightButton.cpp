#include "LightButton.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "LightingSubsystem.h"
#include "TraceSubsystem.h"
#include "DrawDebugHelpers.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "Stealth/StealthCharacter.h"

ALightButton::ALightButton()
{
	PrimaryActorTick.bCanEverTick = false;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = Root;

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Mesh->SetupAttachment(Root);
	Mesh->SetCollisionProfileName(TEXT("BlockAllDynamic"));

	LightSample = CreateDefaultSubobject<USceneComponent>(TEXT("LightSample"));
	LightSample->SetupAttachment(Root);
	LightSample->SetRelativeLocation(FVector(0.f, 0.f, 30.f));
}

void ALightButton::BeginPlay()
{
	Super::BeginPlay();

	ButtonMaterial = Mesh->CreateAndSetMaterialInstanceDynamic(0);
	UpdateColor();

	LastCheckTime = GetWorld()->GetTimeSeconds();
	GetWorldTimerManager().SetTimer(CheckTimer, this, &ALightButton::CheckLight, CheckInterval, true);
}

void ALightButton::CheckLight()
{
	const AStealthCharacter* Player = Cast<AStealthCharacter>(UGameplayStatics::GetPlayerCharacter(this, 0));
	if (!Player || bActivated) { return; }

	const float Now = GetWorld()->GetTimeSeconds();
	const float DeltaTime = FMath::Min(Now - LastCheckTime, CheckInterval * 2.f);
	LastCheckTime = Now;

	const FVector Point = LightSample->GetComponentLocation();
	const float Light = Player->IsFlashlightOn() ? Player->GetFlashlightLightAt(Point) : 0.f;

	if (Light >= Button_LightThreshold)
	{
		LastLitTime = Now;
		Progress = FMath::Min(Progress + DeltaTime / FMath::Max(Button_RequiredExposure, 0.01f), 1.f);
	}
	else if (Now - LastLitTime >= Button_DecayDelay)
	{
		Progress = FMath::Max(Progress - Button_DecayRate * DeltaTime, 0.f);
	}

	UpdateColor();

	if (bShowDebug)
	{
		DrawDebugString(GetWorld(), Point + FVector(0.f, 0.f, 30.f),
			FString::Printf(TEXT("%.2f  %d%%"), Light, FMath::RoundToInt(Progress * 100.f)),
			nullptr, FColor::White, CheckInterval * 1.5f, true, 1.2f);
	}

	if (Progress >= 1.f)
	{
		Activate();
	}
}

void ALightButton::Activate()
{
	bActivated = true;
	Progress = 1.f;
	UpdateColor();
	GetWorldTimerManager().ClearTimer(CheckTimer);

	UE_LOG(LogTemp, Warning, TEXT("%s: BUTTON ON"), *GetActorNameOrLabel());

	if (Button_ActivateNoiseRadius > 0.f)
	{
		if (UTraceSubsystem* Traces = GetWorld()->GetSubsystem<UTraceSubsystem>())
		{
			Traces->SpawnNoise(this, GetActorLocation(), Button_ActivateNoiseRadius, 2.5f, bShowDebug);
		}
	}

	OnActivated.Broadcast(this);
}

void ALightButton::UpdateColor()
{
	if (!ButtonMaterial) { return; }

	const float Shown = bButton_ShowProgress ? Progress : 0.f;
	const FLinearColor Color = bActivated
		? FLinearColor::Green
		: FLinearColor::LerpUsingHSV(FLinearColor::Red, FLinearColor::Yellow, Shown);

	ButtonMaterial->SetVectorParameterValue(TEXT("Color"), Color);
}