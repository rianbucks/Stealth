#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ExitZone.generated.h"

class UBoxComponent;

UCLASS()
class STEALTH_API AExitZone : public AActor
{
	GENERATED_BODY()

public:
	AExitZone();

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, Category = "Exit")
	TObjectPtr<UBoxComponent> Zone;

	UFUNCTION()
	void OnZoneBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
};