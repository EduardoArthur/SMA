#include "Game/NSGameMode.h"

#include "Tetris.h"
#include "EngineUtils.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "Game/NSBoardActor.h"
#include "Game/NSPlayerController.h"
#include "NSDefaultAssets.h"
#include "Game/NSSceneBootstrap.h"
#include "Misc/PackageName.h"
#include "NSDebug.h"

ANSGameMode::ANSGameMode()
{
	PlayerControllerClass = ANSPlayerController::StaticClass();
	DefaultPawnClass = nullptr; // a câmera vem de um CameraActor no mapa (Auto Activate for Player 0)
	MainMenuLevel = TSoftObjectPtr<UWorld>(FSoftObjectPath(NSAssets::MenuMap));
}

void ANSGameMode::StartPlay()
{
	NSBootstrap::EnsureScene(GetWorld(), /*bGameplay*/ true); // antes do BeginPlay dos atores
	Super::StartPlay();
}

void ANSGameMode::BeginPlay()
{
	Super::BeginPlay();
	NSBootstrap::ActivateCamera(GetWorld());

	for (TActorIterator<ANSBoardActor> It(GetWorld()); It; ++It)
	{
		Board = *It;
		break;
	}
	if (!Board.IsValid())
	{
		NSDebug::Screen(TEXT("NSGameMode: nenhum NSBoardActor no mapa."), true);
		return;
	}
	Board->OnGameOver.AddDynamic(this, &ANSGameMode::HandleGameOver);

	APlayerController* AnyPC = GetWorld()->GetFirstPlayerController();
	if (ANSPlayerController* PC = Cast<ANSPlayerController>(AnyPC))
	{
		PC->SetBoard(Board.Get());
		PC->InitUI(this, Board.Get());
	}
	else
	{
		NSDebug::Screen(FString::Printf(TEXT("NSGameMode: PlayerController é '%s', esperado NSPlayerController. Sem input/UI."),
			*GetNameSafe(AnyPC ? AnyPC->GetClass() : nullptr)), true);
	}

	UE_LOG(LogNeonStack, Log, TEXT("NSGameMode: pronto (board %s)."), *Board->GetName());
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
	if (MainMenuLevel.IsNull() || !FPackageName::DoesPackageExist(MainMenuLevel.ToSoftObjectPath().GetLongPackageName()))
	{
		UE_LOG(LogNeonStack, Warning, TEXT("NSGameMode: mapa do menu não existe; reiniciando a partida."));
		RestartGame();
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
	UE_LOG(LogNeonStack, Log, TEXT("NSGameMode: estado -> %s"), *UEnum::GetValueAsString(NewState));

	// Rede de segurança: se o widget de contagem não terminar (ex.: UI com problema), começa mesmo assim
	if (NewState == ENSFlowState::Countdown)
	{
		GetWorldTimerManager().SetTimer(CountdownSafetyTimer, this, &ANSGameMode::FinishCountdown, CountdownSafetyTime, false);
	}
	else
	{
		GetWorldTimerManager().ClearTimer(CountdownSafetyTimer);
	}

	OnFlowStateChanged.Broadcast(NewState); // sempre transmite (inclusive Countdown -> Countdown)
}
