#include "UI/NSMainMenuWidget.h"

#include "Tetris.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Systems/NSSaveSubsystem.h"
#include "UI/NSOptionsWidget.h"

void UNSMainMenuWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	PlayButton->OnClicked.AddDynamic(this, &UNSMainMenuWidget::HandlePlay);
	QuitButton->OnClicked.AddDynamic(this, &UNSMainMenuWidget::HandleQuit);
	if (OptionsButton)
	{
		OptionsButton->OnClicked.AddDynamic(this, &UNSMainMenuWidget::HandleOptions);
	}
}

void UNSMainMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();
	if (HighScoreText)
	{
		const UNSSaveSubsystem* Save = UNSSaveSubsystem::Get(this);
		HighScoreText->SetText(FText::AsNumber(Save ? Save->GetHighScore() : 0));
	}
}

UWidget* UNSMainMenuWidget::GetDefaultFocusWidget() const
{
	return PlayButton;
}

void UNSMainMenuWidget::HandlePlay()
{
	if (GameLevel.IsNull())
	{
		UE_LOG(LogNeonStack, Warning, TEXT("NSMainMenuWidget: GameLevel não configurado no WBP_MainMenu."));
		return;
	}
	UGameplayStatics::OpenLevelBySoftObjectPtr(this, GameLevel);
}

void UNSMainMenuWidget::HandleOptions()
{
	if (!OptionsClass)
	{
		UE_LOG(LogNeonStack, Warning, TEXT("NSMainMenuWidget: OptionsClass não configurado."));
		return;
	}
	if (!Options)
	{
		Options = CreateWidget<UNSOptionsWidget>(GetOwningPlayer(), OptionsClass);
		Options->AddToViewport(10);
		Options->OnClosed.AddDynamic(this, &UNSMainMenuWidget::HandleOptionsClosed);
	}
	Options->Open();
}

void UNSMainMenuWidget::HandleOptionsClosed()
{
	FocusDefault();
}

void UNSMainMenuWidget::HandleQuit()
{
	UKismetSystemLibrary::QuitGame(this, GetOwningPlayer(), EQuitPreference::Quit, false);
}
