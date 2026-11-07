#include "FX/NSBackgroundActor.h"

#include "Tetris.h"
#include "EngineUtils.h"
#include "Kismet/KismetMaterialLibrary.h"
#include "Materials/MaterialParameterCollection.h"
#include "Game/NSBoardActor.h"
#include "Systems/NSSaveSubsystem.h"
#include "NSDefaultAssets.h"

namespace NSBg
{
	static FLinearColor Lerp(const FLinearColor& A, const FLinearColor& B, float T) { return A + (B - A) * T; }

	static FNSPalette LerpPalette(const FNSPalette& A, const FNSPalette& B, float T)
	{
		FNSPalette P = B;
		P.SkyTop = Lerp(A.SkyTop, B.SkyTop, T);
		P.SkyHorizon = Lerp(A.SkyHorizon, B.SkyHorizon, T);
		P.SunTop = Lerp(A.SunTop, B.SunTop, T);
		P.SunBottom = Lerp(A.SunBottom, B.SunBottom, T);
		P.Grid = Lerp(A.Grid, B.Grid, T);
		P.Accent = Lerp(A.Accent, B.Accent, T);
		return P;
	}
}

ANSBackgroundActor::ANSBackgroundActor()
{
	PrimaryActorTick.bCanEverTick = true;
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
}

void ANSBackgroundActor::BeginPlay()
{
	Super::BeginPlay();
	NSAssets::FillIfEmpty<UMaterialParameterCollection>(Collection, NSAssets::Collection);
	NSAssets::FillIfEmpty<UNSPaletteData>(Palettes, NSAssets::Palettes);

	if (!Collection)
	{
		UE_LOG(LogNeonStack, Warning, TEXT("NSBackgroundActor: Collection (MPC_NeonStack) não atribuída."));
	}

	for (TActorIterator<ANSBoardActor> It(GetWorld()); It; ++It)
	{
		Board = *It;
		break;
	}
	if (ANSBoardActor* B = Board.Get())
	{
		B->OnLevelUp.AddDynamic(this, &ANSBackgroundActor::HandleLevelUp);
		B->OnScoreChanged.AddDynamic(this, &ANSBackgroundActor::HandleScoreChanged);
		B->OnLinesCleared.AddDynamic(this, &ANSBackgroundActor::HandleLinesCleared);
		B->OnPausedChanged.AddDynamic(this, &ANSBackgroundActor::HandlePausedChanged);
	}
	if (UNSSaveSubsystem* Save = UNSSaveSubsystem::Get(this))
	{
		Save->OnSettingsChanged.AddDynamic(this, &ANSBackgroundActor::HandleSettingsChanged);
		HandleSettingsChanged(Save->GetSettings());
	}

	SetLevel(1, /*bInstant*/ true);
	WriteParameters();
}

void ANSBackgroundActor::SetLevel(int32 Level, bool bInstant)
{
	CurrentLevel = FMath::Max(1, Level);

	const float Raw = ScrollBase + ScrollPerLevel * (CurrentLevel - 1);
	ScrollSpeed = FMath::Min(Raw, ScrollMax);

	const int32 NewIndex = (CurrentLevel - 1) / FMath::Max(1, PaletteEveryNLevels);
	if (!Palettes)
	{
		return;
	}
	if (bInstant)
	{
		TargetPaletteIndex = NewIndex;
		CurrentPalette = Palettes->GetPalette(NewIndex);
		FromPalette = CurrentPalette;
		PaletteAlpha = 1.f;
	}
	else if (NewIndex != TargetPaletteIndex)
	{
		TargetPaletteIndex = NewIndex;
		FromPalette = CurrentPalette;   // parte de onde está (mesmo no meio de outra transição)
		PaletteAlpha = 0.f;
	}
}

void ANSBackgroundActor::HandleLevelUp(int32 NewLevel)
{
	SetLevel(NewLevel, false);
}

