#include "Game/NSMenuGameMode.h"

#include "Tetris.h"
#include "Blueprint/UserWidget.h"
#include "GameFramework/PlayerController.h"
#include "UI/NSMainMenuWidget.h"
#include "Game/NSSceneBootstrap.h"

ANSMenuGameMode::ANSMenuGameMode()
{
	PlayerControllerClass = APlayerController::StaticClass();
	DefaultPawnClass = nullptr;
	MainMenuClass = UNSMainMenuWidget::StaticClass();
}

void ANSMenuGameMode::StartPlay()
{
	NSBootstrap::EnsureScene(GetWorld(), /*bGameplay*/ false);
	Super::StartPlay();
}

void ANSMenuGameMode::BeginPlay()
{
	Super::BeginPlay();

	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	if (!PC || !MainMenuClass)
	{
		UE_LOG(LogNeonStack, Error, TEXT("NSMenuGameMode: MainMenuClass não configurado ou sem PlayerController."));
		return;
	}

	UNSMainMenuWidget* Menu = CreateWidget<UNSMainMenuWidget>(PC, MainMenuClass);
	Menu->AddToViewport();
	Menu->FocusDefault();
}
