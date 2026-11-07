// Neon Stack - pontuação, combo, nível e gravidade.
// Spec: _gdks-output/04-engine/cpp-specs/01-simulation.md §6

#pragma once

#include "CoreMinimal.h"
#include "Simulation/NSTypes.h"

struct TETRIS_API FNSScoring
{
	int64 Score = 0;
	int32 Lines = 0;
	int32 Level = 1;
	int32 Combo = -1;

	struct FLockResult
	{
		int32 Gained = 0;
		bool bLevelUp = false;
	};

	void Reset(const FNSSimConfig& Cfg);

	/** LinesCleared 0..4. Usa o nível ANTES do level up. */
	FLockResult ApplyLock(int32 LinesCleared, const FNSSimConfig& Cfg);

	void AddDropPoints(int32 Cells, bool bHardDrop, const FNSSimConfig& Cfg);

	/** max(Min, (Base - (n-1)*Step)^(n-1)) segundos por linha. */
	float GetGravityInterval(const FNSSimConfig& Cfg) const;
};
