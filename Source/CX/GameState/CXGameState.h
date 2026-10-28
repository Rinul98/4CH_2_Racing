// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "CXGameState.generated.h"

UENUM(BlueprintType)
enum class ECXMatchState : uint8
{
	Waiting,
	Playing,
	Finished
};

/**
 * 모든 플레이어가 같은 게임 상태, 남은 시간, 우승자를 보도록 복제
 */
UCLASS()
class CX_API ACXGameState : public AGameStateBase
{
	GENERATED_BODY()

public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(Replicated, BlueprintReadOnly)
	ECXMatchState MatchState = ECXMatchState::Waiting;

	UPROPERTY(Replicated, BlueprintReadOnly)
	int32 RemainingTime = 0;

	UPROPERTY(Replicated, BlueprintReadOnly)
	TArray<TObjectPtr<APlayerState>> Winners;
};
