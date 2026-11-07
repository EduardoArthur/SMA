#include "UI/NSHUDWidget.h"

#include "Components/TextBlock.h"
#include "Game/NSBoardActor.h"
#include "Systems/NSSaveSubsystem.h"
#include "UI/NSPieceIconWidget.h"

void UNSHUDWidget::InitForBoard(ANSBoardActor* InBoard)
{
	if (Board.IsValid() || !InBoard)
	{
		return;
	}
	Board = InBoard;

	InBoard->OnScoreChanged.AddDynamic(this, &UNSHUDWidget::HandleScoreChanged);
	InBoard->OnHoldChanged.AddDynamic(this, &UNSHUDWidget::HandleHoldChanged);
	InBoard->OnQueueChanged.AddDynamic(this, &UNSHUDWidget::HandleQueueChanged);
	InBoard->OnLinesCleared.AddDynamic(this, &UNSHUDWidget::HandleLinesCleared);
	InBoard->OnLevelUp.AddDynamic(this, &UNSHUDWidget::HandleLevelUp);

	HandleScoreChanged(0, 0, 1);
	HandleHoldChanged(ENSPieceType::None, true);
	HandleQueueChanged(TArray<ENSPieceType>());
	if (ComboText)
	{
		ComboText->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void UNSHUDWidget::HandleScoreChanged(int64 Score, int32 Lines, int32 Level)
{
	ScoreText->SetText(FText::AsNumber(Score));
	LinesText->SetText(FText::AsNumber(Lines));
	LevelText->SetText(FText::AsNumber(Level));
	RefreshHighScore();
}

void UNSHUDWidget::RefreshHighScore()
{
	if (!HighScoreText)
	{
		return;
	}
	const UNSSaveSubsystem* Save = UNSSaveSubsystem::Get(this);
	const int64 Current = Board.IsValid() ? Board->GetScore() : 0;
	const int64 Best = FMath::Max(Save ? Save->GetHighScore() : 0, Current); // mostra o recorde subindo ao vivo
	HighScoreText->SetText(FText::AsNumber(Best));
}

void UNSHUDWidget::HandleHoldChanged(ENSPieceType Held, bool bCanHold)
{
	HoldIcon->SetPiece(Held, !bCanHold);
}

void UNSHUDWidget::HandleQueueChanged(const TArray<ENSPieceType>& Next)
{
	UNSPieceIconWidget* Icons[3] = { Next0, Next1, Next2 };
	for (int32 i = 0; i < 3; ++i)
	{
		if (Icons[i])
		{
			Icons[i]->SetPiece(Next.IsValidIndex(i) ? Next[i] : ENSPieceType::None);
		}
	}
}

void UNSHUDWidget::HandleLinesCleared(const TArray<int32>& Rows, int32 Count, int32 Combo, int32 ScoreGained)
{
	OnLinesClearedShown(Count, ScoreGained);

	if (Combo >= 1)
	{
		if (ComboText)
		{
			ComboText->SetText(FText::Format(NSLOCTEXT("NeonStack", "Combo", "COMBO x{0}"), FText::AsNumber(Combo)));
			ComboText->SetVisibility(ESlateVisibility::HitTestInvisible);
		}
		ComboTimeLeft = ComboDisplayTime;
		OnComboShown(Combo);
	}
}

void UNSHUDWidget::HandleLevelUp(int32 NewLevel)
{
	OnLevelUpShown(NewLevel);
}

void UNSHUDWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (ComboTimeLeft > 0.f)
	{
		ComboTimeLeft -= InDeltaTime;
		if (ComboTimeLeft <= 0.f && ComboText)
		{
			ComboText->SetVisibility(ESlateVisibility::Collapsed);
		}
	}
}
