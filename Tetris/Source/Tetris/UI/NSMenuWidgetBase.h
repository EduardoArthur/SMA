// Neon Stack - base dos widgets navegáveis por teclado (pausa, resultados, opções, menu).

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "NSMenuWidgetBase.generated.h"

UCLASS(Abstract)
class TETRIS_API UNSMenuWidgetBase : public UUserWidget
{
	GENERATED_BODY()

public:
	/** Põe o input em modo UI e dá foco de teclado ao widget padrão (ex.: botão "Continuar"). */
	UFUNCTION(BlueprintCallable, Category = "NeonStack|UI")
	void FocusDefault();

protected:
	/** Widget que recebe o foco ao abrir. */
	virtual UWidget* GetDefaultFocusWidget() const { return nullptr; }
};
