#include "Audio/NSAudioDirector.h"

#include "Tetris.h"
#include "EngineUtils.h"
#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"
#include "Sound/SoundClass.h"
#include "Sound/SoundMix.h"
#include "Game/NSBoardActor.h"
#include "Systems/NSSaveSubsystem.h"

namespace NSAudio
{
	// Lá menor pentatônica: A C D E G A' C' D' (audio-bible §4.1)
	static const int32 ScaleSemitones[8] = { 0, 3, 5, 7, 10, 12, 15, 17 };

	static float DbToLinear(float Db) { return FMath::Pow(10.f, Db / 20.f); }

	/** Menor pitch aceito pelo UAudioComponent. */
	constexpr float MinPitch = 0.4f;
}

ANSAudioDirector::ANSAudioDirector()
{
	PrimaryActorTick.bCanEverTick = true;
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));

	MusicComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("Music"));
	MusicComponent->SetupAttachment(RootComponent);
	MusicComponent->bAutoActivate = false;
	MusicComponent->bIsUISound = true; // continua tocando independente de pausa do mundo
}

ANSAudioDirector* ANSAudioDirector::Get(const UObject* WorldContext)
{
	UWorld* World = WorldContext ? WorldContext->GetWorld() : nullptr;
	if (!World)
	{
		return nullptr;
	}
	for (TActorIterator<ANSAudioDirector> It(World); It; ++It)
	{
		return *It;
	}
	return nullptr;
}

void ANSAudioDirector::BeginPlay()
{
	Super::BeginPlay();

	for (TActorIterator<ANSBoardActor> It(GetWorld()); It; ++It)
	{
		Board = *It;
		break;
	}
	bMenuMode = !Board.IsValid();

	if (ANSBoardActor* B = Board.Get())
	{
		B->OnPieceMoved.AddDynamic(this, &ANSAudioDirector::HandlePieceMoved);
		B->OnPieceRotated.AddDynamic(this, &ANSAudioDirector::HandlePieceRotated);
		B->OnPieceHardDropped.AddDynamic(this, &ANSAudioDirector::HandlePieceHardDropped);
		B->OnPieceLocked.AddDynamic(this, &ANSAudioDirector::HandlePieceLocked);
		B->OnLinesCleared.AddDynamic(this, &ANSAudioDirector::HandleLinesCleared);
		B->OnLevelUp.AddDynamic(this, &ANSAudioDirector::HandleLevelUp);
		B->OnHoldChanged.AddDynamic(this, &ANSAudioDirector::HandleHoldChanged);
		B->OnHoldDenied.AddDynamic(this, &ANSAudioDirector::HandleHoldDenied);
		B->OnLockWarning.AddDynamic(this, &ANSAudioDirector::HandleLockWarning);
		B->OnScoreChanged.AddDynamic(this, &ANSAudioDirector::HandleScoreChanged);
		B->OnGameOver.AddDynamic(this, &ANSAudioDirector::HandleGameOver);
	}
	if (ANSGameMode* GM = GetWorld()->GetAuthGameMode<ANSGameMode>())
	{
		GM->OnFlowStateChanged.AddDynamic(this, &ANSAudioDirector::HandleFlowStateChanged);
	}

	if (VolumeMix)
	{
		UGameplayStatics::PushSoundMixModifier(this, VolumeMix);
	}
	if (UNSSaveSubsystem* Save = UNSSaveSubsystem::Get(this))
	{
		Save->OnSettingsChanged.AddDynamic(this, &ANSAudioDirector::HandleSettingsChanged);
		ApplyVolumes(Save->GetSettings());
	}

	UpdateMusicTargets();
	CurrentCutoff = TargetCutoff;
	CurrentVolumeDb = TargetVolumeDb;
	EnsureMusicPlaying();
}

// ---------------------------------------------------------------------------
// Música
// ---------------------------------------------------------------------------

void ANSAudioDirector::EnsureMusicPlaying()
{
	if (!Music || !MusicComponent)
	{
		return;
	}
	TapeStopLeft = 0.f;
	MusicComponent->SetPitchMultiplier(1.f);
	if (!MusicComponent->IsPlaying())
	{
		MusicComponent->SetSound(Music);
		if (MusicClass)
		{
			MusicComponent->SoundClassOverride = MusicClass;
		}
		MusicComponent->SetLowPassFilterEnabled(true);
		MusicComponent->SetLowPassFilterFrequency(CurrentCutoff);
		MusicComponent->SetVolumeMultiplier(NSAudio::DbToLinear(CurrentVolumeDb));
		MusicComponent->Play();
	}
}

