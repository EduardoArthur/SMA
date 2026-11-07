#include "FX/NSFXDirector.h"

#include "Tetris.h"
#include "EngineUtils.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"
#include "Camera/CameraShakeBase.h"
#include "Engine/PostProcessVolume.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/KismetMaterialLibrary.h"
#include "Materials/MaterialParameterCollection.h"
#include "Data/NSGameConfig.h"
#include "FX/NSBackgroundActor.h"
#include "FX/NSFxSettings.h"
#include "Game/NSBoardActor.h"
#include "Game/NSBoardViewComponent.h"
#include "Systems/NSSaveSubsystem.h"
#include "NSDefaultAssets.h"

ANSFXDirector::ANSFXDirector()
{
	PrimaryActorTick.bCanEverTick = true;
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
}

void ANSFXDirector::BeginPlay()
{
	Super::BeginPlay();
	NSAssets::FillIfEmpty<UMaterialParameterCollection>(Collection, NSAssets::Collection);

	for (TActorIterator<ANSBoardActor> It(GetWorld()); It; ++It)
	{
		Board = *It;
		break;
	}
	for (TActorIterator<ANSBackgroundActor> It(GetWorld()); It; ++It)
	{
		Background = *It;
		break;
	}

	if (ANSBoardActor* B = Board.Get())
	{
		B->OnPieceHardDropped.AddDynamic(this, &ANSFXDirector::HandlePieceHardDropped);
		B->OnPieceLocked.AddDynamic(this, &ANSFXDirector::HandlePieceLocked);
		B->OnLinesCleared.AddDynamic(this, &ANSFXDirector::HandleLinesCleared);
		B->OnLevelUp.AddDynamic(this, &ANSFXDirector::HandleLevelUp);
	}
	else
	{
		UE_LOG(LogNeonStack, Warning, TEXT("NSFXDirector: nenhum BP_Board no mapa."));
	}

	if (UNSSaveSubsystem* Save = UNSSaveSubsystem::Get(this))
	{
		Save->OnSettingsChanged.AddDynamic(this, &ANSFXDirector::HandleSettingsChanged);
		Settings = Save->GetSettings();
	}
	ApplyPostProcess();
}

// ---------------------------------------------------------------------------
// Eventos
// ---------------------------------------------------------------------------

void ANSFXDirector::HandlePieceHardDropped(int32 FromY, int32 ToY, const TArray<FIntPoint>& Cells)
{
	if (Cells.Num() == 0)
	{
		return;
	}
	// Rastro vertical: do ponto de partida ao destino, no centro das colunas da peça
	float SumX = 0.f;
	int32 MinCellY = MAX_int32;
	for (const FIntPoint& C : Cells)
	{
		SumX += C.X;
		MinCellY = FMath::Min(MinCellY, C.Y);
	}
	const float CenterX = SumX / Cells.Num();
	const int32 Distance = FromY - ToY;
	if (Distance > 0)
	{
		const float MidY = MinCellY + Distance * 0.5f;
		const float CellSize = Board.IsValid() ? Board->View->CellSize : 100.f;
		SpawnFX(HardDropTrailFX, BoardPoint(CenterX, MidY), NSFx::ParticleScale(Settings.FxIntensity), Distance * CellSize);
	}
	PlayShake(HardDropShake, HardDropShakeAmplitude, HardDropShakeDuration);
}

void ANSFXDirector::HandlePieceLocked(ENSPieceType Type, const TArray<FIntPoint>& Cells)
{
	ANSBoardActor* B = Board.Get();
	if (!B)
	{
		return;
	}
	B->View->PulseCells(Cells, LockPulseStrength);

	// Faíscas embaixo da peça
	int32 MinY = MAX_int32;
	for (const FIntPoint& C : Cells) { MinY = FMath::Min(MinY, C.Y); }
	const FLinearColor Color = B->View->GetPieceColor(Type);
	for (const FIntPoint& C : Cells)
	{
		if (C.Y == MinY)
		{
			SpawnFX(LockSparkFX, BoardPoint(C.X, C.Y - 0.5f), NSFx::ParticleScale(Settings.FxIntensity), 0.f, Color);
		}
	}
}

void ANSFXDirector::HandleLinesCleared(const TArray<int32>& Rows, int32 Count, int32 Combo, int32 ScoreGained)
{
	ANSBoardActor* B = Board.Get();
	if (!B)
	{
		return;
	}
	B->View->FlashRows(Rows);

	const float Intensity = NSFx::ParticleScale(Settings.FxIntensity);
	const float Width = B->View->CellSize * (B->Config ? B->Config->BoardWidth : 10);
	const float CenterX = ((B->Config ? B->Config->BoardWidth : 10) - 1) * 0.5f;

	float SumY = 0.f;
	for (const int32 Row : Rows)
	{
		SpawnFX(LineClearFX, BoardPoint(CenterX, Row), Intensity, Width);
		SumY += Row;
	}

	if (Count >= 4)
	{
		const FVector Center = BoardPoint(CenterX, SumY / FMath::Max(1, Rows.Num()));
		SpawnFX(QuadBurstFX, Center, Intensity, Width);
		PlayShake(QuadShake, QuadShakeAmplitude, QuadShakeDuration);
		Flash = QuadFlashStrength * NSFx::FlashScale(Settings.FxIntensity);
		FringeBoost = 1.f;
		if (ANSBackgroundActor* Bg = Background.Get())
		{
			Bg->PulseSun();
		}
		OnQuad(Center);
	}
}

