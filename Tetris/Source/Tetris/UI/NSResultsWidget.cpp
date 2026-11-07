#include "UI/NSResultsWidget.h"

#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Engine/World.h"
#include "Game/NSGameMode.h"
#include "Audio/NSAudioDirector.h"

void UNSResultsWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	PlayAgainButton->OnClicked.AddDynamic(this, &UNSResultsWidget::HandlePlayAgain);
	MenuButton->OnClicked.AddDynamic(this, &UNSResultsWidget::HandleMenu);
}

UWidget* UNSResultsWidget::GetDefaultFocusWidget() const
{
	return PlayAgainButton; // "Jogar de novo" é o padrão (Enter)
}

void UNSResultsWidget::SetResult(int64 Score, int32 Lines, int32 Level, bool bNewHighScore)
{
	ScoreText->SetText(FText::AsNumber(Score));
	LinesText->SetText(FText::AsNumber(Lines));
	LevelText->SetText(FText::AsNumber(Level));
	if (NewRecordText)
	{
		NewRecordText->SetVisibility(bNewHighScore ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
	}
	if (bNewHighScore)
	{
		if (ANSAudioDirector* Audio = ANSAudioDirector::Get(this))
		{
			Audio->PlayNewRecord();
		}
		OnNewHighScoreShown();
	}
}

void UNSResultsWidget::HandlePlayAgain()
{
	if (ANSGameMode* GM = GetWorld()->GetAuthGameMode<ANSGameMode>())
	{
		GM->RestartGame();
	}
}

void UNSResultsWidget::HandleMenu()
{
	if (ANSGameMode* GM = GetWorld()->GetAuthGameMode<ANSGameMode>())
	{
		GM->ReturnToMenu();
	}
}
