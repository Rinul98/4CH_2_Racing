// CXGameMode.cpp

#include "CXGameMode.h"
#include "CXPlayerController.h"
#include "EngineUtils.h"
#include "Engine/World.h"
#include "GameFramework/PlayerStart.h"
#include "Subsystem/ChatSubsystem.h"
#include "GameState/CXGameState.h"
#include "PlayerState/CXPlayerState.h"
#include "TimerManager.h"

ACXGameMode::ACXGameMode()
{
	PlayerControllerClass = ACXPlayerController::StaticClass();
	GameStateClass = ACXGameState::StaticClass();
	PlayerStateClass = ACXPlayerState::StaticClass();
}

void ACXGameMode::BeginPlay()
{
	Super::BeginPlay();

	if (ACXGameState* CXGameState = GetGameState<ACXGameState>())
	{
		CXGameState->MatchState = ECXMatchState::Waiting;
		CXGameState->RemainingTime = WaitingTime;
	}
}

void ACXGameMode::OnMatchTimerTick()
{
	ACXGameState* CXGameState = GetGameState<ACXGameState>();
	if (IsValid(CXGameState) == false)
	{
		return;
	}

	CXGameState->RemainingTime = FMath::Max(CXGameState->RemainingTime - 1, 0);
	if (CXGameState->RemainingTime > 0)
	{
		return;
	}

	if (CXGameState->MatchState == ECXMatchState::Waiting)
	{
		CXGameState->MatchState = ECXMatchState::Playing;
		CXGameState->RemainingTime = PlayingTime;
	}
	else if (CXGameState->MatchState == ECXMatchState::Playing)
	{
		CXGameState->MatchState = ECXMatchState::Finished;
		GetWorldTimerManager().ClearTimer(MatchTimerHandle);
		DecideWinners();
	}
}

void ACXGameMode::DecideWinners()
{
	ACXGameState* CXGameState = GetGameState<ACXGameState>();
	if (IsValid(CXGameState) == false)
	{
		return;
	}

	int32 HighestCoins = -1;
	CXGameState->Winners.Empty();

	for (APlayerState* PlayerState : CXGameState->PlayerArray)
	{
		ACXPlayerState* CXPlayerState = Cast<ACXPlayerState>(PlayerState);
		if (IsValid(CXPlayerState) == false)
		{
			continue;
		}

		const int32 Coins = CXPlayerState->GetCoins();
		if (Coins > HighestCoins)
		{
			HighestCoins = Coins;
			CXGameState->Winners.Reset();
			CXGameState->Winners.Add(CXPlayerState);
		}
		else if (Coins == HighestCoins)
		{
			// 최고 원화가 같으면 공동 우승
			CXGameState->Winners.Add(CXPlayerState);
		}
	}
}

void ACXGameMode::PostLogin(APlayerController* NewPlayer)
{
	
	if (IsValid(NewPlayer) == false)
	{
		return;
	}

	if(ACXPlayerController* CXPlayerController = Cast<ACXPlayerController>(NewPlayer))
	{
		if (UChatSubsystem* ChatSubsystem = GetWorld()->GetSubsystem<UChatSubsystem>())
		{
			const TArray<FChatData>ChatLog = ChatSubsystem->GetChatLog();
			if (!ChatLog.IsEmpty())
			{
				CXPlayerController->STCChatLog(ChatLog);
			}
		}
	}

	Super::PostLogin(NewPlayer);

	// 첫 플레이어가 들어오면 대기 카운트다운 시작
	ACXGameState* CXGameState = GetGameState<ACXGameState>();
	if (IsValid(CXGameState) && CXGameState->MatchState == ECXMatchState::Waiting && GetWorldTimerManager().IsTimerActive(MatchTimerHandle) == false)
	{
		CXGameState->RemainingTime = WaitingTime;
		GetWorldTimerManager().SetTimer(MatchTimerHandle, this, &ThisClass::OnMatchTimerTick, 1.0f, true);
	}
}

AActor* ACXGameMode::ChoosePlayerStart_Implementation(AController* Player)
{
	TArray<APlayerStart*> AvailablePlayerStarts;

	for (TActorIterator<APlayerStart> It(GetWorld()); It; ++It)
	{
		APlayerStart* PlayerStart = *It;

		if (IsPlayerStartAvailable(PlayerStart))
		{
			AvailablePlayerStarts.Add(PlayerStart);
		}
	}

	checkf(AvailablePlayerStarts.IsEmpty() == false, TEXT("There is no available Vehicle PlayerStart."));

	const int32 RandomIndex = FMath::RandHelper(AvailablePlayerStarts.Num());
	return Cast<AActor>(AvailablePlayerStarts[RandomIndex]);
}

bool ACXGameMode::IsPlayerStartAvailable(const APlayerStart* PlayerStart) const
{
	if (IsValid(PlayerStart) == false || IsValid(GetWorld()) == false)
	{
		return false;
	}

	const FTransform PlayerStartTransform = PlayerStart->GetActorTransform();
	const FVector CheckLocation = PlayerStartTransform.TransformPosition(VehicleSpawnCheckOffset);
	const FQuat CheckRotation = PlayerStartTransform.GetRotation();
	const FCollisionShape CheckShape = FCollisionShape::MakeBox(VehicleSpawnCheckHalfExtent);
	FCollisionObjectQueryParams ObjectQueryParams;
	ObjectQueryParams.AddObjectTypesToQuery(ECC_Vehicle);

	const FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(VehiclePlayerStartCheck), false);

	const bool bVehicleExists =
		GetWorld()->OverlapAnyTestByObjectType(
			CheckLocation,
			CheckRotation,
			ObjectQueryParams,
			CheckShape,
			QueryParams
		);

	return !bVehicleExists;
}