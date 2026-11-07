# C++ Spec 01 - Simulação pura (`Source/Tetris/Simulation/`)

> **Autora:** Priscilla (UE5 Programmer Lead) 💻
> **Data:** 2026-11-07 · **Versão:** 1.0
> **Base:** [ue5-architecture.md](../ue5-architecture.md) §3.1, §4 · [core-mechanics.md](../../02-design/core-design/mechanics/core-mechanics.md)
> **Marcos:** M1 (tabelas, grade, peça, simulação sem hold/preview) → M2 (randomizer, hold, ghost, DAS/ARR) → M3 (scoring completo)

Regras desta camada:
- **C++ comum.** Só `NSTypes.h` usa reflexão (`UENUM`/`USTRUCT`), porque esses tipos aparecem nos delegates. O resto não tem `UCLASS`/`UPROPERTY`.
- **Sem acesso a UWorld, tempo, input ou rendering.** Entra `Dt` + `FNSInputFrame`, sai `TArray<FNSGameEvent>`.
- **Determinística:** a mesma seed + a mesma sequência de input = o mesmo resultado.
- Coordenadas: `X` 0..9 (esq → dir), `Y` 0..21 (**baixo → cima**). Linhas 20-21 ocultas.

---

## 1. `NSTypes.h`

```cpp
#pragma once

#include "CoreMinimal.h"
#include "NSTypes.generated.h"

/** Tipo de peça. O valor numérico (1..7) é gravado nas células da grade; 0 = vazio. */
UENUM(BlueprintType)
enum class ENSPieceType : uint8
{
	None = 0,
	I, O, T, S, Z, J, L
};

static constexpr int32 NSPieceTypeCount = 7;

/** Ações de input que a simulação entende. */
UENUM(BlueprintType)
enum class ENSInputAction : uint8
{
	MoveLeft,
	MoveRight,
	SoftDrop,
	HardDrop,
	RotateCW,
	RotateCCW,
	Hold
};

/** Estado interno da simulação (Spawning e Locking são instantâneos e não aparecem aqui). */
UENUM(BlueprintType)
enum class ENSSimState : uint8
{
	Ready,          // antes do StartGame
	Falling,        // peça ativa no ar
	Grounded,       // peça apoiada, lock delay contando
	ClearingLines,  // animação de limpeza (sem peça ativa)
	GameOver
};

UENUM(BlueprintType)
enum class ENSGameEventType : uint8
{
	PieceSpawned,     // Piece, Cells, Ghost
	PieceMoved,       // Cells, Ghost, bFlag = movido pela gravidade/soft drop
	PieceRotated,     // Cells, Ghost, IntA = índice do kick usado
	PieceHardDropped, // Cells (posição final), IntA = FromY, IntB = ToY (Y da origem)
	PieceLocked,      // Piece, Cells
	LinesCleared,     // Rows, IntA = Count, IntB = Combo, IntC = ScoreGained
	BoardChanged,     // (sem payload) a grade fixa mudou: lock ou fim da limpeza
	ScoreChanged,     // Score, IntA = Lines, IntB = Level
	LevelUp,          // IntA = NewLevel
	HoldChanged,      // Piece = peça guardada, bFlag = bCanHold
	HoldDenied,       // (sem payload) hold usado de novo na mesma peça
	QueueChanged,     // Queue = próximas peças (PreviewCount)
	LockWarning,      // FloatA = tempo restante do lock delay
	GameOver          // Score, IntA = Lines, IntB = Level
};

/** Evento genérico emitido pela simulação. O significado dos campos depende de Type (ver comentários acima). */
USTRUCT(BlueprintType)
struct FNSGameEvent
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly) ENSGameEventType Type = ENSGameEventType::BoardChanged;
	UPROPERTY(BlueprintReadOnly) ENSPieceType Piece = ENSPieceType::None;
	UPROPERTY(BlueprintReadOnly) TArray<FIntPoint> Cells;
	UPROPERTY(BlueprintReadOnly) TArray<FIntPoint> Ghost;
	UPROPERTY(BlueprintReadOnly) TArray<int32> Rows;
	UPROPERTY(BlueprintReadOnly) TArray<ENSPieceType> Queue;
	UPROPERTY(BlueprintReadOnly) int64 Score = 0;
	UPROPERTY(BlueprintReadOnly) int32 IntA = 0;
	UPROPERTY(BlueprintReadOnly) int32 IntB = 0;
	UPROPERTY(BlueprintReadOnly) int32 IntC = 0;
	UPROPERTY(BlueprintReadOnly) float FloatA = 0.f;
	UPROPERTY(BlueprintReadOnly) bool bFlag = false;
};

/** Input acumulado entre dois ticks. Montado pelo ANSBoardActor. */
struct FNSInputFrame
{
	bool bLeftHeld = false;
	bool bRightHeld = false;
	bool bSoftDropHeld = false;

	/** Bordas de "pressionou", na ordem em que chegaram. Consumidas a cada tick. */
	TArray<ENSInputAction, TInlineAllocator<8>> Pressed;
};

/** Configuração copiada para dentro da simulação (montada a partir de UNSGameConfig). Valores = core-mechanics §11. */
struct FNSSimConfig
{
	int32 BoardWidth = 10;
	int32 VisibleHeight = 20;
	int32 HiddenRows = 2;
	int32 PreviewCount = 3;

	float DAS = 0.170f;
	float ARR = 0.050f;              // 0 = instantâneo até a parede
	float SoftDropFactor = 20.f;
	float SoftDropMaxInterval = 0.033f;

	float LockDelay = 0.5f;
	int32 MaxLockResets = 15;
	float LockWarningTime = 0.2f;
	bool bAllowUpKick = true;

	float LineClearDelay = 0.25f;

	int32 LineScore[5] = { 0, 100, 300, 500, 800 };
	int32 ComboBonus = 50;
	int32 SoftDropPointsPerCell = 1;
	int32 HardDropPointsPerCell = 2;

	int32 LinesPerLevel = 10;
	int32 StartLevel = 1;
	float GravityBase = 0.8f;
	float GravityLevelStep = 0.007f;
	float MinGravity = 0.05f;
	int32 MaxLevel = 0;              // 0 = sem teto

	int32 TotalHeight() const { return VisibleHeight + HiddenRows; }
};
```

