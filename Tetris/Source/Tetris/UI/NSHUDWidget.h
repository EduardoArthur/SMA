// Neon Stack - HUD da partida (GDD §7.5).
// WBP_HUD (pai: NSHUDWidget) precisa ter widgets com estes NOMES exatos:
//   TextBlock: ScoreText, LevelText, LinesText   (opcionais: HighScoreText, ComboText)
//   WBP_PieceIcon: HoldIcon, Next0               (opcionais: Next1, Next2)

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Simulation/NSTypes.h"
#include "NSHUDWidget.generated.h"

class UTextBlock;
class UNSPieceIconWidget;
class ANSBoardActor;

UCLASS()
class TETRIS_API UNSHUDWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/** Sem WBP: monta um layout padrão em C++ antes da inicialização. */
	virtual bool Initialize() override;

private:
	void BuildDefaultLayout();


public:
	/** Liga o HUD aos eventos do board. Chamado pelo PlayerController. */
	void InitForBoard(ANSBoardActor* InBoard);

	/** Tempo que o "COMBO ×N" fica na tela. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NeonStack|UI", meta = (Units = "s"))
	float ComboDisplayTime = 1.5f;

protected:
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	/** Ganchos para animações no Blueprint (M4). */
	UFUNCTION(BlueprintImplementableEvent, Category = "NeonStack|UI")
	void OnComboShown(int32 Combo);

	UFUNCTION(BlueprintImplementableEvent, Category = "NeonStack|UI")
	void OnLevelUpShown(int32 NewLevel);

	UFUNCTION(BlueprintImplementableEvent, Category = "NeonStack|UI")
	void OnLinesClearedShown(int32 Count, int32 ScoreGained);

	UPROPERTY(meta = (BindWidget)) TObjectPtr<UTextBlock> ScoreText;
	UPROPERTY(meta = (BindWidget)) TObjectPtr<UTextBlock> LevelText;
	UPROPERTY(meta = (BindWidget)) TObjectPtr<UTextBlock> LinesText;
	UPROPERTY(meta = (BindWidgetOptional)) TObjectPtr<UTextBlock> HighScoreText;
	UPROPERTY(meta = (BindWidgetOptional)) TObjectPtr<UTextBlock> ComboText;

	UPROPERTY(meta = (BindWidget)) TObjectPtr<UNSPieceIconWidget> HoldIcon;
	UPROPERTY(meta = (BindWidget)) TObjectPtr<UNSPieceIconWidget> Next0;
	UPROPERTY(meta = (BindWidgetOptional)) TObjectPtr<UNSPieceIconWidget> Next1;
	UPROPERTY(meta = (BindWidgetOptional)) TObjectPtr<UNSPieceIconWidget> Next2;

private:
	UFUNCTION() void HandleScoreChanged(int64 Score, int32 Lines, int32 Level);
	UFUNCTION() void HandleHoldChanged(ENSPieceType Held, bool bCanHold);
	UFUNCTION() void HandleQueueChanged(const TArray<ENSPieceType>& Next);
	UFUNCTION() void HandleLinesCleared(const TArray<int32>& Rows, int32 Count, int32 Combo, int32 ScoreGained);
	UFUNCTION() void HandleLevelUp(int32 NewLevel);

	void RefreshHighScore();

	TWeakObjectPtr<ANSBoardActor> Board;
	float ComboTimeLeft = 0.f;
};
