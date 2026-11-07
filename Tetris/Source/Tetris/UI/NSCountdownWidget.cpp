#include "UI/NSCountdownWidget.h"

#include "Components/TextBlock.h"
#include "Engine/World.h"
#include "Game/NSGameMode.h"
#include "Audio/NSAudioDirector.h"

void UNSCountdownWidget::StartCountdown()
{
	bRunning = true;
	Elapsed = 0.f;
	CurrentNumber = 0;
	SetVisibility(ESlateVisibility::HitTestInvisible);
	ShowNumber(CountFrom);
}

void UNSCountdownWidget::ShowNumber(int32 Number)
{
	if (Number == CurrentNumber)
	{
		return;
	}
	CurrentNumber = Number;
	CountText->SetText(FText::AsNumber(Number));
	if (ANSAudioDirector* Audio = ANSAudioDirector::Get(this))
	{
		Audio->PlayCountdown(Number);
	}
	OnNumberShown(Number);
}

void UNSCountdownWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (!bRunning)
	{
		return;
	}

	Elapsed += InDeltaTime;
	const float Total = CountFrom * StepTime;
	if (Elapsed >= Total)
	{
		bRunning = false;
		SetVisibility(ESlateVisibility::Collapsed);
		if (ANSGameMode* GM = GetWorld() ? GetWorld()->GetAuthGameMode<ANSGameMode>() : nullptr)
		{
			GM->FinishCountdown();
		}
		return;
	}
	ShowNumber(CountFrom - FMath::FloorToInt(Elapsed / StepTime));
}