---

## 2. `NSPieceTables.h/.cpp`

```cpp
#pragma once
#include "NSTypes.h"

/** Tabelas estáticas de formas (4 rotações pré-calculadas), spawn e kicks. */
class FNSPieceTables
{
public:
	/** 4 células relativas à origem (canto inferior esquerdo da caixa), Y para cima. Rotation 0..3 (0, R, 2, L). */
	static TConstArrayView<FIntPoint> GetCells(ENSPieceType Type, int32 Rotation);

	/** Tamanho N da caixa N×N (I = 4, O = 2, demais = 3). */
	static int32 GetBoxSize(ENSPieceType Type);

	/** Origem de spawn: a linha mais alta ocupada no estado 0 fica em Y = TotalHeight - 1. */
	static FIntPoint GetSpawnOrigin(ENSPieceType Type, int32 TotalHeight);

	/** Deslocamentos a testar na rotação, em ordem (core-mechanics §4). */
	static void GetKicks(ENSPieceType Type, bool bAllowUpKick, TArray<FIntPoint, TInlineAllocator<6>>& OutKicks);

	static bool CanRotate(ENSPieceType Type) { return Type != ENSPieceType::O; }

private:
	static void BuildIfNeeded();
};
```

### Implementação

Estado 0 em coordenadas da caixa (Y para cima; a linha de cima da caixa é `cy = N-1`):

| Peça | N | Células (cx, cy) | Coluna de spawn |
|------|---|------------------|-----------------|
| I | 4 | (0,2) (1,2) (2,2) (3,2) | 3 |
| O | 2 | (0,0) (1,0) (0,1) (1,1) | 4 |
| T | 3 | (1,2) (0,1) (1,1) (2,1) | 3 |
| S | 3 | (1,2) (2,2) (0,1) (1,1) | 3 |
| Z | 3 | (0,2) (1,2) (1,1) (2,1) | 3 |
| J | 3 | (0,2) (0,1) (1,1) (2,1) | 3 |
| L | 3 | (2,2) (0,1) (1,1) (2,1) | 3 |

