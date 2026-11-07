// Neon Stack - DataAsset com todos os números ajustáveis do jogo.
// Spec: _gdks-output/04-engine/cpp-specs/02-gameplay.md §1

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Simulation/NSTypes.h"
#include "NSGameConfig.generated.h"

UCLASS(BlueprintType)
class TETRIS_API UNSGameConfig : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	// --- Board ---
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NeonStack|Board", meta = (ClampMin = 4))
	int32 BoardWidth = 10;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NeonStack|Board", meta = (ClampMin = 4))
	int32 VisibleHeight = 20;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NeonStack|Board", meta = (ClampMin = 1))
	int32 HiddenRows = 2;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NeonStack|Board", meta = (ClampMin = 0, ClampMax = 6))
	int32 PreviewCount = 3;

	// --- Handling (padrões; as opções do jogador sobrescrevem DAS/ARR a partir do M3) ---
	UPROPERTY(EditAnywhere, Category = "NeonStack|Handling", meta = (ClampMin = 0, Units = "s"))
	float DAS = 0.170f;

	UPROPERTY(EditAnywhere, Category = "NeonStack|Handling", meta = (ClampMin = 0, Units = "s"))
	float ARR = 0.050f;

	UPROPERTY(EditAnywhere, Category = "NeonStack|Handling", meta = (ClampMin = 1))
	float SoftDropFactor = 20.f;

	UPROPERTY(EditAnywhere, Category = "NeonStack|Handling", meta = (ClampMin = 0.001, Units = "s"))
	float SoftDropMaxInterval = 0.033f;

	// --- Lock ---
	UPROPERTY(EditAnywhere, Category = "NeonStack|Lock", meta = (ClampMin = 0, Units = "s"))
	float LockDelay = 0.5f;

	UPROPERTY(EditAnywhere, Category = "NeonStack|Lock", meta = (ClampMin = 0))
	int32 MaxLockResets = 15;

	UPROPERTY(EditAnywhere, Category = "NeonStack|Lock", meta = (ClampMin = 0, Units = "s"))
	float LockWarningTime = 0.2f;

	UPROPERTY(EditAnywhere, Category = "NeonStack|Lock")
	bool bAllowUpKick = true;

	UPROPERTY(EditAnywhere, Category = "NeonStack|Lock", meta = (ClampMin = 0, Units = "s"))
	float LineClearDelay = 0.25f;

	// --- Score ---
	/** Pontos por 0, 1, 2, 3 e 4 linhas (multiplicados pelo nível). */
	UPROPERTY(EditAnywhere, Category = "NeonStack|Score", EditFixedSize)
	TArray<int32> LineScore = { 0, 100, 300, 500, 800 };

	UPROPERTY(EditAnywhere, Category = "NeonStack|Score")
	int32 ComboBonus = 50;

	UPROPERTY(EditAnywhere, Category = "NeonStack|Score")
	int32 SoftDropPointsPerCell = 1;

	UPROPERTY(EditAnywhere, Category = "NeonStack|Score")
	int32 HardDropPointsPerCell = 2;

	// --- Progression ---
	UPROPERTY(EditAnywhere, Category = "NeonStack|Progression", meta = (ClampMin = 1))
	int32 LinesPerLevel = 10;

	UPROPERTY(EditAnywhere, Category = "NeonStack|Progression", meta = (ClampMin = 1))
	int32 StartLevel = 1;

	UPROPERTY(EditAnywhere, Category = "NeonStack|Progression")
	float GravityBase = 0.8f;

	UPROPERTY(EditAnywhere, Category = "NeonStack|Progression")
	float GravityLevelStep = 0.007f;

	UPROPERTY(EditAnywhere, Category = "NeonStack|Progression", meta = (ClampMin = 0.001, Units = "s"))
	float MinGravity = 0.05f;

	/** 0 = sem teto. */
	UPROPERTY(EditAnywhere, Category = "NeonStack|Progression", meta = (ClampMin = 0))
	int32 MaxLevel = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NeonStack|Progression", meta = (ClampMin = 1))
	int32 PaletteEveryNLevels = 5;

	// --- Debug ---
	/** 0 = aleatória a cada partida. Use um valor fixo para reproduzir uma sequência de peças. */
	UPROPERTY(EditAnywhere, Category = "NeonStack|Debug")
	int32 RandomSeed = 0;

	/** Converte para a struct usada pela simulação. */
	FNSSimConfig ToSimConfig() const;
};
