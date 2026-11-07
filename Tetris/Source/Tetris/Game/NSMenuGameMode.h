// Neon Stack - GameMode do menu principal (L_MainMenu).

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "NSMenuGameMode.generated.h"

class UNSMainMenuWidget;

UCLASS()
class TETRIS_API ANSMenuGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ANSMenuGameMode();

	/** WBP_MainMenu (pai: NSMainMenuWidget). */
	UPROPERTY(EditDefaultsOnly, Category = "NeonStack")
	TSubclassOf<UNSMainMenuWidget> MainMenuClass;

	virtual void StartPlay() override;

protected:
	virtual void BeginPlay() override;
};