```cpp
// NSPieceTables.cpp (trechos)
namespace NSPieceTablesPrivate
{
	static FIntPoint GTable[NSPieceTypeCount][4][4]; // [tipo-1][rotação][célula]
	static bool bBuilt = false;

	static const FIntPoint GState0[NSPieceTypeCount][4] = {
		{ {0,2},{1,2},{2,2},{3,2} }, // I
		{ {0,0},{1,0},{0,1},{1,1} }, // O
		{ {1,2},{0,1},{1,1},{2,1} }, // T
		{ {1,2},{2,2},{0,1},{1,1} }, // S
		{ {0,2},{1,2},{1,1},{2,1} }, // Z
		{ {0,2},{0,1},{1,1},{2,1} }, // J
		{ {2,2},{0,1},{1,1},{2,1} }, // L
	};
}

void FNSPieceTables::BuildIfNeeded()
{
	using namespace NSPieceTablesPrivate;
	if (bBuilt) return;
	for (int32 T = 0; T < NSPieceTypeCount; ++T)
	{
		const int32 N = GetBoxSize(static_cast<ENSPieceType>(T + 1));
		for (int32 C = 0; C < 4; ++C) GTable[T][0][C] = GState0[T][C];
		for (int32 R = 1; R < 4; ++R)
			for (int32 C = 0; C < 4; ++C)
			{
				const FIntPoint P = GTable[T][R - 1][C];
				GTable[T][R][C] = FIntPoint(P.Y, N - 1 - P.X); // rotação horária
			}
	}
	bBuilt = true;
}

TConstArrayView<FIntPoint> FNSPieceTables::GetCells(ENSPieceType Type, int32 Rotation)
{
	BuildIfNeeded();
	check(Type != ENSPieceType::None);
	return TConstArrayView<FIntPoint>(NSPieceTablesPrivate::GTable[(int32)Type - 1][Rotation & 3], 4);
}

FIntPoint FNSPieceTables::GetSpawnOrigin(ENSPieceType Type, int32 TotalHeight)
{
	int32 MaxCy = 0;
	for (const FIntPoint& P : GetCells(Type, 0)) MaxCy = FMath::Max(MaxCy, P.Y);
	const int32 Col = (Type == ENSPieceType::O) ? 4 : 3;
	return FIntPoint(Col, (TotalHeight - 1) - MaxCy);   // I/T/S/Z/J/L → Y=19, O → Y=20
}

void FNSPieceTables::GetKicks(ENSPieceType Type, bool bAllowUpKick, TArray<FIntPoint, TInlineAllocator<6>>& Out)
{
	Out.Reset();
	Out.Add({0,0}); Out.Add({-1,0}); Out.Add({1,0});
	if (Type == ENSPieceType::I) { Out.Add({-2,0}); Out.Add({2,0}); }
	if (bAllowUpKick) Out.Add({0,1});
}
```

- Anti-horário = rotação `(R + 3) & 3` (usa a mesma tabela).
- `BuildIfNeeded` não é thread-safe; a simulação roda só na game thread. Opcional: chamar uma vez no `StartupModule`.

---

## 3. `FNSActivePiece` (em `NSBoardGrid.h`)

```cpp
struct FNSActivePiece
{
	ENSPieceType Type = ENSPieceType::None;
	int32 Rotation = 0;
	FIntPoint Origin = FIntPoint::ZeroValue;

	bool IsValid() const { return Type != ENSPieceType::None; }

	/** Células absolutas na grade. */
	void GetCells(FIntPoint OutCells[4]) const
	{
		const TConstArrayView<FIntPoint> Rel = FNSPieceTables::GetCells(Type, Rotation);
		for (int32 i = 0; i < 4; ++i) OutCells[i] = Origin + Rel[i];
	}

	FNSActivePiece Moved(int32 Dx, int32 Dy) const { FNSActivePiece P = *this; P.Origin += FIntPoint(Dx, Dy); return P; }
	FNSActivePiece Rotated(int32 Dir) const { FNSActivePiece P = *this; P.Rotation = (Rotation + (Dir > 0 ? 1 : 3)) & 3; return P; }
};
```

---

## 4. `FNSBoardGrid` (`NSBoardGrid.h/.cpp`)

```cpp
class FNSBoardGrid
{
public:
	void Init(int32 InWidth, int32 InHeight);   // zera tudo
	int32 GetWidth() const  { return Width; }
	int32 GetHeight() const { return Height; }

	/** Valor da célula (0 = vazia, 1..7 = ENSPieceType). Fora da grade retorna 0. */
	uint8 Get(int32 X, int32 Y) const;

	/** Regra de colisão (core-mechanics §4): X fora de [0,W) ou Y < 0 → ocupado; Y >= H → livre. */
	bool IsFree(int32 X, int32 Y) const;

	bool CanPlace(const FNSActivePiece& Piece) const;

	/** Escreve as 4 células (ignora as que estiverem com Y >= Height). Retorna as linhas distintas tocadas. */
	void Lock(const FNSActivePiece& Piece, TArray<int32>& OutTouchedRows);

	/** Das linhas candidatas, retorna as completas em ordem crescente. */
	void FindFullRows(const TArray<int32>& CandidateRows, TArray<int32>& OutFullRows) const;

	/** Remove as linhas indicadas e desce as de cima (gravidade "ingênua"). */
	void ClearRows(const TArray<int32>& Rows);

	/** Posição de pouso do hard drop. */
	FNSActivePiece GetDropPosition(const FNSActivePiece& Piece) const;

private:
	int32 Width = 0;
	int32 Height = 0;
	TArray<uint8> Cells;   // índice = Y * Width + X
};
```

### Pontos de implementação
```cpp
void FNSBoardGrid::ClearRows(const TArray<int32>& Rows)
{
	if (Rows.Num() == 0) return;
	int32 Write = 0;
	for (int32 Read = 0; Read < Height; ++Read)
	{
		if (Rows.Contains(Read)) continue;                 // linha removida
		if (Write != Read)
			FMemory::Memcpy(&Cells[Write * Width], &Cells[Read * Width], Width);
		++Write;
	}
	for (; Write < Height; ++Write)                         // novas linhas vazias no topo
		FMemory::Memzero(&Cells[Write * Width], Width);
}

FNSActivePiece FNSBoardGrid::GetDropPosition(const FNSActivePiece& Piece) const
{
	FNSActivePiece P = Piece;
	while (CanPlace(P.Moved(0, -1))) P = P.Moved(0, -1);
	return P;
}
```
- Teste T5 (linhas 0 e 2 cheias, 1 não) é coberto naturalmente por esse `ClearRows`.

