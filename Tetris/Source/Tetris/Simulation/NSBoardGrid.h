// Neon Stack - grade do poço e peça ativa.
// Spec: _gdks-output/04-engine/cpp-specs/01-simulation.md §3-4

#pragma once

#include "CoreMinimal.h"
#include "Simulation/NSTypes.h"
#include "Simulation/NSPieceTables.h"

struct FNSActivePiece
{
	ENSPieceType Type = ENSPieceType::None;
	int32 Rotation = 0;
	FIntPoint Origin = FIntPoint::ZeroValue;

	bool IsValid() const { return Type != ENSPieceType::None; }

	/** Células absolutas na grade. Só chamar com peça válida. */
	void GetCells(FIntPoint OutCells[4]) const
	{
		const TConstArrayView<FIntPoint> Rel = FNSPieceTables::GetCells(Type, Rotation);
		for (int32 i = 0; i < 4; ++i)
		{
			OutCells[i] = Origin + Rel[i];
		}
	}

	FNSActivePiece Moved(int32 Dx, int32 Dy) const
	{
		FNSActivePiece P = *this;
		P.Origin += FIntPoint(Dx, Dy);
		return P;
	}

	/** Dir > 0 = horário, Dir < 0 = anti-horário. */
	FNSActivePiece Rotated(int32 Dir) const
	{
		FNSActivePiece P = *this;
		P.Rotation = (Rotation + (Dir > 0 ? 1 : 3)) & 3;
		return P;
	}
};

class TETRIS_API FNSBoardGrid
{
public:
	/** Redimensiona e zera a grade. */
	void Init(int32 InWidth, int32 InHeight);

	int32 GetWidth() const { return Width; }
	int32 GetHeight() const { return Height; }

	/** Valor da célula (0 = vazia, 1..7 = ENSPieceType). Fora da grade retorna 0. */
	uint8 Get(int32 X, int32 Y) const;

	/** X fora de [0,W) ou Y < 0 -> ocupado; Y >= H -> livre. */
	bool IsFree(int32 X, int32 Y) const;

	bool CanPlace(const FNSActivePiece& Piece) const;

	/** Escreve as 4 células (ignora Y >= Height). Retorna as linhas distintas tocadas. */
	void Lock(const FNSActivePiece& Piece, TArray<int32>& OutTouchedRows);

	/** Das linhas candidatas, retorna as completas em ordem crescente. */
	void FindFullRows(const TArray<int32>& CandidateRows, TArray<int32>& OutFullRows) const;

	/** Remove as linhas indicadas e desce as de cima. */
	void ClearRows(const TArray<int32>& Rows);

	/** Posição de pouso do hard drop. */
	FNSActivePiece GetDropPosition(const FNSActivePiece& Piece) const;

	/** Só para testes. */
	void Test_SetCell(int32 X, int32 Y, uint8 Value);

private:
	int32 Index(int32 X, int32 Y) const { return Y * Width + X; }
	bool IsInside(int32 X, int32 Y) const { return X >= 0 && X < Width && Y >= 0 && Y < Height; }

	int32 Width = 0;
	int32 Height = 0;
	TArray<uint8> Cells; // índice = Y * Width + X
};
