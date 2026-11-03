#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ExitDoor.generated.h"

class UStaticMeshComponent;
class ALightButton;

UCLASS()
class STEALTH_API AExitDoor : public AActor
{
	GENERATED_BODY()

public:
	AExitDoor();

	bool IsOpen() const { return bOpen; }
	int32 GetActivatedCount() const { return ActivatedCount; }
	int32 GetRequiredButtons() const { return RequiredButtons; }
	int32 GetTotalButtons() const { return Buttons.Num(); }

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, Category = "Exit")
	TObjectPtr<UStaticMeshComponent> Door;

	UPROPERTY(EditAnywhere, Category = "Exit")
	int32 RequiredButtons = 3;

	UPROPERTY(VisibleInstanceOnly, Category = "Exit")
	int32 ActivatedCount = 0;

	UPROPERTY(VisibleInstanceOnly, Category = "Exit")
	bool bOpen = false;

	UPROPERTY()
	TArray<TObjectPtr<ALightButton>> Buttons;

	UFUNCTION()
	void HandleButtonActivated(ALightButton* Button);

	void Open();
};