---

## 5. `FNSRandomizer` (`NSRandomizer.h/.cpp`): M2

```cpp
class FNSRandomizer
{
public:
	/** Seed 0 = aleatória (FMath::Rand). Enche a fila com 2 sacos. */
	void Reset(int32 Seed, int32 InMinQueue);
	ENSPieceType Pop();                                        // retira a próxima e reabastece
	void Peek(int32 Count, TArray<ENSPieceType>& Out) const;   // sem remover

	/** Só para testes: substitui o início da fila. */
	void Test_ForceQueue(const TArray<ENSPieceType>& Pieces);

private:
	void AppendBag();
	FRandomStream Stream;
	TArray<ENSPieceType> Queue;
	int32 MinQueue = 4;  // PreviewCount + 1
};

void FNSRandomizer::AppendBag()
{
	ENSPieceType Bag[7] = { ENSPieceType::I, ENSPieceType::O, ENSPieceType::T, ENSPieceType::S,
	                        ENSPieceType::Z, ENSPieceType::J, ENSPieceType::L };
	for (int32 i = 6; i > 0; --i) Swap(Bag[i], Bag[Stream.RandRange(0, i)]);   // Fisher-Yates
	Queue.Append(Bag, 7);
}

ENSPieceType FNSRandomizer::Pop()
{
	while (Queue.Num() <= MinQueue) AppendBag();
	const ENSPieceType Next = Queue[0];   // fila FIFO: retira do INÍCIO
	Queue.RemoveAt(0);                     // custo desprezível com ~14 itens
	return Next;
}
```
> ⚠️ Não usar `TArray::Pop()`: ele retira do **fim** do array e quebraria a ordem do preview.

---

## 6. `FNSScoring` (`NSScoring.h/.cpp`): M1 (gravidade) + M3 (pontuação)

```cpp
struct FNSScoring
{
	int64 Score = 0;
	int32 Lines = 0;
	int32 Level = 1;
	int32 Combo = -1;

	void Reset(const FNSSimConfig& Cfg) { Score = 0; Lines = 0; Level = Cfg.StartLevel; Combo = -1; }

	struct FLockResult { int32 Gained = 0; bool bLevelUp = false; };

	/** core-mechanics §8.2. LinesCleared 0..4. Usa o nível ANTES do level up. */
	FLockResult ApplyLock(int32 LinesCleared, const FNSSimConfig& Cfg);

	void AddDropPoints(int32 Cells, bool bHardDrop, const FNSSimConfig& Cfg)
	{
		Score += (int64)Cells * (bHardDrop ? Cfg.HardDropPointsPerCell : Cfg.SoftDropPointsPerCell);
	}

	/** progression-system §3: max(Min, (Base - (n-1)*Step)^(n-1)). */
	float GetGravityInterval(const FNSSimConfig& Cfg) const
	{
		const int32 N = Level - 1;
		const float Base = FMath::Max(0.f, Cfg.GravityBase - N * Cfg.GravityLevelStep);
		return FMath::Max(Cfg.MinGravity, FMath::Pow(Base, (float)N));
	}
};

FNSScoring::FLockResult FNSScoring::ApplyLock(int32 K, const FNSSimConfig& Cfg)
{
	FLockResult R;
	if (K <= 0) { Combo = -1; return R; }

	const int32 LevelForScore = Level;
	++Combo;
	R.Gained = Cfg.LineScore[FMath::Clamp(K, 0, 4)] * LevelForScore;
	if (Combo >= 1) R.Gained += Cfg.ComboBonus * Combo * LevelForScore;
	Score += R.Gained;
	Lines += K;

	int32 NewLevel = Cfg.StartLevel + Lines / Cfg.LinesPerLevel;
	if (Cfg.MaxLevel > 0) NewLevel = FMath::Min(NewLevel, Cfg.MaxLevel);
	R.bLevelUp = NewLevel > Level;
	Level = NewLevel;
	return R;
}
```
> Nota: com `StartLevel` fixo em 1 a fórmula `Start + Lines/10` coincide com o GDD. `FMath::Pow(0, 0)` = 1 → nível 1 = 1,0 s ✔.

---

## 7. `FNSGameSimulation` (`NSGameSimulation.h/.cpp`)

### 7.1 Header

