#include "ExitDoor.h"
#include "LightButton.h"
#include "Components/StaticMeshComponent.h"
#include "EngineUtils.h"

AExitDoor::AExitDoor()
{
	PrimaryActorTick.bCanEverTick = false;

	Door = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Door"));
	RootComponent = Door;
}

void AExitDoor::BeginPlay()
{
	Super::BeginPlay();

	for (TActorIterator<ALightButton> It(GetWorld()); It; ++It)
	{
		ALightButton* Button = *It;
		Buttons.Add(Button);
		Button->OnActivated.AddDynamic(this, &AExitDoor::HandleButtonActivated);
	}

	UE_LOG(LogTemp, Warning, TEXT("EXIT: %d buttons, need %d"), Buttons.Num(), RequiredButtons);
}

void AExitDoor::HandleButtonActivated(ALightButton* Button)
{
	++ActivatedCount;
	UE_LOG(LogTemp, Warning, TEXT("EXIT: %d / %d (%s)"), ActivatedCount, RequiredButtons, *GetNameSafe(Button));

	if (!bOpen && ActivatedCount >= RequiredButtons)
	{
		Open();
	}
}

void AExitDoor::Open()
{
	bOpen = true;
	Door->SetVisibility(false);
	Door->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	UE_LOG(LogTemp, Warning, TEXT("EXIT OPEN"));
}