// Neon Stack - controla o fundo synthwave escrevendo numa Material Parameter Collection (MPC_NeonStack).
// Os materiais do céu, sol, chão e poço leem a MPC. Funciona no jogo e no menu (sem board = paleta 0, rolagem lenta).
// Spec: art-bible §4.3, §5 · levels §3

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Data/NSPaletteData.h"
#include "Data/NSSaveGame.h"
#include "NSBackgroundActor.generated.h"

class UMaterialParameterCollection;
class ANSBoardActor;

UCLASS()
class TETRIS_API ANSBackgroundActor : public AActor
{
	GENERATED_BODY()

public:
	ANSBackgroundActor();

	/** MPC_NeonStack. Parâmetros escritos: ver guia M4 §2. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NeonStack")
	TObjectPtr<UMaterialParameterCollection> Collection;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NeonStack")
	TObjectPtr<UNSPaletteData> Palettes;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NeonStack", meta = (ClampMin = 1))
	int32 PaletteEveryNLevels = 5;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NeonStack", meta = (Units = "s"))
	float PaletteTransitionTime = 1.0f;

	/** Rolagem do chão: Base + PerLevel × (nível-1), até Max (art-bible §5). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NeonStack") float ScrollBase = 0.2f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NeonStack") float ScrollPerLevel = 0.05f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NeonStack") float ScrollMax = 1.0f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NeonStack") float ScrollPausedFactor = 0.5f;

	/** Brilho extra do chão por combo (máx. ComboGlowMax), decai em ComboGlowDecayTime. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NeonStack") float ComboGlowPerCombo = 0.1f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NeonStack") float ComboGlowMax = 0.5f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NeonStack", meta = (Units = "s")) float ComboGlowDecayTime = 1.5f;

	/** Pulso do sol no Quad (0 -> 1 -> 0). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NeonStack", meta = (Units = "s")) float SunPulseTime = 0.3f;

	/** Dispara o pulso do sol (também chamado pelo FXDirector). */
	UFUNCTION(BlueprintCallable, Category = "NeonStack")
	void PulseSun();

	virtual void Tick(float DeltaSeconds) override;

protected:
	virtual void BeginPlay() override;

private:
	UFUNCTION() void HandleLevelUp(int32 NewLevel);
	UFUNCTION() void HandleScoreChanged(int64 Score, int32 Lines, int32 Level);
	UFUNCTION() void HandleLinesCleared(const TArray<int32>& Rows, int32 Count, int32 Combo, int32 ScoreGained);
	UFUNCTION() void HandlePausedChanged(bool bInPaused);
	UFUNCTION() void HandleSettingsChanged(const FNSSettings& Settings);

	void SetLevel(int32 Level, bool bInstant);
	void WriteParameters();

	TWeakObjectPtr<ANSBoardActor> Board;

	int32 CurrentLevel = 1;
	int32 TargetPaletteIndex = 0;
	FNSPalette FromPalette;
	FNSPalette CurrentPalette;
	float PaletteAlpha = 1.f;

	float ScrollSpeed = 0.2f;
	float FloorOffset = 0.f;
	float ComboGlow = 0.f;
	float SunPulse = 0.f;     // tempo restante do pulso
	bool bPaused = false;
	bool bReducedMotion = false; // FX Baixa: rolagem lenta fixa
};
