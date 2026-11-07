// Neon Stack - Enhanced Input -> ANSBoardActor.
// Spec: _gdks-output/04-engine/cpp-specs/02-gameplay.md §6

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "Simulation/NSTypes.h"
#include "Game/NSGameMode.h"
#include "NSPlayerController.generated.h"

class UInputAction;
class UInputMappingContext;
class ANSBoardActor;
class UNSHUDWidget;
class UNSCountdownWidget;
class UNSPauseWidget;
class UNSResultsWidget;
class UNSOptionsWidget;

UCLASS()
class TETRIS_API ANSPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, Category = "NeonStack|Input")
	TObjectPtr<UInputMappingContext> GameplayContext;

	/** Contexto dos menus (pausa/resultados). Deve conter IA_Pause para o Esc retomar o jogo. */
	UPROPERTY(EditDefaultsOnly, Category = "NeonStack|Input")
	TObjectPtr<UInputMappingContext> UIContext;

	// --- Widgets (WBP_*) ---
	UPROPERTY(EditDefaultsOnly, Category = "NeonStack|UI") TSubclassOf<UNSHUDWidget> HUDClass;
	UPROPERTY(EditDefaultsOnly, Category = "NeonStack|UI") TSubclassOf<UNSCountdownWidget> CountdownClass;
	UPROPERTY(EditDefaultsOnly, Category = "NeonStack|UI") TSubclassOf<UNSPauseWidget> PauseClass;
	UPROPERTY(EditDefaultsOnly, Category = "NeonStack|UI") TSubclassOf<UNSResultsWidget> ResultsClass;
	UPROPERTY(EditDefaultsOnly, Category = "NeonStack|UI") TSubclassOf<UNSOptionsWidget> OptionsClass;

	/** Cria os widgets e passa a seguir o fluxo do GameMode. Chamado pelo ANSGameMode. */
	void InitUI(ANSGameMode* InGameMode, ANSBoardActor* InBoard);

	/** Abre a tela de opções (a partir da pausa). */
	UFUNCTION(BlueprintCallable, Category = "NeonStack|UI")
	void OpenOptions();

	UPROPERTY(EditDefaultsOnly, Category = "NeonStack|Input") TObjectPtr<UInputAction> IA_MoveLeft;
	UPROPERTY(EditDefaultsOnly, Category = "NeonStack|Input") TObjectPtr<UInputAction> IA_MoveRight;
	UPROPERTY(EditDefaultsOnly, Category = "NeonStack|Input") TObjectPtr<UInputAction> IA_SoftDrop;
	UPROPERTY(EditDefaultsOnly, Category = "NeonStack|Input") TObjectPtr<UInputAction> IA_HardDrop;
	UPROPERTY(EditDefaultsOnly, Category = "NeonStack|Input") TObjectPtr<UInputAction> IA_RotateCW;
	UPROPERTY(EditDefaultsOnly, Category = "NeonStack|Input") TObjectPtr<UInputAction> IA_RotateCCW;
	UPROPERTY(EditDefaultsOnly, Category = "NeonStack|Input") TObjectPtr<UInputAction> IA_Hold;
	UPROPERTY(EditDefaultsOnly, Category = "NeonStack|Input") TObjectPtr<UInputAction> IA_Pause;

	UFUNCTION(BlueprintCallable, Category = "NeonStack")
	void SetBoard(ANSBoardActor* InBoard);

	/** true = contexto de gameplay; false = contexto de UI. */
	UFUNCTION(BlueprintCallable, Category = "NeonStack")
	void SetGameplayInputEnabled(bool bEnabled);

protected:
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;

private:
	void BindPressRelease(class UEnhancedInputComponent* EIC, const UInputAction* IA, ENSInputAction Action);
	void HandlePressed(ENSInputAction Action);
	void HandleReleased(ENSInputAction Action);
	void HandlePause();

	/** Preenche campos vazios com os assets padrão (NSDefaultAssets.h) e as classes de widget C++. */
	void EnsureDefaults();
	bool bDefaultsEnsured = false;

	UFUNCTION()
	void HandleFlowStateChanged(ENSFlowState NewState);

	UFUNCTION()
	void HandleOptionsClosed();

	TWeakObjectPtr<ANSBoardActor> Board;
	TWeakObjectPtr<ANSGameMode> GameMode;

	UPROPERTY(Transient) TObjectPtr<UNSHUDWidget> HUDWidget;
	UPROPERTY(Transient) TObjectPtr<UNSCountdownWidget> CountdownWidget;
	UPROPERTY(Transient) TObjectPtr<UNSPauseWidget> PauseWidget;
	UPROPERTY(Transient) TObjectPtr<UNSResultsWidget> ResultsWidget;
	UPROPERTY(Transient) TObjectPtr<UNSOptionsWidget> OptionsWidget;
};
