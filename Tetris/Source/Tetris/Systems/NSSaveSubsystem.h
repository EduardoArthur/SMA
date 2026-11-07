// Neon Stack - carrega/salva recorde e opções (slot "NeonStack_Save").
// Spec: _gdks-output/04-engine/cpp-specs/02-gameplay.md §3

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Data/NSSaveGame.h"
#include "NSSaveSubsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FNSOnSettingsChanged, const FNSSettings&, Settings);

UCLASS()
class TETRIS_API UNSSaveSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	UFUNCTION(BlueprintPure, Category = "NeonStack|Save")
	FNSSettings GetSettings() const;

	/** Grava, salva no disco e dispara OnSettingsChanged. */
	UFUNCTION(BlueprintCallable, Category = "NeonStack|Save")
	void SetSettings(const FNSSettings& NewSettings);

	UFUNCTION(BlueprintPure, Category = "NeonStack|Save")
	int64 GetHighScore() const;

	UFUNCTION(BlueprintPure, Category = "NeonStack|Save")
	int32 GetHighScoreLines() const;

	UFUNCTION(BlueprintPure, Category = "NeonStack|Save")
	int32 GetHighScoreLevel() const;

	/** Registra o resultado de uma partida. Retorna true se for novo recorde (e salva). */
	UFUNCTION(BlueprintCallable, Category = "NeonStack|Save")
	bool SubmitResult(int64 Score, int32 Lines, int32 Level);

	UPROPERTY(BlueprintAssignable, Category = "NeonStack|Save")
	FNSOnSettingsChanged OnSettingsChanged;

	/** Atalho para pegar o subsystem a partir de qualquer objeto com mundo. */
	static UNSSaveSubsystem* Get(const UObject* WorldContext);

private:
	void Save();

	UPROPERTY(Transient)
	TObjectPtr<UNSSaveGame> Data;
};
