#include "Simulation/NSScoring.h"

void FNSScoring::Reset(const FNSSimConfig& Cfg)
{
	Score = 0;
	Lines = 0;
	Level = Cfg.StartLevel;
	Combo = -1;
}

FNSScoring::FLockResult FNSScoring::ApplyLock(int32 LinesCleared, const FNSSimConfig& Cfg)
{
	FLockResult Result;
	if (LinesCleared <= 0)
	{
		Combo = -1;
		return Result;
	}

	const int32 LevelForScore = Level;
	++Combo;
	Result.Gained = Cfg.LineScore[FMath::Clamp(LinesCleared, 0, 4)] * LevelForScore;
	if (Combo >= 1)
	{
		Result.Gained += Cfg.ComboBonus * Combo * LevelForScore;
	}
	Score += Result.Gained;
	Lines += LinesCleared;

	int32 NewLevel = Cfg.StartLevel + Lines / FMath::Max(1, Cfg.LinesPerLevel);
	if (Cfg.MaxLevel > 0)
	{
		NewLevel = FMath::Min(NewLevel, Cfg.MaxLevel);
	}
	Result.bLevelUp = NewLevel > Level;
	Level = NewLevel;
	return Result;
}

void FNSScoring::AddDropPoints(int32 Cells, bool bHardDrop, const FNSSimConfig& Cfg)
{
	Score += static_cast<int64>(Cells) * (bHardDrop ? Cfg.HardDropPointsPerCell : Cfg.SoftDropPointsPerCell);
}

float FNSScoring::GetGravityInterval(const FNSSimConfig& Cfg) const
{
	const int32 N = FMath::Max(0, Level - 1);
	const float Base = FMath::Max(0.f, Cfg.GravityBase - N * Cfg.GravityLevelStep);
	return FMath::Max(Cfg.MinGravity, FMath::Pow(Base, static_cast<float>(N))); // nível 1: Base^0 = 1,0 s
}
