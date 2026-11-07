// Neon Stack - sorteio por saco de 7.
// Spec: _gdks-output/04-engine/cpp-specs/01-simulation.md §5

#pragma once

#include "CoreMinimal.h"
#include "Math/RandomStream.h"
#include "Simulation/NSTypes.h"

class TETRIS_API FNSRandomizer
{
public:
	/** Seed 0 = aleatória. MinQueue = PreviewCount + 1. */
	void Reset(int32 Seed, int32 InMinQueue);

	/** Retira a próxima peça (FIFO) e reabastece. */
	ENSPieceType Pop();

	/** Copia as próximas Count peças sem remover. */
	void Peek(int32 Count, TArray<ENSPieceType>& Out) const;

	/** Só para testes: insere peças no início da fila. */
	void Test_ForceQueue(const TArray<ENSPieceType>& Pieces);

private:
	void AppendBag();

	FRandomStream Stream;
	TArray<ENSPieceType> Queue;
	int32 MinQueue = 4;
};