```cpp
#pragma once
#include "NSTypes.h"
#include "NSBoardGrid.h"
#include "NSRandomizer.h"
#include "NSScoring.h"

class FNSGameSimulation
{
public:
	void Configure(const FNSSimConfig& InConfig) { Config = InConfig; }
	void SetHandling(float InDAS, float InARR) { Config.DAS = InDAS; Config.ARR = InARR; }

	/** Zera tudo e faz o primeiro spawn. */
	void Start(int32 Seed, TArray<FNSGameEvent>& OutEvents);

	/** Avança Dt segundos (já limitado pelo chamador). */
	void Tick(float Dt, const FNSInputFrame& Input, TArray<FNSGameEvent>& OutEvents);

	// --- Leitura (para a view e o HUD) ---
	ENSSimState GetState() const { return State; }
	const FNSBoardGrid& GetBoard() const { return Board; }
	const FNSActivePiece& GetActive() const { return Active; }  // inválida em ClearingLines/GameOver
	FNSActivePiece GetGhost() const { return Board.GetDropPosition(Active); }
	ENSPieceType GetHoldPiece() const { return HoldPiece; }
	bool CanHold() const { return bCanHold; }
	void GetQueue(TArray<ENSPieceType>& Out) const { Randomizer.Peek(Config.PreviewCount, Out); }
	const FNSScoring& GetScoring() const { return Scoring; }
	uint32 GetBoardRevision() const { return BoardRevision; }   // muda quando a grade fixa muda
	const FNSSimConfig& GetConfig() const { return Config; }

	// --- Só para testes ---
	FNSBoardGrid& Test_GetBoard() { return Board; }
	void Test_ForceQueue(const TArray<ENSPieceType>& Pieces) { Randomizer.Test_ForceQueue(Pieces); }
	void Test_SetActive(const FNSActivePiece& P) { Active = P; State = ENSSimState::Falling; }

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

	// Sub-etapas do Tick
	void ProcessPressed(const FNSInputFrame& Input, TArray<FNSGameEvent>& Out);
	void UpdateAutoShift(float Dt, const FNSInputFrame& Input, TArray<FNSGameEvent>& Out);
	void UpdateGravity(float Dt, const FNSInputFrame& Input, TArray<FNSGameEvent>& Out);
	void UpdateLock(float Dt, TArray<FNSGameEvent>& Out);
	void OnSuccessfulManipulation();     // lock reset

	bool IsPieceControllable() const { return State == ENSSimState::Falling || State == ENSSimState::Grounded; }
	bool IsGrounded() const { return !Board.CanPlace(Active.Moved(0, -1)); }

	// Helpers de evento
	void EmitPieceEvent(ENSGameEventType Type, TArray<FNSGameEvent>& Out, int32 IntA = 0, bool bFlag = false);
	void EmitScore(TArray<FNSGameEvent>& Out);
	void EmitQueue(TArray<FNSGameEvent>& Out);

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

	int8 ShiftDir = 0;          // -1, 0, +1
	float DasTimer = 0.f;
	float ArrTimer = 0.f;
	bool bDasCharged = false;

	uint32 BoardRevision = 0;
};
```

### 7.2 `Tick`: ordem obrigatória

```cpp
void FNSGameSimulation::Tick(float Dt, const FNSInputFrame& Input, TArray<FNSGameEvent>& Out)
{
	if (State == ENSSimState::Ready || State == ENSSimState::GameOver) return;

	ProcessPressed(Input, Out);          // 1. bordas de input, em ordem (input ANTES da gravidade)
	UpdateAutoShift(Dt, Input, Out);     // 2. DAS/ARR (o timer carrega mesmo em ClearingLines)

	if (State == ENSSimState::ClearingLines)
	{
		ClearTimer -= Dt;                // 3. animação de limpeza
		if (ClearTimer <= 0.f) FinishLineClear(Out);
		return;
	}
	if (!IsPieceControllable()) return;  // pode ter virado GameOver em ProcessPressed

	UpdateGravity(Dt, Input, Out);       // 4. gravidade / soft drop
	UpdateLock(Dt, Out);                 // 5. grounded + lock delay
}
```

### 7.3 `ProcessPressed`
```cpp
for (ENSInputAction A : Input.Pressed)
{
	switch (A)
	{
	case ENSInputAction::MoveLeft:  ShiftDir = -1; DasTimer = ArrTimer = 0; bDasCharged = false;
	                                if (IsPieceControllable()) TryMove(-1, Out); break;
	case ENSInputAction::MoveRight: ShiftDir = +1; DasTimer = ArrTimer = 0; bDasCharged = false;
	                                if (IsPieceControllable()) TryMove(+1, Out); break;
	case ENSInputAction::RotateCW:  if (IsPieceControllable()) TryRotate(+1, Out); break;
	case ENSInputAction::RotateCCW: if (IsPieceControllable()) TryRotate(-1, Out); break;
	case ENSInputAction::HardDrop:  if (IsPieceControllable()) DoHardDrop(Out); break;
	case ENSInputAction::Hold:      if (IsPieceControllable()) DoHold(Out); break;
	case ENSInputAction::SoftDrop:  break; // tratado pelo estado "held"
	}
}
```

