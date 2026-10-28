// Fill out your copyright notice in the Description page of Project Settings.


#include "Item/CXCoinItem.h"
#include "PlayerState/CXPlayerState.h"

void ACXCoinItem::ApplyItemEffect(ACXPlayerState* PlayerState)
{
	PlayerState->AddCoins(CoinAmount);
}