void ANSAudioDirector::UpdateMusicTargets()
{
	if (bMenuMode)
	{
		TargetCutoff = CutoffMenu;
		TargetVolumeDb = VolumeMenuDb;
		return;
	}
	if (bPaused)
	{
		TargetCutoff = CutoffPaused;
		TargetVolumeDb = VolumePausedDb;
		return;
	}
	// Nível 1 -> OpenAtLevel: cutoff sobe em escala logarítmica, volume sobe linear em dB
	const float T = FMath::Clamp((Level - 1) / static_cast<float>(FMath::Max(1, OpenAtLevel - 1)), 0.f, 1.f);
	TargetCutoff = FMath::Exp(FMath::Lerp(FMath::Loge(CutoffLevel1), FMath::Loge(CutoffOpen), T));
	TargetVolumeDb = FMath::Lerp(VolumeLevel1Db, 0.f, T);
}

void ANSAudioDirector::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (!MusicComponent || !MusicComponent->IsPlaying())
	{
		return;
	}

	// Suavização (interpola o cutoff em espaço logarítmico)
	const float Alpha = 1.f - FMath::Exp(-DeltaSeconds / FMath::Max(SmoothTime, KINDA_SMALL_NUMBER));
	CurrentCutoff = FMath::Exp(FMath::Lerp(FMath::Loge(CurrentCutoff), FMath::Loge(TargetCutoff), Alpha));
	CurrentVolumeDb = FMath::Lerp(CurrentVolumeDb, TargetVolumeDb, Alpha);

	ComboBoostDb = FMath::Max(0.f, ComboBoostDb - DeltaSeconds * 1.5f);
	DuckTimeLeft = FMath::Max(0.f, DuckTimeLeft - DeltaSeconds);
	const float DuckDb = DuckTimeLeft > 0.f ? QuadDuckDb : 0.f;

	MusicComponent->SetLowPassFilterFrequency(CurrentCutoff);
	MusicComponent->SetVolumeMultiplier(NSAudio::DbToLinear(CurrentVolumeDb + ComboBoostDb + DuckDb));

	// Tape stop do game over
	if (TapeStopLeft > 0.f)
	{
		TapeStopLeft -= DeltaSeconds;
		const float T = 1.f - FMath::Clamp(TapeStopLeft / FMath::Max(TapeStopTime, KINDA_SMALL_NUMBER), 0.f, 1.f);
		MusicComponent->SetPitchMultiplier(FMath::Lerp(1.f, NSAudio::MinPitch, T));
		if (TapeStopLeft <= 0.f)
		{
			MusicComponent->FadeOut(0.2f, 0.f);
		}
	}
}

void ANSAudioDirector::HandleFlowStateChanged(ENSFlowState NewState)
{
	switch (NewState)
	{
	case ENSFlowState::Countdown:
	case ENSFlowState::Playing:
		bPaused = false;
		EnsureMusicPlaying();
		break;
	case ENSFlowState::Paused:
		bPaused = true;
		break;
	default:
		break;
	}
	UpdateMusicTargets();
}

void ANSAudioDirector::HandleSettingsChanged(const FNSSettings& Settings)
{
	ApplyVolumes(Settings);
}

void ANSAudioDirector::ApplyVolumes(const FNSSettings& Settings)
{
	if (!VolumeMix)
	{
		return;
	}
	if (MusicClass)
	{
		UGameplayStatics::SetSoundMixClassOverride(this, VolumeMix, MusicClass, Settings.MusicVolume / 100.f, 1.f, 0.f, true);
	}
	if (SfxClass)
	{
		UGameplayStatics::SetSoundMixClassOverride(this, VolumeMix, SfxClass, Settings.SfxVolume / 100.f, 1.f, 0.f, true);
	}
}

// ---------------------------------------------------------------------------
// SFX
// ---------------------------------------------------------------------------

