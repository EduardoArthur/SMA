#include "Game/NSGameMode.h"

#include "Tetris.h"
#include "EngineUtils.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "Game/NSBoardActor.h"
#include "Game/NSPlayerController.h"
#include "NSDefaultAssets.h"

ANSGameMode::ANSGameMode()
{
	PlayerControllerClass = ANSPlayerController::StaticClass();
	DefaultPawnClass = nullptr; // a câmera vem de um CameraActor no mapa (Auto Activate for Player 0)
	MainMenuLevel = TSoftObjectPtr<UWorld>(FSoftObjectPath(NSAssets::MenuMap));
}

void ANSGameMode::BeginPlay()
{
	Super::BeginPlay();

	for (TActorIterator<ANSBoardActor> It(GetWorld()); It; ++It)
	{
		Board = *It;
		break;
	}
	if (!Board.IsValid())
	{
		UE_LOG(LogNeonStack, Error, TEXT("NSGameMode: nenhum ANSBoardActor (BP_Board) no mapa."));
		return;
	}
	Board->OnGameOver.AddDynamic(this, &ANSGameMode::HandleGameOver);

	if (ANSPlayerController* PC = Cast<ANSPlayerController>(GetWorld()->GetFirstPlayerController()))
	{
		PC->SetBoard(Board.Get());
		PC->InitUI(this, Board.Get());
	}

	bStartPending = true;
	SetFlowState(ENSFlowState::Countdown);
}

void ANSGameMode::FinishCountdown()
{
	ANSBoardActor* B = Board.Get();
	if (!B || FlowState != ENSFlowState::Countdown)
	{
		return;
	}
	if (bStartPending)
	{
		B->SetPaused(false);
		B->StartGame();
	}
	else
	{
		B->SetPaused(false);
	}
	SetFlowState(ENSFlowState::Playing);
}

void ANSGameMode::TogglePause()
{
	if (FlowState == ENSFlowState::Playing)
	{
		PauseGame();
	}
	else if (FlowState == ENSFlowState::Paused)
	{
		ResumeGame();
	}
}

void ANSGameMode::PauseGame()
{
	ANSBoardActor* B = Board.Get();
	if (!B || FlowState != ENSFlowState::Playing)
	{
		return;
	}
	B->SetPaused(true);
	SetFlowState(ENSFlowState::Paused);
}

void ANSGameMode::ResumeGame()
{
	if (FlowState != ENSFlowState::Paused)
	{
		return;
	}
	bStartPending = false; // o board continua pausado até o FinishCountdown
	SetFlowState(ENSFlowState::Countdown);
}

void ANSGameMode::RestartGame()
{
	GetWorldTimerManager().ClearTimer(ResultsTimer);
	if (ANSBoardActor* B = Board.Get())
	{
		B->StopGame();
		B->SetPaused(false);
	}
	bStartPending = true;
	SetFlowState(ENSFlowState::Countdown);
}

void ANSGameMode::ReturnToMenu()
{
	if (MainMenuLevel.IsNull())
	{
		UE_LOG(LogNeonStack, Warning, TEXT("NSGameMode: MainMenuLevel não configurado no BP_NeonStackGameMode."));
		return;
	}
	UGameplayStatics::OpenLevelBySoftObjectPtr(this, MainMenuLevel);
}

void ANSGameMode::HandleGameOver(int64 Score, int32 Lines, int32 Level, bool bNewHighScore)
{
	LastScore = Score;
	LastLines = Lines;
	LastLevel = Level;
	bLastNewHighScore = bNewHighScore;

	SetFlowState(ENSFlowState::GameOver);
	if (GameOverToResultsDelay > 0.f)
	{
		GetWorldTimerManager().SetTimer(ResultsTimer, this, &ANSGameMode::ShowResults, GameOverToResultsDelay, false);
	}
	else
	{
		ShowResults();
	}
}

void ANSGameMode::ShowResults()
{
	SetFlowState(ENSFlowState::Results);
}

void ANSGameMode::SetFlowState(ENSFlowState NewState)
{
	FlowState = NewState;
	OnFlowStateChanged.Broadcast(NewState); // sempre transmite (inclusive Countdown -> Countdown)
}
