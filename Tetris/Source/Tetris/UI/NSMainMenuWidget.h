// Neon Stack - menu principal (GDD §7.2).
// WBP_MainMenu (pai: NSMainMenuWidget):
//   Button: PlayButton, OptionsButton, QuitButton     (opcional) TextBlock: HighScoreText

#pragma once

#include "CoreMinimal.h"
#include "UI/NSMenuWidgetBase.h"
#include "NSMainMenuWidget.generated.h"

class UButton;
class UTextBlock;
class UNSOptionsWidget;

UCLASS()
class TETRIS_API UNSMainMenuWidget : public UNSMenuWidgetBase
{
	GENERATED_BODY()

public:
	/** Sem WBP: monta um layout padrão em C++ antes da inicialização. */
	virtual bool Initialize() override;

private:
	void BuildDefaultLayout();


public:
	/** Mapa do jogo (L_Game). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NeonStack|UI")
	TSoftObjectPtr<UWorld> GameLevel;

	/** WBP_Options. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NeonStack|UI")
	TSubclassOf<UNSOptionsWidget> OptionsClass;

protected:
	virtual void NativeOnInitialized() override;
	virtual void NativeConstruct() override;
	virtual UWidget* GetDefaultFocusWidget() const override;

	UPROPERTY(meta = (BindWidget)) TObjectPtr<UButton> PlayButton;
	UPROPERTY(meta = (BindWidgetOptional)) TObjectPtr<UButton> OptionsButton;
	UPROPERTY(meta = (BindWidget)) TObjectPtr<UButton> QuitButton;
	UPROPERTY(meta = (BindWidgetOptional)) TObjectPtr<UTextBlock> HighScoreText;

private:
	UFUNCTION() void HandlePlay();
	UFUNCTION() void HandleOptions();
	UFUNCTION() void HandleQuit();
	UFUNCTION() void HandleOptionsClosed();

	UPROPERTY(Transient)
	TObjectPtr<UNSOptionsWidget> Options;
};
