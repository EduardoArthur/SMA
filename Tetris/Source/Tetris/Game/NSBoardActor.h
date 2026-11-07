// Neon Stack - Actor do poço: roda a simulação e transmite os eventos para os Blueprints.
// Spec: _gdks-output/04-engine/cpp-specs/02-gameplay.md §4

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Simulation/NSGameSimulation.h"
#include "NSBoardActor.generated.h"

class UNSGameConfig;
class UNSBoardViewComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FNSOnPieceSpawned, ENSPieceType, Type, const TArray<FIntPoint>&, Cells);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FNSOnPieceMoved, const TArray<FIntPoint>&, Cells, const TArray<FIntPoint>&, Ghost, bool, bByGravity);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FNSOnPieceRotated, const TArray<FIntPoint>&, Cells, const TArray<FIntPoint>&, Ghost, int32, KickIndex);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FNSOnPieceHardDropped, int32, FromY, int32, ToY, const TArray<FIntPoint>&, Cells);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FNSOnPieceLocked, ENSPieceType, Type, const TArray<FIntPoint>&, Cells);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FNSOnLinesCleared, const TArray<int32>&, Rows, int32, Count, int32, Combo, int32, ScoreGained);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FNSOnScoreChanged, int64, Score, int32, Lines, int32, Level);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FNSOnLevelUp, int32, NewLevel);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FNSOnHoldChanged, ENSPieceType, Held, bool, bCanHold);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FNSOnHoldDenied);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FNSOnQueueChanged, const TArray<ENSPieceType>&, Next);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FNSOnLockWarning, float, Remaining);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FNSOnPausedChanged, bool, bPaused);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FNSOnGameOver, int64, Score, int32, Lines, int32, Level, bool, bNewHighScore);

UCLASS()
class TETRIS_API ANSBoardActor : public AActor
{
	GENERATED_BODY()

public:
	ANSBoardActor();

	// --- Configuração ---
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NeonStack")
	TObjectPtr<UNSGameConfig> Config;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "NeonStack")
	TObjectPtr<UNSBoardViewComponent> View;

	// --- Controle ---
	UFUNCTION(BlueprintCallable, Category = "NeonStack")
	void StartGame();

	UFUNCTION(BlueprintCallable, Category = "NeonStack")
	void StopGame();

	UFUNCTION(BlueprintCallable, Category = "NeonStack")
	void SetPaused(bool bInPaused);

	UFUNCTION(BlueprintCallable, Category = "NeonStack")
	void PressAction(ENSInputAction Action);

	UFUNCTION(BlueprintCallable, Category = "NeonStack")
	void ReleaseAction(ENSInputAction Action);

	/** Sobrescreve DAS/ARR (opções do jogador). */
	UFUNCTION(BlueprintCallable, Category = "NeonStack")
	void SetHandling(int32 DasMs, int32 ArrMs);

	// --- Leitura ---
	UFUNCTION(BlueprintPure, Category = "NeonStack")
	bool IsRunning() const { return bRunning; }

	UFUNCTION(BlueprintPure, Category = "NeonStack")
	bool IsPaused() const { return bPaused; }

	UFUNCTION(BlueprintPure, Category = "NeonStack")
	ENSSimState GetSimState() const { return Sim.GetState(); }

	UFUNCTION(BlueprintPure, Category = "NeonStack")
	int64 GetScore() const { return Sim.GetScoring().Score; }

	UFUNCTION(BlueprintPure, Category = "NeonStack")
	int32 GetLines() const { return Sim.GetScoring().Lines; }

	UFUNCTION(BlueprintPure, Category = "NeonStack")
	int32 GetLevel() const { return Sim.GetScoring().Level; }

	/** (Level - 1) / PaletteEveryNLevels. */
	UFUNCTION(BlueprintPure, Category = "NeonStack")
	int32 GetPaletteIndex() const;

	/** Posição no mundo do centro de uma célula (para spawnar FX). */
	UFUNCTION(BlueprintPure, Category = "NeonStack")
	FVector CellToWorld(FIntPoint Cell) const;

	const FNSGameSimulation& GetSimulation() const { return Sim; }

	// --- Eventos ---
	UPROPERTY(BlueprintAssignable, Category = "NeonStack|Events") FNSOnPieceSpawned OnPieceSpawned;
	UPROPERTY(BlueprintAssignable, Category = "NeonStack|Events") FNSOnPieceMoved OnPieceMoved;
	UPROPERTY(BlueprintAssignable, Category = "NeonStack|Events") FNSOnPieceRotated OnPieceRotated;
	UPROPERTY(BlueprintAssignable, Category = "NeonStack|Events") FNSOnPieceHardDropped OnPieceHardDropped;
	UPROPERTY(BlueprintAssignable, Category = "NeonStack|Events") FNSOnPieceLocked OnPieceLocked;
	UPROPERTY(BlueprintAssignable, Category = "NeonStack|Events") FNSOnLinesCleared OnLinesCleared;
	UPROPERTY(BlueprintAssignable, Category = "NeonStack|Events") FNSOnScoreChanged OnScoreChanged;
	UPROPERTY(BlueprintAssignable, Category = "NeonStack|Events") FNSOnLevelUp OnLevelUp;
	UPROPERTY(BlueprintAssignable, Category = "NeonStack|Events") FNSOnHoldChanged OnHoldChanged;
	UPROPERTY(BlueprintAssignable, Category = "NeonStack|Events") FNSOnHoldDenied OnHoldDenied;
	UPROPERTY(BlueprintAssignable, Category = "NeonStack|Events") FNSOnQueueChanged OnQueueChanged;
	UPROPERTY(BlueprintAssignable, Category = "NeonStack|Events") FNSOnLockWarning OnLockWarning;
	UPROPERTY(BlueprintAssignable, Category = "NeonStack|Events") FNSOnPausedChanged OnPausedChanged;
	UPROPERTY(BlueprintAssignable, Category = "NeonStack|Events") FNSOnGameOver OnGameOver;

	virtual void Tick(float DeltaSeconds) override;

protected:
	virtual void BeginPlay() override;

private:
	void ProcessEvents();
	void BroadcastEvent(const FNSGameEvent& E);
	void SetHeld(ENSInputAction Action, bool bHeld);

	FNSGameSimulation Sim;
	FNSInputFrame PendingInput;
	TArray<FNSGameEvent> EventBuffer;
	bool bRunning = false;
	bool bPaused = false;

	static constexpr float MaxDeltaTime = 0.1f;
};