### 7.4 `UpdateAutoShift` (DAS/ARR, core-mechanics §5)
```cpp
// 1. Resolver a direção ativa (última pressionada; soltou → volta para a outra, com DAS reiniciado)
const bool bHeldActive = (ShiftDir < 0 && Input.bLeftHeld) || (ShiftDir > 0 && Input.bRightHeld);
if (!bHeldActive)
{
	const int8 Other = Input.bLeftHeld ? -1 : (Input.bRightHeld ? +1 : 0);
	if (Other != ShiftDir) { ShiftDir = Other; DasTimer = ArrTimer = 0; bDasCharged = false; }
}
if (ShiftDir == 0) return;

// 2. Carregar o DAS
DasTimer += Dt;
if (!bDasCharged)
{
	if (DasTimer < Config.DAS) return;
	bDasCharged = true;
	ArrTimer = DasTimer - Config.DAS;          // sobra do frame
	if (IsPieceControllable()) TryMove(ShiftDir, Out);   // 1º passo automático
}
else
{
	ArrTimer += Dt;
}
if (!IsPieceControllable()) return;           // em ClearingLines só carrega

// 3. Repetição
if (Config.ARR <= 0.f) { while (TryMove(ShiftDir, Out)) {} return; }
while (ArrTimer >= Config.ARR)
{
	ArrTimer -= Config.ARR;
	if (!TryMove(ShiftDir, Out)) { ArrTimer = 0.f; break; }
}
```

### 7.5 `UpdateGravity`
```cpp
if (IsGrounded()) { GravityTimer = 0.f; return; }

const float G = Scoring.GetGravityInterval(Config);
const bool bSoft = Input.bSoftDropHeld;
const float Interval = bSoft ? FMath::Min(G / Config.SoftDropFactor, Config.SoftDropMaxInterval) : G;

GravityTimer += Dt;
int32 Fallen = 0;
while (GravityTimer >= Interval)
{
	if (!Board.CanPlace(Active.Moved(0, -1))) { GravityTimer = 0.f; break; }
	Active = Active.Moved(0, -1);
	GravityTimer -= Interval;
	++Fallen;
}
if (Fallen > 0)
{
	if (bSoft) { Scoring.AddDropPoints(Fallen, false, Config); EmitScore(Out); }
	EmitPieceEvent(ENSGameEventType::PieceMoved, Out, 0, /*bFlag byGravity*/ true);
}
```
> Um único `PieceMoved` por tick, mesmo se caiu várias linhas (em alta velocidade).

### 7.6 `UpdateLock` (core-mechanics §6.4)
```cpp
if (!IsGrounded())
{
	if (State == ENSSimState::Grounded) { State = ENSSimState::Falling; LockTimer = 0.f; bLockWarned = false; }
	return;
}
if (State == ENSSimState::Falling) { State = ENSSimState::Grounded; LockTimer = 0.f; bLockWarned = false; }

LockTimer += Dt;
const float Remaining = Config.LockDelay - LockTimer;
if (!bLockWarned && Remaining <= Config.LockWarningTime)
{
	bLockWarned = true;
	FNSGameEvent E; E.Type = ENSGameEventType::LockWarning; E.FloatA = FMath::Max(0.f, Remaining); Out.Add(E);
}
if (LockTimer >= Config.LockDelay) LockPiece(Out);
```

### 7.7 `TryMove` / `TryRotate` / lock reset
```cpp
bool FNSGameSimulation::TryMove(int32 Dx, TArray<FNSGameEvent>& Out)
{
	const FNSActivePiece Next = Active.Moved(Dx, 0);
	if (!Board.CanPlace(Next)) return false;
	Active = Next;
	OnSuccessfulManipulation();
	EmitPieceEvent(ENSGameEventType::PieceMoved, Out, 0, false);
	return true;
}

bool FNSGameSimulation::TryRotate(int32 Dir, TArray<FNSGameEvent>& Out)
{
	if (!FNSPieceTables::CanRotate(Active.Type)) return false;
	TArray<FIntPoint, TInlineAllocator<6>> Kicks;
	FNSPieceTables::GetKicks(Active.Type, Config.bAllowUpKick, Kicks);
	const FNSActivePiece Rot = Active.Rotated(Dir);
	for (int32 i = 0; i < Kicks.Num(); ++i)
	{
		const FNSActivePiece Cand = Rot.Moved(Kicks[i].X, Kicks[i].Y);
		if (Board.CanPlace(Cand))
		{
			Active = Cand;
			OnSuccessfulManipulation();
			EmitPieceEvent(ENSGameEventType::PieceRotated, Out, i);
			return true;
		}
	}
	return false;
}

void FNSGameSimulation::OnSuccessfulManipulation()
{
	if (State == ENSSimState::Grounded && LockResets < Config.MaxLockResets)
	{
		LockTimer = 0.f; ++LockResets; bLockWarned = false;
	}
}
```

