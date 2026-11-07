// Neon Stack - layouts padrão em C++ dos widgets (usados quando não há WBP).
// Se o widget for um Widget Blueprint, PrepareNativeTree() retorna false e o layout do designer é usado.

#include "UI/NSUIBuilder.h"

#include "Components/CheckBox.h"
#include "Components/ComboBoxString.h"
#include "Components/Slider.h"
#include "UI/NSCountdownWidget.h"
#include "UI/NSHUDWidget.h"
#include "UI/NSMainMenuWidget.h"
#include "UI/NSOptionsWidget.h"
#include "UI/NSPauseWidget.h"
#include "UI/NSPieceIconWidget.h"
#include "UI/NSResultsWidget.h"

#define NS_DEFAULT_INITIALIZE(ClassName)            \
	bool ClassName::Initialize()                    \
	{                                               \
		if (NSUI::PrepareNativeTree(this))          \
		{                                           \
			BuildDefaultLayout();                   \
		}                                           \
		return Super::Initialize();                 \
	}

NS_DEFAULT_INITIALIZE(UNSHUDWidget)
NS_DEFAULT_INITIALIZE(UNSCountdownWidget)
NS_DEFAULT_INITIALIZE(UNSPauseWidget)
NS_DEFAULT_INITIALIZE(UNSResultsWidget)
NS_DEFAULT_INITIALIZE(UNSOptionsWidget)
NS_DEFAULT_INITIALIZE(UNSMainMenuWidget)

#undef NS_DEFAULT_INITIALIZE

namespace
{
	UButton* AddMenuButton(UWidgetTree* T, UVerticalBox* Box, const TCHAR* Label, float Bottom = 12.f, float Height = 56.f)
	{
		UButton* B = NSUI::Button(T, Label);
		NSUI::AddV(Box, NSUI::Sized(T, B, 380.f, Height), Bottom, HAlign_Center);
		return B;
	}

	UImage* Overlay(UWidgetTree* T, UCanvasPanel* Canvas, float Alpha)
	{
		UImage* I = NSUI::Rect(T, FLinearColor(0.01f, 0.f, 0.02f, Alpha));
		NSUI::Fill(Canvas, I);
		return I;
	}
}

// ---------------------------------------------------------------------------
// HUD - GDD §7.5: hold e placar à esquerda, próximas à direita, poço no centro
// ---------------------------------------------------------------------------
void UNSHUDWidget::BuildDefaultLayout()
{
	UWidgetTree* T = WidgetTree;
	UCanvasPanel* Canvas = NSUI::RootCanvas(T);

	auto Icon = [T](float W, float H, UNSPieceIconWidget*& Out) -> UWidget*
	{
		Out = T->ConstructWidget<UNSPieceIconWidget>(UNSPieceIconWidget::StaticClass());
		return NSUI::Sized(T, Out, W, H);
	};
	auto Label = [T](const TCHAR* Str) { return NSUI::Text(T, Str, 16, NSUI::Accent()); };

	UNSPieceIconWidget* Hold = nullptr;
	UNSPieceIconWidget* N0 = nullptr;
	UNSPieceIconWidget* N1 = nullptr;
	UNSPieceIconWidget* N2 = nullptr;

	// Esquerda
	UVerticalBox* Left = NSUI::VBox(T);
	NSUI::AddV(Left, Label(TEXT("HOLD")), 4.f);
	NSUI::AddV(Left, Icon(140.f, 70.f, Hold), 40.f, HAlign_Left);
	ScoreText = NSUI::Text(T, TEXT("0"), 30);
	LevelText = NSUI::Text(T, TEXT("1"), 30);
	LinesText = NSUI::Text(T, TEXT("0"), 30);
	HighScoreText = NSUI::Text(T, TEXT("0"), 22, NSUI::Amber());
	NSUI::AddV(Left, Label(TEXT("PONTOS")), 0.f);
	NSUI::AddV(Left, ScoreText, 16.f);
	NSUI::AddV(Left, Label(TEXT("NÍVEL")), 0.f);
	NSUI::AddV(Left, LevelText, 16.f);
	NSUI::AddV(Left, Label(TEXT("LINHAS")), 0.f);
	NSUI::AddV(Left, LinesText, 16.f);
	NSUI::AddV(Left, Label(TEXT("RECORDE")), 0.f);
	NSUI::AddV(Left, HighScoreText, 0.f);
	NSUI::Place(Canvas, Left, FVector2D(0.5f, 0.5f), FVector2D(1.f, 0.5f), FVector2D(-270.f, 0.f));

	// Direita
	UVerticalBox* Right = NSUI::VBox(T);
	NSUI::AddV(Right, Label(TEXT("NEXT")), 4.f);
	NSUI::AddV(Right, Icon(140.f, 70.f, N0), 16.f, HAlign_Left);
	NSUI::AddV(Right, Icon(100.f, 50.f, N1), 12.f, HAlign_Left);
	NSUI::AddV(Right, Icon(100.f, 50.f, N2), 0.f, HAlign_Left);
	NSUI::Place(Canvas, Right, FVector2D(0.5f, 0.5f), FVector2D(0.f, 0.5f), FVector2D(270.f, -120.f));

	HoldIcon = Hold;
	Next0 = N0;
	Next1 = N1;
	Next2 = N2;

	ComboText = NSUI::Text(T, TEXT("COMBO x2"), 34, NSUI::Cyan());
	NSUI::Place(Canvas, ComboText, FVector2D(0.5f, 0.5f), FVector2D(0.f, 0.5f), FVector2D(270.f, 180.f));
}

