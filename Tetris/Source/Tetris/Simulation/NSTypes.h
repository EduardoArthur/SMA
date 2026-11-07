// Neon Stack - tipos compartilhados da simulação.
// Spec: _gdks-output/04-engine/cpp-specs/01-simulation.md §1

#pragma once

#include "CoreMinimal.h"
#include "NSTypes.generated.h"

/** Tipo de peça. O valor numérico (1..7) é gravado nas células da grade; 0 = vazio. */
UENUM(BlueprintType)
enum class ENSPieceType : uint8
{
	None = 0,
	I,
	O,
	T,
	S,
	Z,
	J,
	L
};

constexpr int32 NSPieceTypeCount = 7;

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
	Ready,
	Falling,
	Grounded,
	ClearingLines,
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
	BoardChanged,     // a grade fixa mudou (lock ou fim da limpeza)
	ScoreChanged,     // Score, IntA = Lines, IntB = Level
	LevelUp,          // IntA = NewLevel
	HoldChanged,      // Piece = peça guardada, bFlag = bCanHold
	HoldDenied,       // hold usado de novo na mesma peça
	QueueChanged,     // Queue = próximas peças
	LockWarning,      // FloatA = tempo restante do lock delay
	GameOver          // Score, IntA = Lines, IntB = Level
};

/** Evento genérico emitido pela simulação. O significado dos campos depende de Type. */
USTRUCT(BlueprintType)
struct FNSGameEvent
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "NeonStack")
	ENSGameEventType Type = ENSGameEventType::BoardChanged;

	UPROPERTY(BlueprintReadOnly, Category = "NeonStack")
	ENSPieceType Piece = ENSPieceType::None;

	UPROPERTY(BlueprintReadOnly, Category = "NeonStack")
	TArray<FIntPoint> Cells;

	UPROPERTY(BlueprintReadOnly, Category = "NeonStack")
	TArray<FIntPoint> Ghost;

	UPROPERTY(BlueprintReadOnly, Category = "NeonStack")
	TArray<int32> Rows;

	UPROPERTY(BlueprintReadOnly, Category = "NeonStack")
	TArray<ENSPieceType> Queue;

	UPROPERTY(BlueprintReadOnly, Category = "NeonStack")
	int64 Score = 0;

	UPROPERTY(BlueprintReadOnly, Category = "NeonStack")
	int32 IntA = 0;

	UPROPERTY(BlueprintReadOnly, Category = "NeonStack")
	int32 IntB = 0;

	UPROPERTY(BlueprintReadOnly, Category = "NeonStack")
	int32 IntC = 0;

	UPROPERTY(BlueprintReadOnly, Category = "NeonStack")
	float FloatA = 0.f;

	UPROPERTY(BlueprintReadOnly, Category = "NeonStack")
	bool bFlag = false;
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

/** Configuração copiada para dentro da simulação (montada a partir de UNSGameConfig). */
struct FNSSimConfig
{
	int32 BoardWidth = 10;
	int32 VisibleHeight = 20;
	int32 HiddenRows = 2;
	int32 PreviewCount = 3;

	float DAS = 0.170f;
	float ARR = 0.050f;               // 0 = instantâneo até a parede
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
	int32 MaxLevel = 0;               // 0 = sem teto

	int32 TotalHeight() const { return VisibleHeight + HiddenRows; }
};
