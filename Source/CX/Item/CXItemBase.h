// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CXItemBase.generated.h"

class USphereComponent;
class UStaticMeshComponent;
class UParticleSystem;
class USoundBase;
class ACXPlayerState;

/**
 * 코인, 스피드 버프 아이템의 공통 부모. 겹치면 효과 적용 후 파괴
 */
UCLASS(abstract)
class CX_API ACXItemBase : public AActor
{
	GENERATED_BODY()

public:
	ACXItemBase();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	// 자식 클래스가 아이템 효과를 정함 (서버에서만 호출됨)
	virtual void ApplyItemEffect(ACXPlayerState* PlayerState) PURE_VIRTUAL(ACXItemBase::ApplyItemEffect, );

	UFUNCTION()
	void OnSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

protected:
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USphereComponent> SphereCollision;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> ItemMesh;

	UPROPERTY(EditAnywhere, Category = "Item")
	TObjectPtr<UParticleSystem> DestroyParticle;

	UPROPERTY(EditAnywhere, Category = "Item")
	TObjectPtr<USoundBase> DestroySound;
};
