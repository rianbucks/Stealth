#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "StealthGameMode.generated.h"

class UUserWidget;
class AExitDoor;

UENUM()
enum class EMatchState : uint8
{
	Playing,
	Escaped,
	Caught,
	TimeUp
};

UCLASS(minimalapi)

class AStealthGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AStealthGameMode();

	void ReportPlayerCaught();
	void ReportPlayerEscaped();

	UFUNCTION(BlueprintPure, Category = "Match")
	FText GetTimerText() const;

	UFUNCTION(BlueprintPure, Category = "Match")
	bool ShouldShowTimer() const;

	UFUNCTION(BlueprintPure, Category = "Match")
	FText GetObjectiveText() const;

	UFUNCTION(BlueprintPure, Category = "Match")
	FText GetResultText() const;

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditDefaultsOnly, Category = "Match")
	float Match_TimeLimit = 600.f;

	UPROPERTY(EditDefaultsOnly, Category = "Match")
	float Match_TimerShowLast = 0.f;

	UPROPERTY(EditDefaultsOnly, Category = "Match")
	float Match_RestartDelay = 5.f;

	UPROPERTY(EditDefaultsOnly, Category = "Match")
	TSubclassOf<UUserWidget> ResultWidgetClass;

	UPROPERTY(VisibleInstanceOnly, Category = "Match")
	EMatchState State = EMatchState::Playing;

	UPROPERTY()
	TObjectPtr<AExitDoor> ExitDoor;

	float StartTime = 0.f;
	float EndTime = 0.f;

	FTimerHandle TimeoutTimer;
	FTimerHandle RestartTimer;

	float GetRemainingTime() const;
	void OnTimeout();
	void EndMatch(EMatchState Result);
	void RestartLevel();
};