// ---------------------------------------------------------------------------
// Contagem
// ---------------------------------------------------------------------------
void UNSCountdownWidget::BuildDefaultLayout()
{
	UCanvasPanel* Canvas = NSUI::RootCanvas(WidgetTree);
	CountText = NSUI::Text(WidgetTree, TEXT("3"), 160, NSUI::Cyan(), true);
	NSUI::Place(Canvas, CountText, FVector2D(0.5f, 0.5f), FVector2D(0.5f, 0.5f));
}

// ---------------------------------------------------------------------------
// Pausa - cobre o poço (GDD §7.3)
// ---------------------------------------------------------------------------
void UNSPauseWidget::BuildDefaultLayout()
{
	UWidgetTree* T = WidgetTree;
	UCanvasPanel* Canvas = NSUI::RootCanvas(T);
	Overlay(T, Canvas, 0.92f);

	UVerticalBox* Box = NSUI::VBox(T);
	NSUI::AddV(Box, NSUI::Text(T, TEXT("PAUSA"), 64, NSUI::Accent(), true), 32.f, HAlign_Center);
	ContinueButton = AddMenuButton(T, Box, TEXT("CONTINUAR"));
	RestartButton = AddMenuButton(T, Box, TEXT("REINICIAR"));
	OptionsButton = AddMenuButton(T, Box, TEXT("OPÇÕES"));
	MenuButton = AddMenuButton(T, Box, TEXT("MENU"), 0.f);
	NSUI::Place(Canvas, Box, FVector2D(0.5f, 0.5f), FVector2D(0.5f, 0.5f));
}

// ---------------------------------------------------------------------------
// Resultados (GDD §7.6)
// ---------------------------------------------------------------------------
void UNSResultsWidget::BuildDefaultLayout()
{
	UWidgetTree* T = WidgetTree;
	UCanvasPanel* Canvas = NSUI::RootCanvas(T);
	Overlay(T, Canvas, 0.88f);

	UVerticalBox* Box = NSUI::VBox(T);
	NSUI::AddV(Box, NSUI::Text(T, TEXT("FIM DE JOGO"), 56, NSUI::Accent(), true), 12.f, HAlign_Center);
	NewRecordText = NSUI::Text(T, TEXT("NOVO RECORDE!"), 34, NSUI::Amber(), true);
	NSUI::AddV(Box, NewRecordText, 24.f, HAlign_Center);

	ScoreText = NSUI::Text(T, TEXT("0"), 48, FLinearColor::White, true);
	LinesText = NSUI::Text(T, TEXT("0"), 28, FLinearColor::White, true);
	LevelText = NSUI::Text(T, TEXT("1"), 28, FLinearColor::White, true);
	NSUI::AddV(Box, NSUI::Text(T, TEXT("PONTOS"), 16, NSUI::Cyan(), true), 0.f, HAlign_Center);
	NSUI::AddV(Box, ScoreText, 16.f, HAlign_Center);
	NSUI::AddV(Box, NSUI::Text(T, TEXT("LINHAS"), 16, NSUI::Cyan(), true), 0.f, HAlign_Center);
	NSUI::AddV(Box, LinesText, 12.f, HAlign_Center);
	NSUI::AddV(Box, NSUI::Text(T, TEXT("NÍVEL"), 16, NSUI::Cyan(), true), 0.f, HAlign_Center);
	NSUI::AddV(Box, LevelText, 32.f, HAlign_Center);

	PlayAgainButton = AddMenuButton(T, Box, TEXT("JOGAR DE NOVO"));
	MenuButton = AddMenuButton(T, Box, TEXT("MENU"), 0.f);
	NSUI::Place(Canvas, Box, FVector2D(0.5f, 0.5f), FVector2D(0.5f, 0.5f));
}

