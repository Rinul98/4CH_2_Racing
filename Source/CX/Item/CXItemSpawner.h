// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CXItemSpawner.generated.h"

class ACXItemBase;

/**
 * 서버에서 아이템을 스폰하고, 파괴되면 RespawnDelay 뒤 같은 종류로 다시 스폰
 */
UCLASS()
class CX_API ACXItemSpawner : public AActor
{
	GENERATED_BODY()

public:
	ACXItemSpawner();

protected:
	virtual void BeginPlay() override;

	void SpawnItem();

	UFUNCTION()
	void OnSpawnedItemDestroyed(AActor* DestroyedActor);

protected:
	// 스포너마다 코인 / 버프 중 하나를 지정
	UPROPERTY(EditAnywhere, Category = "Spawner")
	TSubclassOf<ACXItemBase> ItemClass;

	UPROPERTY(EditAnywhere, Category = "Spawner")
	float RespawnDelay = 2.0f;

	UPROPERTY()
	TObjectPtr<ACXItemBase> SpawnedItem;

	FTimerHandle RespawnTimerHandle;
};
