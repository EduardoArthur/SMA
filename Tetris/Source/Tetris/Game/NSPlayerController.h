// Neon Stack - Enhanced Input -> ANSBoardActor.
// Spec: _gdks-output/04-engine/cpp-specs/02-gameplay.md §6

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "Simulation/NSTypes.h"
#include "NSPlayerController.generated.h"

class UInputAction;
class UInputMappingContext;
class ANSBoardActor;

UCLASS()
class TETRIS_API ANSPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, Category = "NeonStack|Input")
	TObjectPtr<UInputMappingContext> GameplayContext;

	/** M3: contexto dos menus. Opcional no M1. */
	UPROPERTY(EditDefaultsOnly, Category = "NeonStack|Input")
	TObjectPtr<UInputMappingContext> UIContext;

	UPROPERTY(EditDefaultsOnly, Category = "NeonStack|Input") TObjectPtr<UInputAction> IA_MoveLeft;
	UPROPERTY(EditDefaultsOnly, Category = "NeonStack|Input") TObjectPtr<UInputAction> IA_MoveRight;
	UPROPERTY(EditDefaultsOnly, Category = "NeonStack|Input") TObjectPtr<UInputAction> IA_SoftDrop;
	UPROPERTY(EditDefaultsOnly, Category = "NeonStack|Input") TObjectPtr<UInputAction> IA_HardDrop;
	UPROPERTY(EditDefaultsOnly, Category = "NeonStack|Input") TObjectPtr<UInputAction> IA_RotateCW;
	UPROPERTY(EditDefaultsOnly, Category = "NeonStack|Input") TObjectPtr<UInputAction> IA_RotateCCW;
	UPROPERTY(EditDefaultsOnly, Category = "NeonStack|Input") TObjectPtr<UInputAction> IA_Hold;
	UPROPERTY(EditDefaultsOnly, Category = "NeonStack|Input") TObjectPtr<UInputAction> IA_Pause;

	UFUNCTION(BlueprintCallable, Category = "NeonStack")
	void SetBoard(ANSBoardActor* InBoard);

	/** true = contexto de gameplay; false = contexto de UI. */
	UFUNCTION(BlueprintCallable, Category = "NeonStack")
	void SetGameplayInputEnabled(bool bEnabled);

protected:
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;

private:
	void BindPressRelease(class UEnhancedInputComponent* EIC, const UInputAction* IA, ENSInputAction Action);
	void HandlePressed(ENSInputAction Action);
	void HandleReleased(ENSInputAction Action);
	void HandlePause();

	TWeakObjectPtr<ANSBoardActor> Board;
};
