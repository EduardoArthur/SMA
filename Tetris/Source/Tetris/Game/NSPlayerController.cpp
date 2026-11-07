#include "Game/NSPlayerController.h"

#include "Tetris.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputAction.h"
#include "InputMappingContext.h"
#include "Engine/LocalPlayer.h"
#include "EngineUtils.h"
#include "Game/NSBoardActor.h"
#include "Game/NSGameMode.h"
#include "NSDefaultAssets.h"
#include "UI/NSCountdownWidget.h"
#include "UI/NSHUDWidget.h"
#include "UI/NSOptionsWidget.h"
#include "UI/NSPauseWidget.h"
#include "UI/NSResultsWidget.h"

void ANSPlayerController::BeginPlay()
{
	Super::BeginPlay();
	if (!GameMode.IsValid()) // se o GameMode já iniciou a UI, ele controla o input
	{
		SetGameplayInputEnabled(true);
	}

	if (!Board.IsValid())
	{
		for (TActorIterator<ANSBoardActor> It(GetWorld()); It; ++It)
		{
			Board = *It;
			break;
		}
	}
}

void ANSPlayerController::SetBoard(ANSBoardActor* InBoard)
{
	Board = InBoard;
}

void ANSPlayerController::EnsureDefaults()
{
	if (bDefaultsEnsured)
	{
		return;
	}
	bDefaultsEnsured = true;

	auto FillIA = [](TObjectPtr<UInputAction>& Ptr, const TCHAR* Name)
	{
		if (!Ptr) { Ptr = NSAssets::LoadInput<UInputAction>(Name); }
	};
	FillIA(IA_MoveLeft, TEXT("IA_MoveLeft"));
	FillIA(IA_MoveRight, TEXT("IA_MoveRight"));
	FillIA(IA_SoftDrop, TEXT("IA_SoftDrop"));
	FillIA(IA_HardDrop, TEXT("IA_HardDrop"));
	FillIA(IA_RotateCW, TEXT("IA_RotateCW"));
	FillIA(IA_RotateCCW, TEXT("IA_RotateCCW"));
	FillIA(IA_Hold, TEXT("IA_Hold"));
	FillIA(IA_Pause, TEXT("IA_Pause"));

	if (!GameplayContext) { GameplayContext = NSAssets::LoadInput<UInputMappingContext>(TEXT("IMC_Gameplay")); }
	if (!UIContext) { UIContext = NSAssets::LoadInput<UInputMappingContext>(TEXT("IMC_UI")); }

	// Sem WBP: usa as próprias classes C++, que montam um layout padrão
	if (!HUDClass) { HUDClass = UNSHUDWidget::StaticClass(); }
	if (!CountdownClass) { CountdownClass = UNSCountdownWidget::StaticClass(); }
	if (!PauseClass) { PauseClass = UNSPauseWidget::StaticClass(); }
	if (!ResultsClass) { ResultsClass = UNSResultsWidget::StaticClass(); }
	if (!OptionsClass) { OptionsClass = UNSOptionsWidget::StaticClass(); }
}

// ---------------------------------------------------------------------------
// UI
// ---------------------------------------------------------------------------

void ANSPlayerController::InitUI(ANSGameMode* InGameMode, ANSBoardActor* InBoard)
{
	EnsureDefaults();
	GameMode = InGameMode;
	if (InGameMode)
	{
		InGameMode->OnFlowStateChanged.AddUniqueDynamic(this, &ANSPlayerController::HandleFlowStateChanged);
	}

	if (HUDClass && !HUDWidget)
	{
		HUDWidget = CreateWidget<UNSHUDWidget>(this, HUDClass);
		HUDWidget->AddToViewport(0);
		HUDWidget->InitForBoard(InBoard);
	}
	if (CountdownClass && !CountdownWidget)
	{
		CountdownWidget = CreateWidget<UNSCountdownWidget>(this, CountdownClass);
		CountdownWidget->AddToViewport(5);
		CountdownWidget->SetVisibility(ESlateVisibility::Collapsed);
	}
	if (PauseClass && !PauseWidget)
	{
		PauseWidget = CreateWidget<UNSPauseWidget>(this, PauseClass);
		PauseWidget->AddToViewport(10);
		PauseWidget->SetVisibility(ESlateVisibility::Collapsed);
	}
	if (ResultsClass && !ResultsWidget)
	{
		ResultsWidget = CreateWidget<UNSResultsWidget>(this, ResultsClass);
		ResultsWidget->AddToViewport(10);
		ResultsWidget->SetVisibility(ESlateVisibility::Collapsed);
	}

	if (!HUDClass || !CountdownClass || !PauseClass || !ResultsClass)
	{
		UE_LOG(LogNeonStack, Warning, TEXT("NSPlayerController: alguma classe de widget (HUD/Countdown/Pause/Results) não foi atribuída no BP_NSPlayerController."));
	}
}

