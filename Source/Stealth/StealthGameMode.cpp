#include "StealthGameMode.h"
#include "StealthCharacter.h"
#include "UObject/ConstructorHelpers.h"
#include "ExitDoor.h"
#include "AIController.h"
#include "BrainComponent.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"
#include "EngineUtils.h"
#include "TimerManager.h"

AStealthGameMode::AStealthGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}

void AStealthGameMode::BeginPlay()
{
	Super::BeginPlay();

	StartTime = GetWorld()->GetTimeSeconds();

	for (TActorIterator<AExitDoor> It(GetWorld()); It; ++It)
	{
		ExitDoor = *It;
		break;
	}

	if (Match_TimeLimit > 0.f)
	{
		GetWorldTimerManager().SetTimer(TimeoutTimer, this, &AStealthGameMode::OnTimeout, Match_TimeLimit, false);
	}
}

float AStealthGameMode::GetRemainingTime() const
{
	if (Match_TimeLimit <= 0.f) { return 0.f; }

	const float Now = State == EMatchState::Playing ? GetWorld()->GetTimeSeconds() : EndTime;
	return FMath::Max(StartTime + Match_TimeLimit - Now, 0.f);
}

FText AStealthGameMode::GetTimerText() const
{
	const int32 Seconds = FMath::CeilToInt(GetRemainingTime());
	return FText::FromString(FString::Printf(TEXT("%02d:%02d"), Seconds / 60, Seconds % 60));
}

bool AStealthGameMode::ShouldShowTimer() const
{
	if (State != EMatchState::Playing || Match_TimeLimit <= 0.f) { return false; }
	return Match_TimerShowLast <= 0.f || GetRemainingTime() <= Match_TimerShowLast;
}

FText AStealthGameMode::GetObjectiveText() const
{
	if (!ExitDoor) { return FText::GetEmpty(); }
	if (ExitDoor->IsOpen()) { return FText::FromString(TEXT("EXIT OPEN")); }

	return FText::FromString(FString::Printf(TEXT("BUTTONS %d / %d"),
		ExitDoor->GetActivatedCount(), ExitDoor->GetRequiredButtons()));
}

FText AStealthGameMode::GetResultText() const
{
	const TCHAR* Title = State == EMatchState::Escaped ? TEXT("ESCAPED")
		: (State == EMatchState::Caught ? TEXT("CAUGHT") : TEXT("TIME UP"));

	const int32 Elapsed = FMath::FloorToInt(EndTime - StartTime);
	const AStealthCharacter* Player = Cast<AStealthCharacter>(UGameplayStatics::GetPlayerCharacter(this, 0));

	return FText::FromString(FString::Printf(TEXT("%s\nTIME %02d:%02d    BUTTONS %d / %d    HITS %d"),
		Title, Elapsed / 60, Elapsed % 60,
		ExitDoor ? ExitDoor->GetActivatedCount() : 0,
		ExitDoor ? ExitDoor->GetTotalButtons() : 0,
		Player ? Player->HitCount : 0));
}

void AStealthGameMode::ReportPlayerCaught()
{
	EndMatch(EMatchState::Caught);
}

void AStealthGameMode::ReportPlayerEscaped()
{
	EndMatch(EMatchState::Escaped);
}

void AStealthGameMode::OnTimeout()
{
	EndMatch(EMatchState::TimeUp);
}

void AStealthGameMode::EndMatch(EMatchState Result)
{
	if (State != EMatchState::Playing) { return; }

	State = Result;
	EndTime = GetWorld()->GetTimeSeconds();
	GetWorldTimerManager().ClearTimer(TimeoutTimer);

	UE_LOG(LogTemp, Warning, TEXT("MATCH OVER: %s"), *UEnum::GetValueAsString(Result));

	for (TActorIterator<AAIController> It(GetWorld()); It; ++It)
	{
		if (UBrainComponent* Brain = It->GetBrainComponent())
		{
			Brain->StopLogic(TEXT("MatchOver"));
		}
		It->StopMovement();
	}

	if (APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0))
	{
		if (APawn* Pawn = PC->GetPawn())
		{
			Pawn->DisableInput(PC);
		}
	}

	if (ResultWidgetClass)
	{
		if (UUserWidget* ResultWidget = CreateWidget<UUserWidget>(GetWorld(), ResultWidgetClass))
		{
			ResultWidget->AddToViewport(10);
		}
	}

	GetWorldTimerManager().SetTimer(RestartTimer, this, &AStealthGameMode::RestartLevel, Match_RestartDelay, false);
}

void AStealthGameMode::RestartLevel()
{
	UGameplayStatics::OpenLevel(this, FName(*UGameplayStatics::GetCurrentLevelName(this)));
}