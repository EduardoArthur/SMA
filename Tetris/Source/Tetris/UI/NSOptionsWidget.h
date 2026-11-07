// Neon Stack - tela de opções (GDD §7.4). Salva via UNSSaveSubsystem ao fechar.
// WBP_Options (pai: NSOptionsWidget):
//   Slider: MusicSlider, SfxSlider, DasSlider, ArrSlider
//   ComboBoxString: FxCombo     CheckBox: ShakeCheck     Button: BackButton
//   (opcionais) TextBlock: MusicValueText, SfxValueText, DasValueText, ArrValueText

#pragma once

#include "CoreMinimal.h"
#include "UI/NSMenuWidgetBase.h"
#include "NSOptionsWidget.generated.h"

class UButton;
class UCheckBox;
class UComboBoxString;
class USlider;
class UTextBlock;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FNSOnOptionsClosed);

UCLASS()
class TETRIS_API UNSOptionsWidget : public UNSMenuWidgetBase
{
	GENERATED_BODY()

public:
	/** Sem WBP: monta um layout padrão em C++ antes da inicialização. */
	virtual bool Initialize() override;

private:
	void BuildDefaultLayout();


public:
	/** Carrega as opções salvas nos controles, mostra e foca. */
	UFUNCTION(BlueprintCallable, Category = "NeonStack|UI")
	void Open();

	/** Aplica, salva, esconde e dispara OnClosed. */
	UFUNCTION(BlueprintCallable, Category = "NeonStack|UI")
	void ApplyAndClose();

	UPROPERTY(BlueprintAssignable, Category = "NeonStack|UI")
	FNSOnOptionsClosed OnClosed;

protected:
	virtual void NativeOnInitialized() override;
	virtual UWidget* GetDefaultFocusWidget() const override;

	UPROPERTY(meta = (BindWidget)) TObjectPtr<USlider> MusicSlider;
	UPROPERTY(meta = (BindWidget)) TObjectPtr<USlider> SfxSlider;
	UPROPERTY(meta = (BindWidget)) TObjectPtr<USlider> DasSlider;
	UPROPERTY(meta = (BindWidget)) TObjectPtr<USlider> ArrSlider;
	UPROPERTY(meta = (BindWidget)) TObjectPtr<UComboBoxString> FxCombo;
	UPROPERTY(meta = (BindWidget)) TObjectPtr<UCheckBox> ShakeCheck;
	UPROPERTY(meta = (BindWidget)) TObjectPtr<UButton> BackButton;

	UPROPERTY(meta = (BindWidgetOptional)) TObjectPtr<UTextBlock> MusicValueText;
	UPROPERTY(meta = (BindWidgetOptional)) TObjectPtr<UTextBlock> SfxValueText;
	UPROPERTY(meta = (BindWidgetOptional)) TObjectPtr<UTextBlock> DasValueText;
	UPROPERTY(meta = (BindWidgetOptional)) TObjectPtr<UTextBlock> ArrValueText;

private:
	UFUNCTION() void HandleBack();
	UFUNCTION() void HandleSliderChanged(float Value);

	void RefreshValueTexts();
};
