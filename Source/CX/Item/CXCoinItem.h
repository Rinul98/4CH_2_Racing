// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Item/CXItemBase.h"
#include "CXCoinItem.generated.h"

UCLASS()
class CX_API ACXCoinItem : public ACXItemBase
{
	GENERATED_BODY()

protected:
	virtual void ApplyItemEffect(ACXPlayerState* PlayerState) override;

	UPROPERTY(EditAnywhere, Category = "Item")
	int32 CoinAmount = 10;
};