void ANSBackgroundActor::HandleScoreChanged(int64 Score, int32 Lines, int32 Level)
{
	// Nova partida (nível volta a 1): reseta na hora
	if (Level < CurrentLevel)
	{
		SetLevel(Level, true);
	}
}

void ANSBackgroundActor::HandleLinesCleared(const TArray<int32>& Rows, int32 Count, int32 Combo, int32 ScoreGained)
{
	if (Combo >= 1)
	{
		ComboGlow = FMath::Min(ComboGlowMax, Combo * ComboGlowPerCombo);
	}
	if (Count >= 4)
	{
		PulseSun();
	}
}

void ANSBackgroundActor::HandlePausedChanged(bool bInPaused)
{
	bPaused = bInPaused;
}

void ANSBackgroundActor::HandleSettingsChanged(const FNSSettings& Settings)
{
	bReducedMotion = Settings.FxIntensity == ENSFxIntensity::Low;
}

void ANSBackgroundActor::PulseSun()
{
	SunPulse = SunPulseTime;
}

void ANSBackgroundActor::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	// Paleta
	if (Palettes && PaletteAlpha < 1.f)
	{
		PaletteAlpha = FMath::Min(1.f, PaletteAlpha + DeltaSeconds / FMath::Max(PaletteTransitionTime, KINDA_SMALL_NUMBER));
		const float Smooth = FMath::SmoothStep(0.f, 1.f, PaletteAlpha);
		CurrentPalette = NSBg::LerpPalette(FromPalette, Palettes->GetPalette(TargetPaletteIndex), Smooth);
	}

	// Rolagem do chão (offset acumulado: mudar a velocidade não dá "pulo")
	float Speed = bReducedMotion ? ScrollBase : ScrollSpeed;
	if (bPaused || !Board.IsValid())
	{
		Speed *= ScrollPausedFactor;
	}
	FloorOffset = FMath::Fmod(FloorOffset + Speed * DeltaSeconds, 1000.f);

	// Decaimentos
	ComboGlow = FMath::Max(0.f, ComboGlow - DeltaSeconds * (ComboGlowMax / FMath::Max(ComboGlowDecayTime, KINDA_SMALL_NUMBER)));
	SunPulse = FMath::Max(0.f, SunPulse - DeltaSeconds);

	WriteParameters();
}

void ANSBackgroundActor::WriteParameters()
{
	if (!Collection)
	{
		return;
	}
	UWorld* World = GetWorld();
	UKismetMaterialLibrary::SetVectorParameterValue(World, Collection, TEXT("SkyTop"), CurrentPalette.SkyTop);
	UKismetMaterialLibrary::SetVectorParameterValue(World, Collection, TEXT("SkyHorizon"), CurrentPalette.SkyHorizon);
	UKismetMaterialLibrary::SetVectorParameterValue(World, Collection, TEXT("SunTop"), CurrentPalette.SunTop);
	UKismetMaterialLibrary::SetVectorParameterValue(World, Collection, TEXT("SunBottom"), CurrentPalette.SunBottom);
	UKismetMaterialLibrary::SetVectorParameterValue(World, Collection, TEXT("GridColor"), CurrentPalette.Grid);
	UKismetMaterialLibrary::SetVectorParameterValue(World, Collection, TEXT("AccentColor"), CurrentPalette.Accent);

	// Pulso do sol: 0 -> 1 -> 0 (seno) ao longo de SunPulseTime
	const float PulseT = SunPulseTime > 0.f ? 1.f - SunPulse / SunPulseTime : 1.f;
	const float SunPulseValue = SunPulse > 0.f ? FMath::Sin(PulseT * PI) : 0.f;

	UKismetMaterialLibrary::SetScalarParameterValue(World, Collection, TEXT("FloorOffset"), FloorOffset);
	UKismetMaterialLibrary::SetScalarParameterValue(World, Collection, TEXT("ComboGlow"), ComboGlow);
	UKismetMaterialLibrary::SetScalarParameterValue(World, Collection, TEXT("SunPulse"), SunPulseValue);
}
