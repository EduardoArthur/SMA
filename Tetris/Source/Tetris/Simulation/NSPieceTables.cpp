#include "Simulation/NSPieceTables.h"

namespace NSPieceTablesPrivate
{
	// Estado 0 de cada peça em coordenadas da caixa (Y para cima). Ordem: I, O, T, S, Z, J, L.
	static const FIntPoint GState0[NSPieceTypeCount][4] = {
		{ {0, 2}, {1, 2}, {2, 2}, {3, 2} }, // I
		{ {0, 0}, {1, 0}, {0, 1}, {1, 1} }, // O
		{ {1, 2}, {0, 1}, {1, 1}, {2, 1} }, // T
		{ {1, 2}, {2, 2}, {0, 1}, {1, 1} }, // S
		{ {0, 2}, {1, 2}, {1, 1}, {2, 1} }, // Z
		{ {0, 2}, {0, 1}, {1, 1}, {2, 1} }, // J
		{ {2, 2}, {0, 1}, {1, 1}, {2, 1} }, // L
	};

	struct FTables
	{
		FIntPoint Cells[NSPieceTypeCount][4][4]; // [tipo-1][rotação][célula]

		FTables()
		{
			for (int32 T = 0; T < NSPieceTypeCount; ++T)
			{
				const int32 N = FNSPieceTables::GetBoxSize(static_cast<ENSPieceType>(T + 1));
				for (int32 C = 0; C < 4; ++C)
				{
					Cells[T][0][C] = GState0[T][C];
				}
				for (int32 R = 1; R < 4; ++R)
				{
					for (int32 C = 0; C < 4; ++C)
					{
						const FIntPoint P = Cells[T][R - 1][C];
						Cells[T][R][C] = FIntPoint(P.Y, N - 1 - P.X); // rotação horária
					}
				}
			}
		}
	};

	static const FTables& GetTables()
	{
		static const FTables Tables; // inicialização thread-safe (magic static)
		return Tables;
	}
}

TConstArrayView<FIntPoint> FNSPieceTables::GetCells(ENSPieceType Type, int32 Rotation)
{
	check(Type != ENSPieceType::None);
	const int32 TypeIndex = static_cast<int32>(Type) - 1;
	return TConstArrayView<FIntPoint>(NSPieceTablesPrivate::GetTables().Cells[TypeIndex][Rotation & 3], 4);
}

int32 FNSPieceTables::GetBoxSize(ENSPieceType Type)
{
	switch (Type)
	{
	case ENSPieceType::I: return 4;
	case ENSPieceType::O: return 2;
	default:              return 3;
	}
}

FIntPoint FNSPieceTables::GetSpawnOrigin(ENSPieceType Type, int32 TotalHeight)
{
	int32 MaxCy = 0;
	for (const FIntPoint& P : GetCells(Type, 0))
	{
		MaxCy = FMath::Max(MaxCy, P.Y);
	}
	const int32 Column = (Type == ENSPieceType::O) ? 4 : 3;
	return FIntPoint(Column, (TotalHeight - 1) - MaxCy); // I/T/S/Z/J/L -> Y=19, O -> Y=20 (com altura 22)
}

void FNSPieceTables::GetKicks(ENSPieceType Type, bool bAllowUpKick, TArray<FIntPoint, TInlineAllocator<6>>& OutKicks)
{
	OutKicks.Reset();
	OutKicks.Add(FIntPoint(0, 0));
	OutKicks.Add(FIntPoint(-1, 0));
	OutKicks.Add(FIntPoint(1, 0));
	if (Type == ENSPieceType::I)
	{
		OutKicks.Add(FIntPoint(-2, 0));
		OutKicks.Add(FIntPoint(2, 0));
	}
	if (bAllowUpKick)
	{
		OutKicks.Add(FIntPoint(0, 1));
	}
}
