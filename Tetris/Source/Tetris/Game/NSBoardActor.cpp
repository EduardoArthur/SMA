#include "Game/NSBoardActor.h"

#include "Tetris.h"
#include "Data/NSGameConfig.h"
#include "Game/NSBoardViewComponent.h"
#include "Systems/NSSaveSubsystem.h"
#include "NSDefaultAssets.h"

ANSBoardActor::ANSBoardActor()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickGroup = TG_PrePhysics;

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	View = CreateDefaultSubobject<UNSBoardViewComponent>(TEXT("View"));
	View->SetupAttachment(RootComponent);
}

void ANSBoardActor::BeginPlay()
{
	Super::BeginPlay();

	NSAssets::FillIfEmpty<UNSGameConfig>(Config, NSAssets::GameConfig);
	if (!Config)
	{
		UE_LOG(LogNeonStack, Error, TEXT("%s: Config vazio. Atribua DA_GameConfig no BP_Board."), *GetName());
		return;
	}
	View->InitializeView(Config);

	if (UNSSaveSubsystem* Save = UNSSaveSubsystem::Get(this))
	{
		Save->OnSettingsChanged.AddDynamic(this, &ANSBoardActor::HandleSettingsChanged);
	}
}

void ANSBoardActor::HandleSettingsChanged(const FNSSettings& Settings)
{
	SetHandling(Settings.DasMs, Settings.ArrMs); // vale na hora, inclusive no meio da partida
}

void ANSBoardActor::StartGame()
{
	NSAssets::FillIfEmpty<UNSGameConfig>(Config, NSAssets::GameConfig);
	if (!Config)
	{
		UE_LOG(LogNeonStack, Error, TEXT("%s: StartGame sem Config."), *GetName());
		return;
	}

	// Garante a view pronta mesmo se o GameMode chamar StartGame antes do nosso BeginPlay
	View->InitializeView(Config);

	Sim.Configure(Config->ToSimConfig());
	if (const UNSSaveSubsystem* Save = UNSSaveSubsystem::Get(this))
	{
		const FNSSettings Settings = Save->GetSettings();
		SetHandling(Settings.DasMs, Settings.ArrMs);
	}

	PendingInput = FNSInputFrame();
	bRunning = true;
	bPaused = false;

	EventBuffer.Reset();
	Sim.Start(Config->RandomSeed, EventBuffer);
	ProcessEvents();
}

void ANSBoardActor::StopGame()
{
	bRunning = false;
}

void ANSBoardActor::SetPaused(bool bInPaused)
{
	if (bPaused == bInPaused)
	{
		return;
	}
	bPaused = bInPaused;
	if (bPaused)
	{
		PendingInput = FNSInputFrame(); // descarta input pendente e teclas seguradas
	}
	OnPausedChanged.Broadcast(bPaused);
}

void ANSBoardActor::PressAction(ENSInputAction Action)
{
	if (!bRunning || bPaused)
	{
		return;
	}
	SetHeld(Action, true);
	PendingInput.Pressed.Add(Action);
}

void ANSBoardActor::ReleaseAction(ENSInputAction Action)
{
	SetHeld(Action, false); // funciona mesmo pausado
}

void ANSBoardActor::SetHeld(ENSInputAction Action, bool bHeld)
{
	switch (Action)
	{
	case ENSInputAction::MoveLeft:  PendingInput.bLeftHeld = bHeld; break;
	case ENSInputAction::MoveRight: PendingInput.bRightHeld = bHeld; break;
	case ENSInputAction::SoftDrop:  PendingInput.bSoftDropHeld = bHeld; break;
	default: break;
	}
}

void ANSBoardActor::SetHandling(int32 DasMs, int32 ArrMs)
{
	Sim.SetHandling(DasMs / 1000.f, ArrMs / 1000.f);
}

int32 ANSBoardActor::GetPaletteIndex() const
{
	const int32 Every = Config ? FMath::Max(1, Config->PaletteEveryNLevels) : 5;
	return FMath::Max(0, GetLevel() - 1) / Every;
}

FVector ANSBoardActor::CellToWorld(FIntPoint Cell) const
{
	return View->GetComponentTransform().TransformPosition(View->CellToLocal(Cell));
}

void ANSBoardActor::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (!bRunning || bPaused)
	{
		return;
	}

	EventBuffer.Reset();
	Sim.Tick(FMath::Min(DeltaSeconds, MaxDeltaTime), PendingInput, EventBuffer);
	PendingInput.Pressed.Reset();
	ProcessEvents();
}

void ANSBoardActor::ProcessEvents()
{
	if (EventBuffer.Num() == 0)
	{
		return;
	}

	// Atualiza o desenho ANTES de avisar os Blueprints
	View->Sync(Sim);

	// Cópia local: um handler pode chamar StartGame() e reutilizar o EventBuffer
	TArray<FNSGameEvent> Events;
	Swap(Events, EventBuffer);
	for (const FNSGameEvent& E : Events)
	{
		BroadcastEvent(E);
	}
}

void ANSBoardActor::BroadcastEvent(const FNSGameEvent& E)
{
	switch (E.Type)
	{
	case ENSGameEventType::PieceSpawned:     OnPieceSpawned.Broadcast(E.Piece, E.Cells); break;
	case ENSGameEventType::PieceMoved:       OnPieceMoved.Broadcast(E.Cells, E.Ghost, E.bFlag); break;
	case ENSGameEventType::PieceRotated:     OnPieceRotated.Broadcast(E.Cells, E.Ghost, E.IntA); break;
	case ENSGameEventType::PieceHardDropped: OnPieceHardDropped.Broadcast(E.IntA, E.IntB, E.Cells); break;
	case ENSGameEventType::PieceLocked:      OnPieceLocked.Broadcast(E.Piece, E.Cells); break;
	case ENSGameEventType::LinesCleared:     OnLinesCleared.Broadcast(E.Rows, E.IntA, E.IntB, E.IntC); break;
	case ENSGameEventType::BoardChanged:     break; // só a view
	case ENSGameEventType::ScoreChanged:     OnScoreChanged.Broadcast(E.Score, E.IntA, E.IntB); break;
	case ENSGameEventType::LevelUp:          OnLevelUp.Broadcast(E.IntA); break;
	case ENSGameEventType::HoldChanged:      OnHoldChanged.Broadcast(E.Piece, E.bFlag); break;
	case ENSGameEventType::HoldDenied:       OnHoldDenied.Broadcast(); break;
	case ENSGameEventType::QueueChanged:     OnQueueChanged.Broadcast(E.Queue); break;
	case ENSGameEventType::LockWarning:      OnLockWarning.Broadcast(E.FloatA); break;
	case ENSGameEventType::GameOver:
	{
		bRunning = false;
		UNSSaveSubsystem* Save = UNSSaveSubsystem::Get(this);
		const bool bNewHighScore = Save && Save->SubmitResult(E.Score, E.IntA, E.IntB);
		UE_LOG(LogNeonStack, Log, TEXT("Game over: score %lld, linhas %d, nível %d"), E.Score, E.IntA, E.IntB);
		OnGameOver.Broadcast(E.Score, E.IntA, E.IntB, bNewHighScore);
		break;
	}
	default:
		break;
	}
}
