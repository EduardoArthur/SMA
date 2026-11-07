// Neon Stack - fluxo da partida: contagem -> jogo <-> pausa -> game over -> resultados.
// Spec: _gdks-output/04-engine/cpp-specs/02-gameplay.md §7

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "NSGameMode.generated.h"

class ANSBoardActor;

UENUM(BlueprintType)
enum class ENSFlowState : uint8
{
	Countdown,
	Playing,
	Paused,
	GameOver,
	Results
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FNSOnFlowStateChanged, ENSFlowState, NewState);

UCLASS()
class TETRIS_API ANSGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ANSGameMode();

	/** Mapa do menu principal (L_MainMenu). */
	UPROPERTY(EditDefaultsOnly, Category = "NeonStack")
	TSoftObjectPtr<UWorld> MainMenuLevel;

	/** Duração da animação de game over antes da tela de resultados. */
	UPROPERTY(EditDefaultsOnly, Category = "NeonStack", meta = (ClampMin = 0, Units = "s"))
	float GameOverToResultsDelay = 1.5f;

	UFUNCTION(BlueprintPure, Category = "NeonStack")
	ENSFlowState GetFlowState() const { return FlowState; }

	UFUNCTION(BlueprintPure, Category = "NeonStack")
	ANSBoardActor* GetBoard() const { return Board.Get(); }

	/** Chamado pelo widget de contagem ao terminar o 3-2-1: inicia a partida ou retoma da pausa. */
	UFUNCTION(BlueprintCallable, Category = "NeonStack")
	void FinishCountdown();

	UFUNCTION(BlueprintCallable, Category = "NeonStack")
	void TogglePause();

	UFUNCTION(BlueprintCallable, Category = "NeonStack")
	void PauseGame();

	/** Pausa -> contagem -> jogo. */
	UFUNCTION(BlueprintCallable, Category = "NeonStack")
	void ResumeGame();

	/** Nova partida (passa pela contagem). */
	UFUNCTION(BlueprintCallable, Category = "NeonStack")
	void RestartGame();

	UFUNCTION(BlueprintCallable, Category = "NeonStack")
	void ReturnToMenu();

	// --- Resultado da última partida (para a tela de resultados) ---
	UFUNCTION(BlueprintPure, Category = "NeonStack")
	int64 GetLastScore() const { return LastScore; }

	UFUNCTION(BlueprintPure, Category = "NeonStack")
	int32 GetLastLines() const { return LastLines; }

	UFUNCTION(BlueprintPure, Category = "NeonStack")
	int32 GetLastLevel() const { return LastLevel; }

	UFUNCTION(BlueprintPure, Category = "NeonStack")
	bool WasLastNewHighScore() const { return bLastNewHighScore; }

	UPROPERTY(BlueprintAssignable, Category = "NeonStack")
	FNSOnFlowStateChanged OnFlowStateChanged;

	/** Cria board/câmera/luz se o mapa não tiver (NSSceneBootstrap). */
	virtual void StartPlay() override;

protected:
	virtual void BeginPlay() override;

private:
	void SetFlowState(ENSFlowState NewState);
	void ShowResults();

	UFUNCTION()
	void HandleGameOver(int64 Score, int32 Lines, int32 Level, bool bNewHighScore);

	TWeakObjectPtr<ANSBoardActor> Board;
	ENSFlowState FlowState = ENSFlowState::Countdown;
	bool bStartPending = true; // a contagem atual é de início (true) ou de retomada (false)
	FTimerHandle ResultsTimer;

	int64 LastScore = 0;
	int32 LastLines = 0;
	int32 LastLevel = 0;
	bool bLastNewHighScore = false;
};
