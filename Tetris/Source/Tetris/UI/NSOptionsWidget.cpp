#include "UI/NSOptionsWidget.h"

#include "Components/Button.h"
#include "Components/CheckBox.h"
#include "Components/ComboBoxString.h"
#include "Components/Slider.h"
#include "Components/TextBlock.h"
#include "Systems/NSSaveSubsystem.h"

namespace NSOptions
{
	static void SetupSlider(USlider* Slider, float Min, float Max, float Step)
	{
		Slider->SetMinValue(Min);
		Slider->SetMaxValue(Max);
		Slider->SetStepSize(Step); // passo das setas do teclado
	}

	static void SetText(UTextBlock* Text, int32 Value, const TCHAR* Suffix)
	{
		if (Text)
		{
			Text->SetText(FText::FromString(FString::Printf(TEXT("%d%s"), Value, Suffix)));
		}
	}
}

void UNSOptionsWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	NSOptions::SetupSlider(MusicSlider, 0.f, 100.f, 5.f);
	NSOptions::SetupSlider(SfxSlider, 0.f, 100.f, 5.f);
	NSOptions::SetupSlider(DasSlider, 50.f, 300.f, 10.f);
	NSOptions::SetupSlider(ArrSlider, 0.f, 100.f, 5.f);

	FxCombo->ClearOptions();
	FxCombo->AddOption(TEXT("Baixa"));
	FxCombo->AddOption(TEXT("Media"));
	FxCombo->AddOption(TEXT("Alta"));

	MusicSlider->OnValueChanged.AddDynamic(this, &UNSOptionsWidget::HandleSliderChanged);
	SfxSlider->OnValueChanged.AddDynamic(this, &UNSOptionsWidget::HandleSliderChanged);
	DasSlider->OnValueChanged.AddDynamic(this, &UNSOptionsWidget::HandleSliderChanged);
	ArrSlider->OnValueChanged.AddDynamic(this, &UNSOptionsWidget::HandleSliderChanged);
	BackButton->OnClicked.AddDynamic(this, &UNSOptionsWidget::HandleBack);
}

UWidget* UNSOptionsWidget::GetDefaultFocusWidget() const
{
	return MusicSlider;
}

void UNSOptionsWidget::Open()
{
	const UNSSaveSubsystem* Save = UNSSaveSubsystem::Get(this);
	const FNSSettings S = Save ? Save->GetSettings() : FNSSettings();

	MusicSlider->SetValue(static_cast<float>(S.MusicVolume));
	SfxSlider->SetValue(static_cast<float>(S.SfxVolume));
	DasSlider->SetValue(static_cast<float>(S.DasMs));
	ArrSlider->SetValue(static_cast<float>(S.ArrMs));
	FxCombo->SetSelectedIndex(static_cast<int32>(S.FxIntensity));
	ShakeCheck->SetIsChecked(S.bScreenShake);
	RefreshValueTexts();

	SetVisibility(ESlateVisibility::Visible);
	FocusDefault();
}

void UNSOptionsWidget::ApplyAndClose()
{
	FNSSettings S;
	S.MusicVolume = FMath::RoundToInt(MusicSlider->GetValue());
	S.SfxVolume = FMath::RoundToInt(SfxSlider->GetValue());
	S.DasMs = FMath::RoundToInt(DasSlider->GetValue());
	S.ArrMs = FMath::RoundToInt(ArrSlider->GetValue());
	S.FxIntensity = static_cast<ENSFxIntensity>(FMath::Clamp(FxCombo->GetSelectedIndex(), 0, 2));
	S.bScreenShake = ShakeCheck->IsChecked();

	if (UNSSaveSubsystem* Save = UNSSaveSubsystem::Get(this))
	{
		Save->SetSettings(S);
	}

	SetVisibility(ESlateVisibility::Collapsed);
	OnClosed.Broadcast();
}

void UNSOptionsWidget::HandleBack()
{
	ApplyAndClose();
}

void UNSOptionsWidget::HandleSliderChanged(float Value)
{
	RefreshValueTexts();
}

void UNSOptionsWidget::RefreshValueTexts()
{
	NSOptions::SetText(MusicValueText, FMath::RoundToInt(MusicSlider->GetValue()), TEXT(""));
	NSOptions::SetText(SfxValueText, FMath::RoundToInt(SfxSlider->GetValue()), TEXT(""));
	NSOptions::SetText(DasValueText, FMath::RoundToInt(DasSlider->GetValue()), TEXT(" ms"));
	NSOptions::SetText(ArrValueText, FMath::RoundToInt(ArrSlider->GetValue()), TEXT(" ms"));
}
