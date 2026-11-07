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

void ANSPlayerController::BeginPlay()
{
	Super::BeginPlay();
	SetGameplayInputEnabled(true);

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

void ANSPlayerController::SetGameplayInputEnabled(bool bEnabled)
{
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
