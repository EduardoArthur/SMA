// Neon Stack - tela de resultados (GDD §7.6).
// WBP_Results (pai: NSResultsWidget):
//   TextBlock: ScoreText, LinesText, LevelText   (opcional: NewRecordText, mostrado só em recorde)
//   Button: PlayAgainButton, MenuButton

#pragma once

#include "CoreMinimal.h"
#include "UI/NSMenuWidgetBase.h"
#include "NSResultsWidget.generated.h"

class UButton;
class UTextBlock;

UCLASS()
class TETRIS_API UNSResultsWidget : public UNSMenuWidgetBase
{
	GENERATED_BODY()

public:
	/** Sem WBP: monta um layout padrão em C++ antes da inicialização. */
	virtual bool Initialize() override;

private:
	void BuildDefaultLayout();


public:
	UFUNCTION(BlueprintCallable, Category = "NeonStack|UI")
	void SetResult(int64 Score, int32 Lines, int32 Level, bool bNewHighScore);

protected:
	virtual void NativeOnInitialized() override;
	virtual UWidget* GetDefaultFocusWidget() const override;

	/** Gancho para animação/SFX do recorde (M4). */
	UFUNCTION(BlueprintImplementableEvent, Category = "NeonStack|UI")
	void OnNewHighScoreShown();

	UPROPERTY(meta = (BindWidget)) TObjectPtr<UTextBlock> ScoreText;
	UPROPERTY(meta = (BindWidget)) TObjectPtr<UTextBlock> LinesText;
	UPROPERTY(meta = (BindWidget)) TObjectPtr<UTextBlock> LevelText;
	UPROPERTY(meta = (BindWidgetOptional)) TObjectPtr<UTextBlock> NewRecordText;
	UPROPERTY(meta = (BindWidget)) TObjectPtr<UButton> PlayAgainButton;
	UPROPERTY(meta = (BindWidget)) TObjectPtr<UButton> MenuButton;

private:
	UFUNCTION() void HandlePlayAgain();
	UFUNCTION() void HandleMenu();
};
