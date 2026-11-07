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
	NSBootstrap::ActivateCamera(GetWorld());

	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	if (!PC || !MainMenuClass)
	{
		UE_LOG(LogNeonStack, Error, TEXT("NSMenuGameMode: MainMenuClass não configurado ou sem PlayerController."));
		return;
	}

	UNSMainMenuWidget* Menu = CreateWidget<UNSMainMenuWidget>(PC, MainMenuClass);
	if (!Menu)
	{
		UE_LOG(LogNeonStack, Error, TEXT("NSMenuGameMode: falhou ao criar o menu."));
		return;
	}
	Menu->AddToViewport();
	Menu->FocusDefault();
	UE_LOG(LogNeonStack, Log, TEXT("NSMenuGameMode: menu criado."));
}
