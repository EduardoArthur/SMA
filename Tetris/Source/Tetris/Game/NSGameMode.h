// Neon Stack - fluxo da partida.
// M1: inicia o jogo no BeginPlay, pausa com Esc e reinicia sozinho após o game over.
// M3 completa o fluxo (contagem, resultados, menu): cpp-specs/02-gameplay.md §7

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

	/** M1: tempo entre o game over e a nova partida automática. */
	UPROPERTY(EditDefaultsOnly, Category = "NeonStack")
	float AutoRestartDelay = 2.0f;

	UFUNCTION(BlueprintPure, Category = "NeonStack")
	ENSFlowState GetFlowState() const { return FlowState; }

	UFUNCTION(BlueprintPure, Category = "NeonStack")
	ANSBoardActor* GetBoard() const { return Board.Get(); }

	UFUNCTION(BlueprintCallable, Category = "NeonStack")
	void TogglePause();

	UFUNCTION(BlueprintCallable, Category = "NeonStack")
	void RestartGame();

	UPROPERTY(BlueprintAssignable, Category = "NeonStack")
	FNSOnFlowStateChanged OnFlowStateChanged;

protected:
	virtual void BeginPlay() override;

private:
	void SetFlowState(ENSFlowState NewState);

	UFUNCTION()
	void HandleGameOver(int64 Score, int32 Lines, int32 Level, bool bNewHighScore);

	TWeakObjectPtr<ANSBoardActor> Board;
	ENSFlowState FlowState = ENSFlowState::Countdown;
	FTimerHandle RestartTimer;
};
