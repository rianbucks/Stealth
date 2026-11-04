#include "ExitZone.h"
#include "Components/BoxComponent.h"
#include "Components/CapsuleComponent.h"
#include "Stealth/StealthCharacter.h"
#include "Stealth/StealthGameMode.h"

AExitZone::AExitZone()
{
	PrimaryActorTick.bCanEverTick = false;

	Zone = CreateDefaultSubobject<UBoxComponent>(TEXT("Zone"));
	RootComponent = Zone;
	Zone->SetBoxExtent(FVector(150.f, 200.f, 175.f));
	Zone->SetCollisionProfileName(TEXT("Trigger"));
	Zone->SetHiddenInGame(false);
	Zone->ShapeColor = FColor::Green;
}

void AExitZone::BeginPlay()
{
	Super::BeginPlay();

	Zone->OnComponentBeginOverlap.AddDynamic(this, &AExitZone::OnZoneBegin);
}

void AExitZone::OnZoneBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	const AStealthCharacter* Player = Cast<AStealthCharacter>(OtherActor);
	if (!Player || OtherComp != Player->GetCapsuleComponent()) { return; }

	if (AStealthGameMode* GameMode = GetWorld()->GetAuthGameMode<AStealthGameMode>())
	{
		GameMode->ReportPlayerEscaped();
	}
}