void ANSFXDirector::HandleLevelUp(int32 NewLevel)
{
	ANSBoardActor* B = Board.Get();
	if (!B)
	{
		return;
	}
	const int32 W = B->Config ? B->Config->BoardWidth : 10;
	const int32 H = B->Config ? B->Config->VisibleHeight : 20;
	SpawnFX(LevelUpSweepFX, BoardPoint((W - 1) * 0.5f, 0.f), NSFx::ParticleScale(Settings.FxIntensity), H * B->View->CellSize);
}

void ANSFXDirector::HandleSettingsChanged(const FNSSettings& NewSettings)
{
	Settings = NewSettings;
	ApplyPostProcess();
}

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

FVector ANSFXDirector::BoardPoint(float CellX, float CellY) const
{
	const ANSBoardActor* B = Board.Get();
	if (!B)
	{
		return GetActorLocation();
	}
	const float S = B->View->CellSize;
	const FVector Local(FXDepthOffset, CellX * S + S * 0.5f, CellY * S + S * 0.5f);
	return B->View->GetComponentTransform().TransformPosition(Local);
}

void ANSFXDirector::SpawnFX(UNiagaraSystem* System, const FVector& Location, float Intensity, float Length, const FLinearColor& Color)
{
	if (!System)
	{
		return;
	}
	if (UNiagaraComponent* NC = UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), System, Location))
	{
		// Parâmetros de usuário (crie no Niagara como User.Intensity, User.Length, User.Color)
		NC->SetVariableFloat(TEXT("Intensity"), Intensity);
		NC->SetVariableFloat(TEXT("Length"), Length);
		NC->SetVariableLinearColor(TEXT("Color"), Color);
	}
}

void ANSFXDirector::PlayShake(TSubclassOf<UCameraShakeBase> Shake, float Amplitude, float Duration)
{
	if (!Settings.bScreenShake)
	{
		return;
	}
	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	if (!PC)
	{
		return;
	}
	if (Shake)
	{
		PC->ClientStartCameraShake(Shake, 1.f);
		return;
	}

	// Tremor procedural: desloca o ator da câmera e devolve à posição original
	AActor* Target = PC->GetViewTarget();
	if (!Target)
	{
		return;
	}
	if (ShakeTimeLeft <= 0.f || ShakeTarget.Get() != Target)
	{
		ShakeTarget = Target;
		ShakeBaseLocation = Target->GetActorLocation();
	}
	if (Amplitude >= ShakeAmplitude * (ShakeTimeLeft / FMath::Max(ShakeDuration, KINDA_SMALL_NUMBER)))
	{
		ShakeAmplitude = Amplitude; // um shake mais forte substitui o atual
		ShakeDuration = Duration;
		ShakeTimeLeft = Duration;
	}
}

void ANSFXDirector::UpdateProceduralShake(float DeltaSeconds)
{
	if (ShakeTimeLeft <= 0.f)
	{
		return;
	}
	AActor* Target = ShakeTarget.Get();
	if (!Target)
	{
		ShakeTimeLeft = 0.f;
		return;
	}
	ShakeTimeLeft -= DeltaSeconds;
	if (ShakeTimeLeft <= 0.f)
	{
		Target->SetActorLocation(ShakeBaseLocation);
		return;
	}
	const float Decay = ShakeTimeLeft / FMath::Max(ShakeDuration, KINDA_SMALL_NUMBER);
	const FVector Offset(0.f, FMath::FRandRange(-1.f, 1.f), FMath::FRandRange(-1.f, 1.f));
	Target->SetActorLocation(ShakeBaseLocation + Offset * ShakeAmplitude * Decay);
}

void ANSFXDirector::ApplyPostProcess()
{
	if (!PostProcess)
	{
		return;
	}
	FPostProcessSettings& PP = PostProcess->Settings;
	PP.bOverride_BloomIntensity = true;
	PP.BloomIntensity = BaseBloomIntensity * NSFx::BloomScale(Settings.FxIntensity);
	PP.bOverride_SceneFringeIntensity = true;
	PP.SceneFringeIntensity = BaseFringe + (QuadFringe - BaseFringe) * FringeBoost;
}

void ANSFXDirector::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (Flash > 0.f)
	{
		Flash = FMath::Max(0.f, Flash - DeltaSeconds / FMath::Max(FlashDecayTime, KINDA_SMALL_NUMBER));
	}
	if (Collection)
	{
		UKismetMaterialLibrary::SetScalarParameterValue(GetWorld(), Collection, TEXT("Flash"), Flash);
	}

	if (FringeBoost > 0.f)
	{
		FringeBoost = FMath::Max(0.f, FringeBoost - DeltaSeconds / FMath::Max(FringeDecayTime, KINDA_SMALL_NUMBER));
		ApplyPostProcess();
	}

	UpdateProceduralShake(DeltaSeconds);
}
