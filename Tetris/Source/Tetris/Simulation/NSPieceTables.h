// Neon Stack - tabelas estáticas de peças (formas, spawn, kicks).
// Spec: _gdks-output/04-engine/cpp-specs/01-simulation.md §2

#pragma once

#include "CoreMinimal.h"
#include "Simulation/NSTypes.h"

class TETRIS_API FNSPieceTables
{
public:
	/** 4 células relativas à origem (canto inferior esquerdo da caixa), Y para cima. Rotation 0..3 (0, R, 2, L). */
	static TConstArrayView<FIntPoint> GetCells(ENSPieceType Type, int32 Rotation);

	/** Tamanho N da caixa N×N (I = 4, O = 2, demais = 3). */
	static int32 GetBoxSize(ENSPieceType Type);

	/** Origem de spawn: a linha mais alta ocupada no estado 0 fica em Y = TotalHeight - 1. */
	static FIntPoint GetSpawnOrigin(ENSPieceType Type, int32 TotalHeight);

	/** Deslocamentos a testar na rotação, em ordem. */
	static void GetKicks(ENSPieceType Type, bool bAllowUpKick, TArray<FIntPoint, TInlineAllocator<6>>& OutKicks);

	static bool CanRotate(ENSPieceType Type) { return Type != ENSPieceType::O; }
};
