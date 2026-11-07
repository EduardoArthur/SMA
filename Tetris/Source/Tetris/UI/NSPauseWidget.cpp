#include "UI/NSPauseWidget.h"

#include "Components/Button.h"
#include "Engine/World.h"
#include "Game/NSGameMode.h"
#include "Game/NSPlayerController.h"

void UNSPauseWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	ContinueButton->OnClicked.AddDynamic(this, &UNSPauseWidget::HandleContinue);
	RestartButton->OnClicked.AddDynamic(this, &UNSPauseWidget::HandleRestart);
	MenuButton->OnClicked.AddDynamic(this, &UNSPauseWidget::HandleMenu);
	if (OptionsButton)
	{
		OptionsButton->OnClicked.AddDynamic(this, &UNSPauseWidget::HandleOptions);
	}
}

UWidget* UNSPauseWidget::GetDefaultFocusWidget() const
{
	return ContinueButton;
}

void UNSPauseWidget::HandleContinue()
{
	if (ANSGameMode* GM = GetWorld()->GetAuthGameMode<ANSGameMode>())
	{
		GM->ResumeGame();
	}
}

void UNSPauseWidget::HandleRestart()
{
	if (ANSGameMode* GM = GetWorld()->GetAuthGameMode<ANSGameMode>())
	{
		GM->RestartGame();
	}
}

void UNSPauseWidget::HandleOptions()
{
	if (ANSPlayerController* PC = Cast<ANSPlayerController>(GetOwningPlayer()))
	{
		PC->OpenOptions();
	}
}

void UNSPauseWidget::HandleMenu()
{
	if (ANSGameMode* GM = GetWorld()->GetAuthGameMode<ANSGameMode>())
	{
		GM->ReturnToMenu();
	}
}
