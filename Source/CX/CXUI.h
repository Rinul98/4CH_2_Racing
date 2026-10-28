// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CXUI.generated.h"


class UChatUI;
class UTextBlock;
enum class ECXMatchState : uint8;

/**
 *  Simple Vehicle HUD class
 *  Displays the current speed and gear.
 *  Widget setup is handled in a Blueprint subclass.
 */
UCLASS(abstract)
class CX_API UCXUI : public UUserWidget
{
	GENERATED_BODY()


#pragma region ChatUI
public:
	void OnNewChatMessage(const FString& InSender, const FString& InMessage);

	void FocusChatInput();

	
protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UChatUI> UI_Chat;
#pragma endregion

protected:

	/** Controls the display of speed in Km/h or MPH */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Vehicle)
	bool bIsMPH = false;

public:

	/** Called to update the speed display */
	void UpdateSpeed(float NewSpeed);

	/** Called to update the gear display */
	void UpdateGear(int32 NewGear);

protected:

	/** Implemented in Blueprint to display the new speed */
	UFUNCTION(BlueprintImplementableEvent, Category = Vehicle)
	void OnSpeedUpdate(float NewSpeed);

	/** Implemented in Blueprint to display the new gear */
	UFUNCTION(BlueprintImplementableEvent, Category = Vehicle)
	void OnGearUpdate(int32 NewGear);

#pragma region MatchUI
public:
	void UpdatePlayerStats(int32 Coins, float BonusMaxSpeed);

	void UpdateMatchInfo(ECXMatchState MatchState, int32 RemainingTime, bool bIsWinner);

protected:
	// WBP에 같은 이름의 Text 위젯이 있으면 자동 연결 (없어도 컴파일 오류 안 남)
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> TextBlock_Coins;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> TextBlock_BonusMaxSpeed;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> TextBlock_MatchState;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> TextBlock_RemainingTime;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> TextBlock_MatchResult;

	// Waiting → Playing 전환을 알아채서 GO!를 잠깐 띄우기 위함
	bool bWasPlaying = false;
	double GoTextHideTime = 0.0;
#pragma endregion
};
