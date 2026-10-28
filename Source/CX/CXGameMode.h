// CXGameMode.h

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "CXGameMode.generated.h"

class APlayerStart;

UCLASS(MinimalAPI)
class ACXGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ACXGameMode();

protected:
	virtual AActor* ChoosePlayerStart_Implementation(AController* Player) override;

	virtual void PostLogin(APlayerController* NewPlayer);

	virtual void BeginPlay() override;

private:
	bool IsPlayerStartAvailable(const APlayerStart* PlayerStart) const;

	// 1초마다 남은 시간을 줄이고, 0이 되면 다음 상태로 넘김
	void OnMatchTimerTick();

	void DecideWinners();

private:
	UPROPERTY(EditDefaultsOnly, Category = "Match")
	int32 WaitingTime = 10;

	UPROPERTY(EditDefaultsOnly, Category = "Match")
	int32 PlayingTime = 60;

	FTimerHandle MatchTimerHandle;

private:
	UPROPERTY(EditDefaultsOnly)
	FVector VehicleSpawnCheckHalfExtent = FVector(250.0, 120.0, 80.0);

	UPROPERTY(EditDefaultsOnly)
	FVector VehicleSpawnCheckOffset = FVector(0.0, 0.0, 80.0);

};



