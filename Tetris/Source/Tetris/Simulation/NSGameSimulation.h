// Neon Stack - simulação pura do jogo (sem UObject, determinística).
// Spec: _gdks-output/04-engine/cpp-specs/01-simulation.md §7

#pragma once

#include "CoreMinimal.h"
#include "Simulation/NSTypes.h"
#include "Simulation/NSBoardGrid.h"
#include "Simulation/NSRandomizer.h"
#include "Simulation/NSScoring.h"

class TETRIS_API FNSGameSimulation
{
public:
	void Configure(const FNSSimConfig& InConfig) { Config = InConfig; }
	void SetHandling(float InDAS, float InARR) { Config.DAS = InDAS; Config.ARR = InARR; }

	/** Zera tudo e faz o primeiro spawn. Seed 0 = aleatória. */
	void Start(int32 Seed, TArray<FNSGameEvent>& OutEvents);

	/** Avança Dt segundos (o chamador limita o Dt). */
	void Tick(float Dt, const FNSInputFrame& Input, TArray<FNSGameEvent>& OutEvents);

	// --- Leitura ---
	ENSSimState GetState() const { return State; }
	const FNSBoardGrid& GetBoard() const { return Board; }
	const FNSActivePiece& GetActive() const { return Active; } // inválida em ClearingLines/GameOver
	FNSActivePiece GetGhost() const { return Active.IsValid() ? Board.GetDropPosition(Active) : Active; }
	ENSPieceType GetHoldPiece() const { return HoldPiece; }
	bool CanHold() const { return bCanHold; }
	void GetQueue(TArray<ENSPieceType>& Out) const { Randomizer.Peek(Config.PreviewCount, Out); }
	const FNSScoring& GetScoring() const { return Scoring; }
	uint32 GetBoardRevision() const { return BoardRevision; }
	const FNSSimConfig& GetConfig() const { return Config; }

	// --- Só para testes ---
	FNSBoardGrid& Test_GetBoard() { return Board; }
	FNSScoring& Test_GetScoring() { return Scoring; }
	void Test_ForceQueue(const TArray<ENSPieceType>& Pieces) { Randomizer.Test_ForceQueue(Pieces); }
	void Test_SetActive(const FNSActivePiece& Piece);

private:
	// Ações
	bool TryMove(int32 Dx, TArray<FNSGameEvent>& Out);
	bool TryRotate(int32 Dir, TArray<FNSGameEvent>& Out);
	void DoHardDrop(TArray<FNSGameEvent>& Out);
	void DoHold(TArray<FNSGameEvent>& Out);

	// Ciclo de vida da peça
	void SpawnPiece(ENSPieceType Type, TArray<FNSGameEvent>& Out);
	void SpawnNextFromQueue(TArray<FNSGameEvent>& Out);
	void LockPiece(TArray<FNSGameEvent>& Out);
	void FinishLineClear(TArray<FNSGameEvent>& Out);
	void EnterGameOver(TArray<FNSGameEvent>& Out);

	// Etapas do Tick
	void ProcessPressed(const FNSInputFrame& Input, TArray<FNSGameEvent>& Out);
	void UpdateAutoShift(float Dt, const FNSInputFrame& Input, TArray<FNSGameEvent>& Out);
	void UpdateGravity(float Dt, const FNSInputFrame& Input, TArray<FNSGameEvent>& Out);
	void UpdateLock(float Dt, TArray<FNSGameEvent>& Out);
	void OnSuccessfulManipulation();
	void ResetShift();

	bool IsPieceControllable() const { return State == ENSSimState::Falling || State == ENSSimState::Grounded; }
	bool IsGrounded() const { return !Board.CanPlace(Active.Moved(0, -1)); }

	// Eventos
	void EmitPieceEvent(ENSGameEventType Type, TArray<FNSGameEvent>& Out, int32 IntA = 0, bool bFlag = false) const;
	void EmitSimple(ENSGameEventType Type, TArray<FNSGameEvent>& Out) const;
	void EmitScore(TArray<FNSGameEvent>& Out) const;
	void EmitQueue(TArray<FNSGameEvent>& Out) const;
	void EmitHold(TArray<FNSGameEvent>& Out) const;

	FNSSimConfig Config;
	FNSBoardGrid Board;
	FNSRandomizer Randomizer;
	FNSScoring Scoring;
	FNSActivePiece Active;

	ENSSimState State = ENSSimState::Ready;
	ENSPieceType HoldPiece = ENSPieceType::None;
	bool bCanHold = true;

	float GravityTimer = 0.f;
	float LockTimer = 0.f;
	int32 LockResets = 0;
	bool bLockWarned = false;

	float ClearTimer = 0.f;
	TArray<int32> PendingClearRows;

	int32 ShiftDir = 0; // -1, 0, +1
	float DasTimer = 0.f;
	float ArrTimer = 0.f;
	bool bDasCharged = false;

	uint32 BoardRevision = 0;
};
