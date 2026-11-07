#include "Simulation/NSRandomizer.h"

void FNSRandomizer::Reset(int32 Seed, int32 InMinQueue)
{
	MinQueue = FMath::Max(1, InMinQueue);
	if (Seed == 0)
	{
		Stream.GenerateNewSeed();
	}
	else
	{
		Stream.Initialize(Seed);
	}
	Queue.Reset();
	while (Queue.Num() <= MinQueue)
	{
		AppendBag();
	}
}

ENSPieceType FNSRandomizer::Pop()
{
	while (Queue.Num() <= MinQueue)
	{
		AppendBag();
	}
	const ENSPieceType Next = Queue[0]; // FIFO: retira do início (não usar TArray::Pop, que tira do fim)
	Queue.RemoveAt(0);
	return Next;
}

void FNSRandomizer::Peek(int32 Count, TArray<ENSPieceType>& Out) const
{
	Out.Reset();
	const int32 N = FMath::Min(Count, Queue.Num());
	for (int32 i = 0; i < N; ++i)
	{
		Out.Add(Queue[i]);
	}
}

void FNSRandomizer::Test_ForceQueue(const TArray<ENSPieceType>& Pieces)
{
	Queue.Insert(Pieces, 0);
}

void FNSRandomizer::AppendBag()
{
	ENSPieceType Bag[NSPieceTypeCount] = {
		ENSPieceType::I, ENSPieceType::O, ENSPieceType::T, ENSPieceType::S,
		ENSPieceType::Z, ENSPieceType::J, ENSPieceType::L
	};
	for (int32 i = NSPieceTypeCount - 1; i > 0; --i) // Fisher-Yates
	{
		Swap(Bag[i], Bag[Stream.RandRange(0, i)]);
	}
	Queue.Append(Bag, NSPieceTypeCount);
}
