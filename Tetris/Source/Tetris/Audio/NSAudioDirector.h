// Neon Stack - áudio: SFX afinados na escala da música + música reativa (filtro/volume por nível, combo, pausa).
// Usa só recursos nativos do UAudioComponent (low-pass, volume, pitch). Não exige MetaSound.
// Spec: audio-bible §3-7

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Data/NSSaveGame.h"
#include "Simulation/NSTypes.h"
#include "Game/NSGameMode.h"
#include "NSAudioDirector.generated.h"

class ANSBoardActor;
class UAudioComponent;
class USoundBase;
class USoundClass;
class USoundMix;

UCLASS()
class TETRIS_API ANSAudioDirector : public AActor
{
	GENERATED_BODY()

public:
	ANSAudioDirector();

	/** Primeiro AudioDirector do mapa (ou nullptr). */
	static ANSAudioDirector* Get(const UObject* WorldContext);

	// --- Mixagem (audio-bible §6) ---
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NeonStack|Mix") TObjectPtr<USoundMix> VolumeMix;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NeonStack|Mix") TObjectPtr<USoundClass> MusicClass;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NeonStack|Mix") TObjectPtr<USoundClass> SfxClass;

	// --- Música (audio-bible §3) ---
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NeonStack|Music") TObjectPtr<USoundBase> Music;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NeonStack|Music") float CutoffLevel1 = 2500.f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NeonStack|Music") float CutoffOpen = 20000.f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NeonStack|Music") int32 OpenAtLevel = 10;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NeonStack|Music") float CutoffMenu = 1200.f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NeonStack|Music") float CutoffPaused = 600.f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NeonStack|Music") float VolumeLevel1Db = -6.f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NeonStack|Music") float VolumeMenuDb = -8.f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NeonStack|Music") float VolumePausedDb = -10.f;
	/** "Brilho" do combo: +dB por combo, até o máximo. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NeonStack|Music") float ComboBoostDbPerCombo = 0.75f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NeonStack|Music") float ComboBoostDbMax = 3.f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NeonStack|Music") float QuadDuckDb = -4.f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NeonStack|Music", meta = (Units = "s")) float QuadDuckTime = 0.4f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NeonStack|Music", meta = (Units = "s")) float SmoothTime = 0.3f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NeonStack|Music", meta = (Units = "s")) float TapeStopTime = 1.2f;

	// --- SFX (audio-bible §5). Vazios são ignorados. ---
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NeonStack|SFX") TObjectPtr<USoundBase> SfxMove;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NeonStack|SFX") TObjectPtr<USoundBase> SfxRotate;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NeonStack|SFX") TObjectPtr<USoundBase> SfxHardDrop;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NeonStack|SFX") TObjectPtr<USoundBase> SfxLock;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NeonStack|SFX") TObjectPtr<USoundBase> SfxLockWarn;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NeonStack|SFX") TObjectPtr<USoundBase> SfxHold;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NeonStack|SFX") TObjectPtr<USoundBase> SfxHoldDenied;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NeonStack|SFX") TObjectPtr<USoundBase> SfxClear;      // tonal, afinado em Lá
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NeonStack|SFX") TObjectPtr<USoundBase> SfxQuad;       // tonal
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NeonStack|SFX") TObjectPtr<USoundBase> SfxComboTick;  // tonal
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NeonStack|SFX") TObjectPtr<USoundBase> SfxLevelUp;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NeonStack|SFX") TObjectPtr<USoundBase> SfxGameOver;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NeonStack|SFX") TObjectPtr<USoundBase> SfxNewRecord;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NeonStack|SFX") TObjectPtr<USoundBase> SfxCountdown;  // tonal

	/** Semitons para afinar os samples tonais no tom da música (samples em Lá; música em Dó menor = +3). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NeonStack|SFX") int32 RootOffsetSemitones = 0;

	/** Variação aleatória de pitch nos sons não tonais (±). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NeonStack|SFX") float PitchJitter = 0.03f;

	// --- Chamados pelos widgets ---
	UFUNCTION(BlueprintCallable, Category = "NeonStack|Audio") void PlayCountdown(int32 Number);
	UFUNCTION(BlueprintCallable, Category = "NeonStack|Audio") void PlayNewRecord();

	/** Pitch para um grau da escala pentatônica (1..8), já com RootOffsetSemitones. */
	UFUNCTION(BlueprintPure, Category = "NeonStack|Audio") float GetScalePitch(int32 Degree) const;

	virtual void Tick(float DeltaSeconds) override;

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "NeonStack")
	TObjectPtr<UAudioComponent> MusicComponent;

private:
	UFUNCTION() void HandlePieceMoved(const TArray<FIntPoint>& Cells, const TArray<FIntPoint>& Ghost, bool bByGravity);
	UFUNCTION() void HandlePieceRotated(const TArray<FIntPoint>& Cells, const TArray<FIntPoint>& Ghost, int32 KickIndex);
	UFUNCTION() void HandlePieceHardDropped(int32 FromY, int32 ToY, const TArray<FIntPoint>& Cells);
	UFUNCTION() void HandlePieceLocked(ENSPieceType Type, const TArray<FIntPoint>& Cells);
	UFUNCTION() void HandleLinesCleared(const TArray<int32>& Rows, int32 Count, int32 Combo, int32 ScoreGained);
	UFUNCTION() void HandleLevelUp(int32 NewLevel);
	UFUNCTION() void HandleHoldChanged(ENSPieceType Held, bool bCanHold);
	UFUNCTION() void HandleHoldDenied();
	UFUNCTION() void HandleLockWarning(float Remaining);
	UFUNCTION() void HandleScoreChanged(int64 Score, int32 Lines, int32 InLevel);
	UFUNCTION() void HandleGameOver(int64 Score, int32 Lines, int32 InLevel, bool bNewHighScore);
	UFUNCTION() void HandleFlowStateChanged(ENSFlowState NewState);
	UFUNCTION() void HandleSettingsChanged(const FNSSettings& Settings);

	void PlaySfx(USoundBase* Sound, float Pitch = 1.f, float Volume = 1.f);
	void PlayJittered(USoundBase* Sound);
	void ApplyVolumes(const FNSSettings& Settings);
	void EnsureMusicPlaying();
	void UpdateMusicTargets();

	TWeakObjectPtr<ANSBoardActor> Board;
	bool bMenuMode = false;
	bool bPaused = false;
	bool bJustHardDropped = false;
	int32 Level = 1;

	// Estado da música
	float CurrentCutoff = 2500.f;
	float TargetCutoff = 2500.f;
	float CurrentVolumeDb = -6.f;
	float TargetVolumeDb = -6.f;
	float ComboBoostDb = 0.f;
	float DuckTimeLeft = 0.f;
	float TapeStopLeft = 0.f;   // > 0 durante o "tape stop"
};
