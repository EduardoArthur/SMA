// Neon Stack - "juice" visual: Niagara, camera shakes, flash, pulsos de bloco e pós-processamento.
// Escuta o ANSBoardActor; respeita as opções de Intensidade de FX e Screen shake.
// Spec: art-bible §6-7 · GDD §6

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Data/NSSaveGame.h"
#include "Simulation/NSTypes.h"
#include "NSFXDirector.generated.h"

class ANSBoardActor;
class ANSBackgroundActor;
class APostProcessVolume;
class UCameraShakeBase;
class UMaterialParameterCollection;
class UNiagaraSystem;

UCLASS()
class TETRIS_API ANSFXDirector : public AActor
{
	GENERATED_BODY()

public:
	ANSFXDirector();

	// --- Niagara (opcionais: o que estiver vazio é ignorado) ---
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NeonStack|Niagara") TObjectPtr<UNiagaraSystem> LineClearFX;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NeonStack|Niagara") TObjectPtr<UNiagaraSystem> QuadBurstFX;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NeonStack|Niagara") TObjectPtr<UNiagaraSystem> HardDropTrailFX;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NeonStack|Niagara") TObjectPtr<UNiagaraSystem> LockSparkFX;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NeonStack|Niagara") TObjectPtr<UNiagaraSystem> LevelUpSweepFX;

	/** Profundidade (X) dos efeitos, à frente dos blocos (a câmera está em X negativo). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NeonStack|Niagara") float FXDepthOffset = -60.f;

	// --- Camera shakes ---
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NeonStack|Shake") TSubclassOf<UCameraShakeBase> HardDropShake;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NeonStack|Shake") TSubclassOf<UCameraShakeBase> QuadShake;

	/** Sem classes de shake: tremor procedural da câmera (amplitude em unidades do mundo). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NeonStack|Shake") float HardDropShakeAmplitude = 5.f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NeonStack|Shake", meta = (Units = "s")) float HardDropShakeDuration = 0.08f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NeonStack|Shake") float QuadShakeAmplitude = 18.f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NeonStack|Shake", meta = (Units = "s")) float QuadShakeDuration = 0.25f;

	// --- Flash / pós-processamento ---
	/** MPC_NeonStack: escreve o escalar "Flash". */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NeonStack|Flash") TObjectPtr<UMaterialParameterCollection> Collection;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NeonStack|Flash") float QuadFlashStrength = 1.0f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NeonStack|Flash", meta = (Units = "s")) float FlashDecayTime = 0.12f;

	/** PostProcessVolume do mapa (opcional): bloom por intensidade de FX e aberração cromática no Quad. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NeonStack|PostProcess") TObjectPtr<APostProcessVolume> PostProcess;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NeonStack|PostProcess") float BaseBloomIntensity = 1.2f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NeonStack|PostProcess") float BaseFringe = 0.1f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NeonStack|PostProcess") float QuadFringe = 0.6f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NeonStack|PostProcess", meta = (Units = "s")) float FringeDecayTime = 0.15f;

	// --- Blocos ---
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NeonStack|Blocks") float LockPulseStrength = 1.0f;

	virtual void Tick(float DeltaSeconds) override;

protected:
	virtual void BeginPlay() override;

	/** Gancho para efeitos extras no Blueprint. */
	UFUNCTION(BlueprintImplementableEvent, Category = "NeonStack")
	void OnQuad(FVector Center);

private:
	UFUNCTION() void HandlePieceHardDropped(int32 FromY, int32 ToY, const TArray<FIntPoint>& Cells);
	UFUNCTION() void HandlePieceLocked(ENSPieceType Type, const TArray<FIntPoint>& Cells);
	UFUNCTION() void HandleLinesCleared(const TArray<int32>& Rows, int32 Count, int32 Combo, int32 ScoreGained);
	UFUNCTION() void HandleLevelUp(int32 NewLevel);
	UFUNCTION() void HandleSettingsChanged(const FNSSettings& NewSettings);

	void SpawnFX(UNiagaraSystem* System, const FVector& Location, float Intensity, float Length = 0.f, const FLinearColor& Color = FLinearColor::White);
	void PlayShake(TSubclassOf<UCameraShakeBase> Shake, float Amplitude, float Duration);
	void UpdateProceduralShake(float DeltaSeconds);

	TWeakObjectPtr<AActor> ShakeTarget;
	FVector ShakeBaseLocation = FVector::ZeroVector;
	float ShakeTimeLeft = 0.f;
	float ShakeDuration = 0.f;
	float ShakeAmplitude = 0.f;
	FVector BoardPoint(float CellX, float CellY) const;
	void ApplyPostProcess();

	TWeakObjectPtr<ANSBoardActor> Board;
	TWeakObjectPtr<ANSBackgroundActor> Background;
	FNSSettings Settings;

	float Flash = 0.f;
	float FringeBoost = 0.f;
};
