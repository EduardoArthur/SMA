#include "Simulation/NSBoardGrid.h"

void FNSBoardGrid::Init(int32 InWidth, int32 InHeight)
{
	Width = InWidth;
	Height = InHeight;
	Cells.Init(0, Width * Height);
}

uint8 FNSBoardGrid::Get(int32 X, int32 Y) const
{
	return IsInside(X, Y) ? Cells[Index(X, Y)] : 0;
}

bool FNSBoardGrid::IsFree(int32 X, int32 Y) const
{
	if (X < 0 || X >= Width || Y < 0)
	{
		return false;
	}
	if (Y >= Height)
	{
		return true;
	}
	return Cells[Index(X, Y)] == 0;
}

bool FNSBoardGrid::CanPlace(const FNSActivePiece& Piece) const
{
	FIntPoint PieceCells[4];
	Piece.GetCells(PieceCells);
	for (const FIntPoint& C : PieceCells)
	{
		if (!IsFree(C.X, C.Y))
		{
			return false;
		}
	}
	return true;
}

void FNSBoardGrid::Lock(const FNSActivePiece& Piece, TArray<int32>& OutTouchedRows)
{
	OutTouchedRows.Reset();
	FIntPoint PieceCells[4];
	Piece.GetCells(PieceCells);
	for (const FIntPoint& C : PieceCells)
	{
		if (IsInside(C.X, C.Y))
		{
			Cells[Index(C.X, C.Y)] = static_cast<uint8>(Piece.Type);
			OutTouchedRows.AddUnique(C.Y);
		}
	}
}

void FNSBoardGrid::FindFullRows(const TArray<int32>& CandidateRows, TArray<int32>& OutFullRows) const
{
	OutFullRows.Reset();
	for (const int32 Y : CandidateRows)
	{
		if (Y < 0 || Y >= Height || OutFullRows.Contains(Y))
		{
			continue;
		}
		bool bFull = true;
		for (int32 X = 0; X < Width; ++X)
		{
			if (Cells[Index(X, Y)] == 0)
			{
				bFull = false;
				break;
			}
		}
		if (bFull)
		{
			OutFullRows.Add(Y);
		}
	}
	OutFullRows.Sort();
}

void FNSBoardGrid::ClearRows(const TArray<int32>& Rows)
{
	if (Rows.Num() == 0)
	{
		return;
	}
	int32 Write = 0;
	for (int32 Read = 0; Read < Height; ++Read)
	{
		if (Rows.Contains(Read))
		{
			continue; // linha removida
		}
		if (Write != Read)
		{
			FMemory::Memcpy(&Cells[Index(0, Write)], &Cells[Index(0, Read)], Width);
		}
		++Write;
	}
	for (; Write < Height; ++Write)
	{
		FMemory::Memzero(&Cells[Index(0, Write)], Width); // novas linhas vazias no topo
	}
}

FNSActivePiece FNSBoardGrid::GetDropPosition(const FNSActivePiece& Piece) const
{
	FNSActivePiece P = Piece;
	while (CanPlace(P.Moved(0, -1)))
	{
		P = P.Moved(0, -1);
	}
	return P;
}

void FNSBoardGrid::Test_SetCell(int32 X, int32 Y, uint8 Value)
{
	if (IsInside(X, Y))
	{
		Cells[Index(X, Y)] = Value;
	}
}