void ANSPlayerController::HandleFlowStateChanged(ENSFlowState NewState)
{
	auto Hide = [](UUserWidget* W) { if (W) { W->SetVisibility(ESlateVisibility::Collapsed); } };

	if (NewState != ENSFlowState::Paused && OptionsWidget)
	{
		Hide(OptionsWidget);
	}

	switch (NewState)
	{
	case ENSFlowState::Countdown:
		Hide(PauseWidget);
		Hide(ResultsWidget);
		SetGameplayInputEnabled(false);
		if (CountdownWidget)
		{
			CountdownWidget->StartCountdown();
		}
		else if (GameMode.IsValid())
		{
			GameMode->FinishCountdown(); // sem widget de contagem: começa direto
		}
		break;

	case ENSFlowState::Playing:
		Hide(CountdownWidget);
		Hide(PauseWidget);
		Hide(ResultsWidget);
		SetGameplayInputEnabled(true);
		break;

	case ENSFlowState::Paused:
		SetGameplayInputEnabled(false);
		if (PauseWidget)
		{
			PauseWidget->SetVisibility(ESlateVisibility::Visible);
			PauseWidget->FocusDefault();
		}
		break;

	case ENSFlowState::GameOver:
		SetGameplayInputEnabled(false);
		break;

	case ENSFlowState::Results:
		SetGameplayInputEnabled(false);
		if (ResultsWidget && GameMode.IsValid())
		{
			ResultsWidget->SetResult(GameMode->GetLastScore(), GameMode->GetLastLines(), GameMode->GetLastLevel(), GameMode->WasLastNewHighScore());
			ResultsWidget->SetVisibility(ESlateVisibility::Visible);
			ResultsWidget->FocusDefault();
		}
		break;
	}
}

void ANSPlayerController::OpenOptions()
{
	if (!OptionsClass)
	{
		UE_LOG(LogNeonStack, Warning, TEXT("NSPlayerController: OptionsClass não atribuído."));
		return;
	}
	if (!OptionsWidget)
	{
		OptionsWidget = CreateWidget<UNSOptionsWidget>(this, OptionsClass);
		OptionsWidget->AddToViewport(20);
		OptionsWidget->OnClosed.AddDynamic(this, &ANSPlayerController::HandleOptionsClosed);
	}
	OptionsWidget->Open();
}

void ANSPlayerController::HandleOptionsClosed()
{
	if (PauseWidget && GameMode.IsValid() && GameMode->GetFlowState() == ENSFlowState::Paused)
	{
		PauseWidget->FocusDefault();
	}
}

void ANSPlayerController::SetGameplayInputEnabled(bool bEnabled)
{
	EnsureDefaults();
	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		Subsystem->ClearAllMappings();
		UInputMappingContext* Context = bEnabled ? GameplayContext.Get() : UIContext.Get();
		if (Context)
		{
			Subsystem->AddMappingContext(Context, 0);
		}
		else
		{
			UE_LOG(LogNeonStack, Warning, TEXT("NSPlayerController: mapping context %s não atribuído."), bEnabled ? TEXT("Gameplay") : TEXT("UI"));
		}
	}

	if (bEnabled)
	{
		SetInputMode(FInputModeGameOnly());
	}
	else
	{
		SetInputMode(FInputModeGameAndUI());
	}
	SetShowMouseCursor(false); // navegação só por teclado
}

void ANSPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();
	EnsureDefaults();

	UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(InputComponent);
	if (!EIC)
	{
		UE_LOG(LogNeonStack, Error, TEXT("NSPlayerController: InputComponent não é UEnhancedInputComponent (verifique o Project Settings > Input)."));
		return;
	}

	BindPressRelease(EIC, IA_MoveLeft, ENSInputAction::MoveLeft);
	BindPressRelease(EIC, IA_MoveRight, ENSInputAction::MoveRight);
	BindPressRelease(EIC, IA_SoftDrop, ENSInputAction::SoftDrop);
	BindPressRelease(EIC, IA_HardDrop, ENSInputAction::HardDrop);
	BindPressRelease(EIC, IA_RotateCW, ENSInputAction::RotateCW);
	BindPressRelease(EIC, IA_RotateCCW, ENSInputAction::RotateCCW);
	BindPressRelease(EIC, IA_Hold, ENSInputAction::Hold);

	if (IA_Pause)
	{
		EIC->BindAction(IA_Pause, ETriggerEvent::Started, this, &ANSPlayerController::HandlePause);
	}
}

void ANSPlayerController::BindPressRelease(UEnhancedInputComponent* EIC, const UInputAction* IA, ENSInputAction Action)
{
	if (!IA)
	{
		UE_LOG(LogNeonStack, Warning, TEXT("NSPlayerController: Input Action não atribuída (%d)."), static_cast<int32>(Action));
		return;
	}
	// Parâmetro extra (Action) é repassado como payload para o handler
	EIC->BindAction(IA, ETriggerEvent::Started, this, &ANSPlayerController::HandlePressed, Action);
	EIC->BindAction(IA, ETriggerEvent::Completed, this, &ANSPlayerController::HandleReleased, Action);
}

void ANSPlayerController::HandlePressed(ENSInputAction Action)
{
	if (ANSBoardActor* B = Board.Get())
	{
		B->PressAction(Action);
	}
}

void ANSPlayerController::HandleReleased(ENSInputAction Action)
{
	if (ANSBoardActor* B = Board.Get())
	{
		B->ReleaseAction(Action);
	}
}

void ANSPlayerController::HandlePause()
{
	if (ANSGameMode* GM = GetWorld()->GetAuthGameMode<ANSGameMode>())
	{
		GM->TogglePause();
	}
}
