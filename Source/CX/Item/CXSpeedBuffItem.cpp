// Fill out your copyright notice in the Description page of Project Settings.


#include "Item/CXSpeedBuffItem.h"
#include "PlayerState/CXPlayerState.h"

void ACXSpeedBuffItem::ApplyItemEffect(ACXPlayerState* PlayerState)
{
	PlayerState->AddBonusMaxSpeed(BonusMaxSpeedAmount);
}
