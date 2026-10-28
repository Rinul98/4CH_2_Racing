// Fill out your copyright notice in the Description page of Project Settings.


#include "Item/CXItemBase.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystem.h"
#include "Sound/SoundBase.h"
#include "PlayerState/CXPlayerState.h"
#include "GameState/CXGameState.h"

ACXItemBase::ACXItemBase()
{
	PrimaryActorTick.bCanEverTick = false;

	bReplicates = true;
	SetNetUpdateFrequency(1.0f);

	SphereCollision = CreateDefaultSubobject<USphereComponent>(TEXT("SphereCollision"));
	SphereCollision->SetSphereRadius(150.0f);
	SphereCollision->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
	SetRootComponent(SphereCollision);

	ItemMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ItemMesh"));
	ItemMesh->SetupAttachment(SphereCollision);
	ItemMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void ACXItemBase::BeginPlay()
{
	Super::BeginPlay();

	// 획득 판정은 서버에서만
	if (HasAuthority())
	{
		SphereCollision->OnComponentBeginOverlap.AddDynamic(this, &ThisClass::OnSphereBeginOverlap);
	}
}

void ACXItemBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// 서버에서 Destroy되면 클라이언트에서도 Destroyed로 EndPlay가 불리니까 모두에게 연출이 보임
	// 레벨 종료 같은 다른 이유일 땐 연출 생략
	if (EndPlayReason == EEndPlayReason::Destroyed)
	{
		if (IsValid(DestroyParticle))
		{
			UGameplayStatics::SpawnEmitterAtLocation(this, DestroyParticle, GetActorLocation());
		}

		if (IsValid(DestroySound))
		{
			UGameplayStatics::PlaySoundAtLocation(this, DestroySound, GetActorLocation());
		}
	}

	Super::EndPlay(EndPlayReason);
}

void ACXItemBase::OnSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (IsValid(this) == false || IsActorBeingDestroyed())
	{
		return;
	}

	// 게임 진행 중에만 획득 (종료 후 코인 변동 방지)
	const ACXGameState* CXGameState = GetWorld()->GetGameState<ACXGameState>();
	if (IsValid(CXGameState) == false || CXGameState->MatchState != ECXMatchState::Playing)
	{
		return;
	}

	APawn* OverlappedPawn = Cast<APawn>(OtherActor);
	if (IsValid(OverlappedPawn) == false)
	{
		return;
	}

	ACXPlayerState* PlayerState = OverlappedPawn->GetPlayerState<ACXPlayerState>();
	if (IsValid(PlayerState) == false)
	{
		return;
	}

	ApplyItemEffect(PlayerState);
	Destroy();
}
