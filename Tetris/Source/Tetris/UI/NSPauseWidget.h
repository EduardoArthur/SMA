// Neon Stack - menu de pausa (cobre o poço).
// WBP_Pause (pai: NSPauseWidget): Buttons ContinueButton, RestartButton, OptionsButton, MenuButton.

#pragma once

#include "CoreMinimal.h"
#include "UI/NSMenuWidgetBase.h"
#include "NSPauseWidget.generated.h"

class UButton;

UCLASS()
class TETRIS_API UNSPauseWidget : public UNSMenuWidgetBase
{
	GENERATED_BODY()

public:
	/** Sem WBP: monta um layout padrão em C++ antes da inicialização. */
	virtual bool Initialize() override;

private:
	void BuildDefaultLayout();


protected:
	virtual void NativeOnInitialized() override;
	virtual UWidget* GetDefaultFocusWidget() const override;

	UPROPERTY(meta = (BindWidget)) TObjectPtr<UButton> ContinueButton;
	UPROPERTY(meta = (BindWidget)) TObjectPtr<UButton> RestartButton;
	UPROPERTY(meta = (BindWidgetOptional)) TObjectPtr<UButton> OptionsButton;
	UPROPERTY(meta = (BindWidget)) TObjectPtr<UButton> MenuButton;

private:
	UFUNCTION() void HandleContinue();
	UFUNCTION() void HandleRestart();
	UFUNCTION() void HandleOptions();
	UFUNCTION() void HandleMenu();
};
