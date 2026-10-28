// Fill out your copyright notice in the Description page of Project Settings.


#include "ChatUI.h"
#include "CXPlayerController.h"

void UChatUI::NativeConstruct()
{
	Super::NativeConstruct();

	if (EditableTextBox_ChatInput->OnTextCommitted.IsAlreadyBound(this, &UChatUI::OnChatInputTextCommitted) == false) // 이미 되어있을때 처리 연결 안될때만 연결
	{
		EditableTextBox_ChatInput->OnTextCommitted.AddDynamic(this, &UChatUI::OnChatInputTextCommitted); // 커밋된 시점
	}
}

void UChatUI::NativeDestruct()
{

	if (EditableTextBox_ChatInput->OnTextCommitted.IsAlreadyBound(this, &UChatUI::OnChatInputTextCommitted) == true)
	{
		EditableTextBox_ChatInput->OnTextCommitted.AddDynamic(this, &UChatUI::OnChatInputTextCommitted); // 커밋된 시점
	}
	Super::NativeDestruct();
}

void UChatUI::OnNewChatMessage(const FString& InSender, const FString& InMessage)
{
	FString NewMessage = InSender + TEXT(": ") + InMessage;
	
	FString OldMessage = MultiLineEditableTextBox_ChatLog->GetText().ToString();
	OldMessage += '\n';
	
	FText Message = FText::FromString(OldMessage + NewMessage);


	MultiLineEditableTextBox_ChatLog->SetText(Message);
}

void UChatUI::OnChatInputTextCommitted(const FText& Text, ETextCommit::Type CommitMethod)
{
	if (CommitMethod == ETextCommit::OnEnter)
	{
		if (Text.IsEmpty() == false)
		{
			FString ChatMessage = Text.ToString();
			EditableTextBox_ChatInput->SetText(FText::GetEmpty());
			if (ACXPlayerController* PC = GetOwningPlayer<ACXPlayerController>())
			{
				PC->CTSChatMessage(ChatMessage);
			}
		}
	}

	// Enter(빈 칸 포함)나 Esc로 입력이 끝나면 다시 운전 조작으로
	if (CommitMethod == ETextCommit::OnEnter || CommitMethod == ETextCommit::OnCleared)
	{
		if (APlayerController* PlayerController = GetOwningPlayer())
		{
			PlayerController->SetInputMode(FInputModeGameOnly());
		}
	}
}

void UChatUI::FocusChatInput()
{
	APlayerController* PlayerController = GetOwningPlayer();
	if (IsValid(PlayerController) == false || IsValid(EditableTextBox_ChatInput) == false)
	{
		return;
	}

	// 입력 중엔 키가 채팅창으로만 가도록 UI 전용 모드 (보내면 GameOnly로 되돌림)
	FInputModeUIOnly InputMode;
	InputMode.SetWidgetToFocus(EditableTextBox_ChatInput->TakeWidget());
	PlayerController->SetInputMode(InputMode);
}