### 7.8 Hard drop, lock, limpeza e spawn
```cpp
void FNSGameSimulation::DoHardDrop(TArray<FNSGameEvent>& Out)
{
	const int32 FromY = Active.Origin.Y;
	Active = Board.GetDropPosition(Active);
	const int32 Dist = FromY - Active.Origin.Y;
	Scoring.AddDropPoints(Dist, true, Config);
	EmitPieceEvent(ENSGameEventType::PieceHardDropped, Out, FromY);   // IntB = ToY preenchido no helper
	Out.Last().IntB = Active.Origin.Y;
	EmitScore(Out);
	LockPiece(Out);
}

void FNSGameSimulation::LockPiece(TArray<FNSGameEvent>& Out)
{
	FIntPoint Cells[4]; Active.GetCells(Cells);

	// Lock out: todas as células na zona oculta
	bool bAllHidden = true;
	for (const FIntPoint& C : Cells) bAllHidden &= (C.Y >= Config.VisibleHeight);

	// Emitir ANTES de gravar na grade (o ghost seria calculado errado depois)
	EmitPieceEvent(ENSGameEventType::PieceLocked, Out);

	TArray<int32> Touched;
	Board.Lock(Active, Touched);
	++BoardRevision;
	{ FNSGameEvent B; B.Type = ENSGameEventType::BoardChanged; Out.Add(B); }

	const ENSPieceType LockedType = Active.Type;
	Active = FNSActivePiece();     // sem peça ativa até o próximo spawn
	bCanHold = true;
	{ FNSGameEvent H; H.Type = ENSGameEventType::HoldChanged; H.Piece = HoldPiece; H.bFlag = true; Out.Add(H); }

	if (bAllHidden) { EnterGameOver(Out); return; }

	TArray<int32> Full;
	Board.FindFullRows(Touched, Full);
	const FNSScoring::FLockResult R = Scoring.ApplyLock(Full.Num(), Config);

	if (Full.Num() > 0)
	{
		FNSGameEvent E; E.Type = ENSGameEventType::LinesCleared;
		E.Rows = Full; E.IntA = Full.Num(); E.IntB = Scoring.Combo; E.IntC = R.Gained;
		Out.Add(E);
		EmitScore(Out);
		if (R.bLevelUp) { FNSGameEvent L; L.Type = ENSGameEventType::LevelUp; L.IntA = Scoring.Level; Out.Add(L); }

		PendingClearRows = Full;
		ClearTimer = Config.LineClearDelay;
		State = ENSSimState::ClearingLines;
		return;
	}
	SpawnNextFromQueue(Out);
}

void FNSGameSimulation::FinishLineClear(TArray<FNSGameEvent>& Out)
{
	Board.ClearRows(PendingClearRows);
	PendingClearRows.Reset();
	++BoardRevision;
	FNSGameEvent B; B.Type = ENSGameEventType::BoardChanged; Out.Add(B);
	SpawnNextFromQueue(Out);
}

void FNSGameSimulation::SpawnPiece(ENSPieceType Type, TArray<FNSGameEvent>& Out)
{
	Active.Type = Type;
	Active.Rotation = 0;
	Active.Origin = FNSPieceTables::GetSpawnOrigin(Type, Config.TotalHeight());

	if (!Board.CanPlace(Active)) { EnterGameOver(Out); return; }    // Block out
	if (Board.CanPlace(Active.Moved(0, -1))) Active = Active.Moved(0, -1);  // desce 1 ao nascer

	State = ENSSimState::Falling;
	GravityTimer = 0.f; LockTimer = 0.f; LockResets = 0; bLockWarned = false;
	EmitPieceEvent(ENSGameEventType::PieceSpawned, Out);
}

void FNSGameSimulation::SpawnNextFromQueue(TArray<FNSGameEvent>& Out)
{
	SpawnPiece(Randomizer.Pop(), Out);
	EmitQueue(Out);
}
```

### 7.9 Hold
```cpp
void FNSGameSimulation::DoHold(TArray<FNSGameEvent>& Out)
{
	if (!bCanHold) { FNSGameEvent E; E.Type = ENSGameEventType::HoldDenied; Out.Add(E); return; }

	const ENSPieceType Current = Active.Type;
	bCanHold = false;
	if (HoldPiece == ENSPieceType::None)
	{
		HoldPiece = Current;
		SpawnNextFromQueue(Out);
	}
	else
	{
		const ENSPieceType FromHold = HoldPiece;
		HoldPiece = Current;
		SpawnPiece(FromHold, Out);
	}
	FNSGameEvent H; H.Type = ENSGameEventType::HoldChanged; H.Piece = HoldPiece; H.bFlag = false; Out.Add(H);
}
```

