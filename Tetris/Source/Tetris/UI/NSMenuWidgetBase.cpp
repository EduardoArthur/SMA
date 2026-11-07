#include "UI/NSMenuWidgetBase.h"

#include "GameFramework/PlayerController.h"

void UNSMenuWidgetBase::FocusDefault()
{
	APlayerController* PC = GetOwningPlayer();
	if (!PC)
	{
		return;
	}

	UWidget* Target = GetDefaultFocusWidget();
	FInputModeGameAndUI Mode; // GameAndUI: teclas não tratadas pela UI (ex.: Esc) ainda chegam ao Enhanced Input
	Mode.SetWidgetToFocus(Target ? Target->TakeWidget() : TakeWidget());
	Mode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	Mode.SetHideCursorDuringCapture(false);
	PC->SetInputMode(Mode);
	PC->SetShowMouseCursor(false);

	if (Target)
	{
		Target->SetKeyboardFocus();
	}
	else
	{
		SetKeyboardFocus();
	}
}
