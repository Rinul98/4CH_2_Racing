// Fill out your copyright notice in the Description page of Project Settings.


#include "Item/CXItemSpawner.h"
#include "Item/CXItemBase.h"
#include "Components/SceneComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"

ACXItemSpawner::ACXItemSpawner()
{
	PrimaryActorTick.bCanEverTick = false;

	SetRootComponent(CreateDefaultSubobject<USceneComponent>(TEXT("Root")));
}

void ACXItemSpawner::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		SpawnItem();
	}
}

void ACXItemSpawner::SpawnItem()
{
	if (IsValid(ItemClass) == false)
	{
		return;
	}

	FActorSpawnParameters SpawnParameters;
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	SpawnedItem = GetWorld()->SpawnActor<ACXItemBase>(ItemClass, GetActorTransform(), SpawnParameters);
	if (IsValid(SpawnedItem))
	{
		SpawnedItem->OnDestroyed.AddDynamic(this, &ThisClass::OnSpawnedItemDestroyed);
	}
}

void ACXItemSpawner::OnSpawnedItemDestroyed(AActor* DestroyedActor)
{
	SpawnedItem = nullptr;

	// 레벨이 내려가는 중이면 리스폰하지 않음
	if (IsValid(this) == false || GetWorld() == nullptr || GetWorld()->bIsTearingDown)
	{
		return;
	}

	GetWorldTimerManager().SetTimer(RespawnTimerHandle, this, &ThisClass::SpawnItem, RespawnDelay, false);
}
