#include "Data/NSGameConfig.h"

FNSSimConfig UNSGameConfig::ToSimConfig() const
{
	FNSSimConfig C;
	C.BoardWidth = BoardWidth;
	C.VisibleHeight = VisibleHeight;
	C.HiddenRows = HiddenRows;
	C.PreviewCount = PreviewCount;

	C.DAS = DAS;
	C.ARR = ARR;
	C.SoftDropFactor = SoftDropFactor;
	C.SoftDropMaxInterval = SoftDropMaxInterval;

	C.LockDelay = LockDelay;
	C.MaxLockResets = MaxLockResets;
	C.LockWarningTime = LockWarningTime;
	C.bAllowUpKick = bAllowUpKick;
	C.LineClearDelay = LineClearDelay;

	const int32 NumScores = FMath::Min(LineScore.Num(), 5);
	for (int32 i = 0; i < NumScores; ++i)
	{
		C.LineScore[i] = LineScore[i];
	}
	C.ComboBonus = ComboBonus;
	C.SoftDropPointsPerCell = SoftDropPointsPerCell;
	C.HardDropPointsPerCell = HardDropPointsPerCell;

	C.LinesPerLevel = LinesPerLevel;
	C.StartLevel = StartLevel;
	C.GravityBase = GravityBase;
	C.GravityLevelStep = GravityLevelStep;
	C.MinGravity = MinGravity;
	C.MaxLevel = MaxLevel;
	return C;
}