### 7.10 `Start`, `EnterGameOver` e helpers
```cpp
void FNSGameSimulation::Start(int32 Seed, TArray<FNSGameEvent>& Out)
{
	Board.Init(Config.BoardWidth, Config.TotalHeight());
	Randomizer.Reset(Seed, Config.PreviewCount + 1);
	Scoring.Reset(Config);
	HoldPiece = ENSPieceType::None; bCanHold = true;
	ShiftDir = 0; DasTimer = ArrTimer = 0.f; bDasCharged = false;
	PendingClearRows.Reset(); ++BoardRevision;

	FNSGameEvent B; B.Type = ENSGameEventType::BoardChanged; Out.Add(B);
	EmitScore(Out);
	FNSGameEvent H; H.Type = ENSGameEventType::HoldChanged; H.Piece = HoldPiece; H.bFlag = true; Out.Add(H);
	SpawnNextFromQueue(Out);
}

void FNSGameSimulation::EnterGameOver(TArray<FNSGameEvent>& Out)
{
	State = ENSSimState::GameOver;
	FNSGameEvent E; E.Type = ENSGameEventType::GameOver;
	E.Score = Scoring.Score; E.IntA = Scoring.Lines; E.IntB = Scoring.Level;
	Out.Add(E);
}

void FNSGameSimulation::EmitPieceEvent(ENSGameEventType Type, TArray<FNSGameEvent>& Out, int32 IntA, bool bFlag)
{
	FNSGameEvent E; E.Type = Type; E.Piece = Active.Type; E.IntA = IntA; E.bFlag = bFlag;
	FIntPoint C[4]; Active.GetCells(C); E.Cells.Append(C, 4);
	FIntPoint G[4]; GetGhost().GetCells(G); E.Ghost.Append(G, 4);
	Out.Add(MoveTemp(E));
}
// EmitScore: Type=ScoreChanged, Score, IntA=Lines, IntB=Level
// EmitQueue: Type=QueueChanged, Queue = Peek(PreviewCount)
```

---

## 8. Testes (`Source/Tetris/Tests/NSSimulationTests.cpp`)

```cpp
#include "Misc/AutomationTest.h"
#include "Simulation/NSGameSimulation.h"

#if WITH_DEV_AUTOMATION_TESTS

namespace NSTest
{
	/** Avança a simulação em passos fixos de 1/60 s. */
	static void Advance(FNSGameSimulation& Sim, float Seconds, const FNSInputFrame& Held = FNSInputFrame())
	{
		TArray<FNSGameEvent> Ev;
		for (float T = 0; T < Seconds; T += 1.f / 60.f) { Ev.Reset(); Sim.Tick(1.f / 60.f, Held, Ev); }
	}
	static void Press(FNSGameSimulation& Sim, ENSInputAction A, TArray<FNSGameEvent>& Ev)
	{
		FNSInputFrame F; F.Pressed.Add(A); Sim.Tick(0.f, F, Ev);
	}
	static void FillRow(FNSBoardGrid& B, int32 Y, int32 HoleX = -1)
	{
		/* escreve 1 em todas as colunas exceto HoleX, via Lock de peças O ou método de teste */
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FNSTest_T4_Quad, "NeonStack.Simulation.T04_Quad",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)
bool FNSTest_T4_Quad::RunTest(const FString&)
{
	FNSGameSimulation Sim; Sim.Configure(FNSSimConfig());
	TArray<FNSGameEvent> Ev; Sim.Start(1234, Ev);
	// Preparar 4 linhas cheias com buraco na coluna 0, colocar uma I vertical na coluna 0 (Test_SetActive), hard drop
	// ...
	const int64 Expected = 800 /* Quad nível 1 */ + 2 * DropDistance /* hard drop */;
	TestEqual(TEXT("Score do Quad no nível 1"), Sim.GetScoring().Score, Expected);
	TestEqual(TEXT("Linhas"), Sim.GetScoring().Lines, 4);
	return true;
}

#endif
```

- Precisa de um helper de teste no `FNSBoardGrid`: `void Test_SetCell(int32 X, int32 Y, uint8 V)`.
- Um teste por caso T1-T12 de core-mechanics §13. Nome: `NeonStack.Simulation.Txx_Descricao`.
- Rodar: **Tools → Session Frontend → Automation**, filtrar por `NeonStack`.
- A flag `EAutomationTestFlags_ApplicationContextMask` é a forma atual (UE 5.5+). Se o compilador reclamar, usar `EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter`.

---

## 9. Checklist de aceite (camada de simulação)

- [ ] Compila sem incluir nada de `Engine/` além de `CoreMinimal` (exceto `NSTypes.generated.h`)
- [ ] T1-T12 passam
- [ ] Mesma seed + mesmo input = mesmos eventos (rodar 2× e comparar)
- [ ] Nenhuma alocação por frame em regime normal (os `TArray` de eventos são reutilizados pelo chamador)

---

*Spec por Priscilla, GD-KS UE5 Programmer Lead.*
