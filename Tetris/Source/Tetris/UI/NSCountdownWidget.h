// Neon Stack - contagem 3-2-1 antes de começar/retomar.
// WBP_Countdown (pai: NSCountdownWidget) precisa de um TextBlock chamado CountText.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "NSCountdownWidget.generated.h"

class UTextBlock;

UCLASS(Abstract)
class TETRIS_API UNSCountdownWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/** Mostra o widget e começa a contar. Ao terminar, chama ANSGameMode::FinishCountdown. */
	UFUNCTION(BlueprintCallable, Category = "NeonStack|UI")
	void StartCountdown();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NeonStack|UI", meta = (ClampMin = 1))
	int32 CountFrom = 3;

	/** Duração de cada número (GDD: contagem total <= 1,5 s). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NeonStack|UI", meta = (ClampMin = 0.05, Units = "s"))
	float StepTime = 0.5f;

protected:
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	/** Gancho para SFX/animação de cada número (M4). */
	UFUNCTION(BlueprintImplementableEvent, Category = "NeonStack|UI")
	void OnNumberShown(int32 Number);

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> CountText;

private:
	void ShowNumber(int32 Number);

	bool bRunning = false;
	float Elapsed = 0.f;
	int32 CurrentNumber = 0;
};
