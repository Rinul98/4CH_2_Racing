// Copyright Epic Games, Inc. All Rights Reserved.


#include "CXUI.h"
#include "UI/ChatUI.h"
#include "Components/TextBlock.h"
#include "GameState/CXGameState.h"

void UCXUI::UpdatePlayerStats(int32 Coins, float BonusMaxSpeed)
{
	if (TextBlock_Coins)
	{
		TextBlock_Coins->SetText(FText::FromString(FString::Printf(TEXT("COINS  %d"), Coins)));
	}

	if (TextBlock_BonusMaxSpeed)
	{
		TextBlock_BonusMaxSpeed->SetText(FText::FromString(FString::Printf(TEXT("MAX SPEED  +%.0f km/h"), BonusMaxSpeed)));
	}
}

void UCXUI::UpdateMatchInfo(ECXMatchState MatchState, int32 RemainingTime, bool bIsWinner)
{
	if (TextBlock_MatchState)
	{
		const TCHAR* StateName = TEXT("GET READY");
		if (MatchState == ECXMatchState::Playing)
		{
			StateName = TEXT("COLLECT COINS!");
		}
		else if (MatchState == ECXMatchState::Finished)
		{
			StateName = TEXT("FINISH");
		}
		TextBlock_MatchState->SetText(FText::FromString(StateName));
	}

	if (TextBlock_RemainingTime)
	{
		// 0:45 형식
		TextBlock_RemainingTime->SetText(FText::FromString(FString::Printf(TEXT("%d:%02d"), RemainingTime / 60, RemainingTime % 60)));
	}

	// 시작하는 순간을 기억해서 GO!를 1.5초 동안 띄움
	const double Now = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0;
	const bool bIsPlaying = MatchState == ECXMatchState::Playing;
	if (bIsPlaying && bWasPlaying == false)
	{
		GoTextHideTime = Now + 1.5;
	}
	bWasPlaying = bIsPlaying;

	if (TextBlock_MatchResult)
	{
		FString CenterText;
		if (MatchState == ECXMatchState::Waiting && RemainingTime > 0 && RemainingTime <= 5)
		{
			CenterText = FString::FromInt(RemainingTime);
		}
		else if (bIsPlaying && Now < GoTextHideTime)
		{
			CenterText = TEXT("GO!");
		}
		else if (MatchState == ECXMatchState::Finished)
		{
			CenterText = bIsWinner ? TEXT("WIN") : TEXT("LOSE");
		}
		TextBlock_MatchResult->SetText(FText::FromString(CenterText));
	}
}

void UCXUI::OnNewChatMessage(const FString& InSender, const FString& InMessage)
{
	UI_Chat->OnNewChatMessage(InSender, InMessage);
}

void UCXUI::FocusChatInput()
{
	if (UI_Chat)
	{
		UI_Chat->FocusChatInput();
	}
}

void UCXUI::UpdateSpeed(float NewSpeed)
{
	// format the speed to KPH or MPH
	float FormattedSpeed = FMath::Abs(NewSpeed) * (bIsMPH ? 0.022f : 0.036f);

	// call the Blueprint handler
	OnSpeedUpdate(FormattedSpeed);
}

void UCXUI::UpdateGear(int32 NewGear)
{
	// call the Blueprint handler
	OnGearUpdate(NewGear);
}