// ---------------------------------------------------------------------------
// Opções (GDD §7.4)
// ---------------------------------------------------------------------------
void UNSOptionsWidget::BuildDefaultLayout()
{
	UWidgetTree* T = WidgetTree;
	UCanvasPanel* Canvas = NSUI::RootCanvas(T);
	Overlay(T, Canvas, 0.96f);

	auto Label = [T](const TCHAR* Str) { return NSUI::Text(T, Str, 22, NSUI::Cyan()); };
	auto Value = [T]() { return NSUI::Text(T, TEXT("0"), 22); };

	MusicSlider = T->ConstructWidget<USlider>(USlider::StaticClass());
	SfxSlider = T->ConstructWidget<USlider>(USlider::StaticClass());
	DasSlider = T->ConstructWidget<USlider>(USlider::StaticClass());
	ArrSlider = T->ConstructWidget<USlider>(USlider::StaticClass());
	FxCombo = T->ConstructWidget<UComboBoxString>(UComboBoxString::StaticClass());
	ShakeCheck = T->ConstructWidget<UCheckBox>(UCheckBox::StaticClass());
	MusicValueText = Value();
	SfxValueText = Value();
	DasValueText = Value();
	ArrValueText = Value();

	UVerticalBox* Box = NSUI::VBox(T);
	NSUI::AddV(Box, NSUI::Text(T, TEXT("OPÇÕES"), 56, NSUI::Accent(), true), 32.f, HAlign_Center);
	NSUI::AddV(Box, NSUI::Row(T, Label(TEXT("MÚSICA")), MusicSlider, MusicValueText), 12.f);
	NSUI::AddV(Box, NSUI::Row(T, Label(TEXT("EFEITOS")), SfxSlider, SfxValueText), 12.f);
	NSUI::AddV(Box, NSUI::Row(T, Label(TEXT("INTENSIDADE FX")), FxCombo), 12.f);
	NSUI::AddV(Box, NSUI::Row(T, Label(TEXT("TREMOR DE TELA")), ShakeCheck), 12.f);
	NSUI::AddV(Box, NSUI::Row(T, Label(TEXT("DAS (atraso)")), DasSlider, DasValueText), 12.f);
	NSUI::AddV(Box, NSUI::Row(T, Label(TEXT("ARR (repetição)")), ArrSlider, ArrValueText), 32.f);
	BackButton = AddMenuButton(T, Box, TEXT("VOLTAR"), 0.f);
	NSUI::Place(Canvas, Box, FVector2D(0.5f, 0.5f), FVector2D(0.5f, 0.5f));
}

// ---------------------------------------------------------------------------
// Menu principal (GDD §7.2)
// ---------------------------------------------------------------------------
void UNSMainMenuWidget::BuildDefaultLayout()
{
	UWidgetTree* T = WidgetTree;
	UCanvasPanel* Canvas = NSUI::RootCanvas(T);

	UVerticalBox* Box = NSUI::VBox(T);
	NSUI::AddV(Box, NSUI::Text(T, TEXT("NEON STACK"), 96, NSUI::Accent(), true), 8.f, HAlign_Center);
	NSUI::AddV(Box, NSUI::Text(T, TEXT("RECORDE"), 18, NSUI::Cyan(), true), 0.f, HAlign_Center);
	HighScoreText = NSUI::Text(T, TEXT("0"), 32, NSUI::Amber(), true);
	NSUI::AddV(Box, HighScoreText, 48.f, HAlign_Center);

	PlayButton = AddMenuButton(T, Box, TEXT("JOGAR"), 14.f, 64.f);
	OptionsButton = AddMenuButton(T, Box, TEXT("OPÇÕES"), 14.f);
	QuitButton = AddMenuButton(T, Box, TEXT("SAIR"), 0.f);
	NSUI::Place(Canvas, Box, FVector2D(0.5f, 0.5f), FVector2D(0.5f, 0.5f));
}