float ANSAudioDirector::GetScalePitch(int32 Degree) const
{
	const int32 Index = FMath::Clamp(Degree - 1, 0, 7);
	return FMath::Pow(2.f, (NSAudio::ScaleSemitones[Index] + RootOffsetSemitones) / 12.f);
}

void ANSAudioDirector::PlaySfx(USoundBase* Sound, float Pitch, float Volume)
{
	if (!Sound)
	{
		return;
	}
	if (UAudioComponent* AC = UGameplayStatics::CreateSound2D(this, Sound, Volume, Pitch, 0.f, nullptr, false, true))
	{
		if (SfxClass)
		{
			AC->SoundClassOverride = SfxClass;
		}
		AC->Play();
	}
}

void ANSAudioDirector::PlayJittered(USoundBase* Sound)
{
	PlaySfx(Sound, 1.f + FMath::FRandRange(-PitchJitter, PitchJitter));
}

void ANSAudioDirector::HandlePieceMoved(const TArray<FIntPoint>& Cells, const TArray<FIntPoint>& Ghost, bool bByGravity)
{
	if (!bByGravity)
	{
		PlayJittered(SfxMove);
	}
}

void ANSAudioDirector::HandlePieceRotated(const TArray<FIntPoint>& Cells, const TArray<FIntPoint>& Ghost, int32 KickIndex)
{
	PlayJittered(SfxRotate);
}

void ANSAudioDirector::HandlePieceHardDropped(int32 FromY, int32 ToY, const TArray<FIntPoint>& Cells)
{
	bJustHardDropped = true; // o PieceLocked que vem logo em seguida não toca o "thump"
	PlayJittered(SfxHardDrop);
}

void ANSAudioDirector::HandlePieceLocked(ENSPieceType Type, const TArray<FIntPoint>& Cells)
{
	if (bJustHardDropped)
	{
		bJustHardDropped = false;
		return;
	}
	PlayJittered(SfxLock);
}

void ANSAudioDirector::HandleLinesCleared(const TArray<int32>& Rows, int32 Count, int32 Combo, int32 ScoreGained)
{
	const int32 SafeCombo = FMath::Max(0, Combo);
	if (Count >= 4)
	{
		PlaySfx(SfxQuad, GetScalePitch(1 + SafeCombo));
		DuckTimeLeft = QuadDuckTime;
	}
	else
	{
		// Single = grau 1, Double = 2, Triple = 3; cada combo sobe mais um grau (audio-bible §4.2)
		PlaySfx(SfxClear, GetScalePitch(Count + SafeCombo));
	}

	if (SafeCombo >= 1)
	{
		PlaySfx(SfxComboTick, GetScalePitch(1 + SafeCombo));
		ComboBoostDb = FMath::Min(ComboBoostDbMax, SafeCombo * ComboBoostDbPerCombo);
	}
}

void ANSAudioDirector::HandleLevelUp(int32 NewLevel)
{
	PlaySfx(SfxLevelUp, GetScalePitch(1));
}

void ANSAudioDirector::HandleHoldChanged(ENSPieceType Held, bool bCanHold)
{
	if (!bCanHold) // só a ação de hold bloqueia; o desbloqueio ao travar não toca som
	{
		PlaySfx(SfxHold);
	}
}

void ANSAudioDirector::HandleHoldDenied()
{
	PlaySfx(SfxHoldDenied);
}

void ANSAudioDirector::HandleLockWarning(float Remaining)
{
	PlaySfx(SfxLockWarn);
}

void ANSAudioDirector::HandleScoreChanged(int64 Score, int32 Lines, int32 InLevel)
{
	if (InLevel != Level)
	{
		Level = InLevel;
		UpdateMusicTargets();
	}
}

void ANSAudioDirector::HandleGameOver(int64 Score, int32 Lines, int32 InLevel, bool bNewHighScore)
{
	PlaySfx(SfxGameOver);
	TapeStopLeft = TapeStopTime;
}

void ANSAudioDirector::PlayCountdown(int32 Number)
{
	PlaySfx(SfxCountdown, GetScalePitch(Number <= 1 ? 6 : 1)); // 3 e 2 em Lá, 1 em Lá (8ª)
}

void ANSAudioDirector::PlayNewRecord()
{
	PlaySfx(SfxNewRecord);
}
