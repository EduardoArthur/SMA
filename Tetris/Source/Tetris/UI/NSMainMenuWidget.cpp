#include "UI/NSMainMenuWidget.h"

#include "Tetris.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Systems/NSSaveSubsystem.h"
#include "UI/NSOptionsWidget.h"
#include "NSDefaultAssets.h"

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
	const TSoftObjectPtr<UWorld> Level = GameLevel.IsNull()
		? TSoftObjectPtr<UWorld>(FSoftObjectPath(NSAssets::GameMap))
		: GameLevel;
	UGameplayStatics::OpenLevelBySoftObjectPtr(this, Level);
}

void UNSMainMenuWidget::HandleOptions()
{
	if (!OptionsClass)
	{
		OptionsClass = UNSOptionsWidget::StaticClass(); // layout padrão em C++
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
