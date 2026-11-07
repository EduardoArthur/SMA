#include "Game/NSGameMode.h"

#include "Tetris.h"
#include "EngineUtils.h"
#include "TimerManager.h"
#include "Game/NSBoardActor.h"
#include "Game/NSPlayerController.h"

ANSGameMode::ANSGameMode()
{
	PlayerControllerClass = ANSPlayerController::StaticClass();
	DefaultPawnClass = nullptr; // a câmera vem de um CameraActor no mapa (Auto Activate for Player 0)
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
	}

	RestartGame();
}

void ANSGameMode::RestartGame()
{
	GetWorldTimerManager().ClearTimer(RestartTimer);
	if (ANSBoardActor* B = Board.Get())
	{
		B->SetPaused(false);
		B->StartGame();
		SetFlowState(ENSFlowState::Playing);
	}
}

void ANSGameMode::TogglePause()
{
	ANSBoardActor* B = Board.Get();
	if (!B)
	{
		return;
	}
	if (FlowState == ENSFlowState::Playing)
	{
		B->SetPaused(true);
		SetFlowState(ENSFlowState::Paused);
	}
	else if (FlowState == ENSFlowState::Paused)
	{
		B->SetPaused(false); // M3: passar pela contagem 3-2-1 antes de retomar
		SetFlowState(ENSFlowState::Playing);
	}
}

void ANSGameMode::HandleGameOver(int64 Score, int32 Lines, int32 Level, bool bNewHighScore)
{
	SetFlowState(ENSFlowState::GameOver);
	// M1: reinicia sozinho. M3: tela de resultados.
	GetWorldTimerManager().SetTimer(RestartTimer, this, &ANSGameMode::RestartGame, AutoRestartDelay, false);
}

void ANSGameMode::SetFlowState(ENSFlowState NewState)
{
	if (FlowState == NewState)
	{
		return;
	}
	FlowState = NewState;
	OnFlowStateChanged.Broadcast(NewState);
}
