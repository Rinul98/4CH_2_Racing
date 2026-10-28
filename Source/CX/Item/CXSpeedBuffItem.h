// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Item/CXItemBase.h"
#include "CXSpeedBuffItem.generated.h"

UCLASS()
class CX_API ACXSpeedBuffItem : public ACXItemBase
{
	GENERATED_BODY()

protected:
	virtual void ApplyItemEffect(ACXPlayerState* PlayerState) override;

	// km/h
	UPROPERTY(EditAnywhere, Category = "Item")
	float BonusMaxSpeedAmount = 10.0f;
};
