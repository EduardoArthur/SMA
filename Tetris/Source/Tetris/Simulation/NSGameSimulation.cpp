#include "Simulation/NSGameSimulation.h"

// ---------------------------------------------------------------------------
// Ciclo principal
// ---------------------------------------------------------------------------

void FNSGameSimulation::Start(int32 Seed, TArray<FNSGameEvent>& Out)
{
	Board.Init(Config.BoardWidth, Config.TotalHeight());
	Randomizer.Reset(Seed, Config.PreviewCount + 1);
	Scoring.Reset(Config);
	Active = FNSActivePiece();
	HoldPiece = ENSPieceType::None;
	bCanHold = true;
	ResetShift();
	PendingClearRows.Reset();
	ClearTimer = 0.f;
	++BoardRevision;

	EmitSimple(ENSGameEventType::BoardChanged, Out);
	EmitScore(Out);
	EmitHold(Out);
	SpawnNextFromQueue(Out);
}

void FNSGameSimulation::Tick(float Dt, const FNSInputFrame& Input, TArray<FNSGameEvent>& Out)
{
	if (State == ENSSimState::Ready || State == ENSSimState::GameOver)
	{
		return;
	}

	ProcessPressed(Input, Out);      // 1. bordas de input (antes da gravidade)
	UpdateAutoShift(Dt, Input, Out); // 2. DAS/ARR (carrega mesmo durante a limpeza)

	if (State == ENSSimState::ClearingLines)
	{
		ClearTimer -= Dt;            // 3. animação de limpeza
		if (ClearTimer <= 0.f)
		{
			FinishLineClear(Out);
		}
		return;
	}
	if (!IsPieceControllable())
	{
		return;                      // pode ter virado GameOver
	}

	UpdateGravity(Dt, Input, Out);   // 4. gravidade / soft drop
	UpdateLock(Dt, Out);             // 5. apoio + lock delay
}

void FNSGameSimulation::Test_SetActive(const FNSActivePiece& Piece)
{
	Active = Piece;
	State = ENSSimState::Falling;
	GravityTimer = 0.f;
	LockTimer = 0.f;
	LockResets = 0;
	bLockWarned = false;
}

// ---------------------------------------------------------------------------
// Etapas do Tick
// ---------------------------------------------------------------------------

void FNSGameSimulation::ProcessPressed(const FNSInputFrame& Input, TArray<FNSGameEvent>& Out)
{
	for (const ENSInputAction Action : Input.Pressed)
	{
		switch (Action)
		{
		case ENSInputAction::MoveLeft:
			ResetShift();
			ShiftDir = -1;
			if (IsPieceControllable()) { TryMove(-1, Out); }
			break;
		case ENSInputAction::MoveRight:
			ResetShift();
			ShiftDir = 1;
			if (IsPieceControllable()) { TryMove(1, Out); }
			break;
		case ENSInputAction::RotateCW:
			if (IsPieceControllable()) { TryRotate(1, Out); }
			break;
		case ENSInputAction::RotateCCW:
			if (IsPieceControllable()) { TryRotate(-1, Out); }
			break;
		case ENSInputAction::HardDrop:
			if (IsPieceControllable()) { DoHardDrop(Out); }
			break;
		case ENSInputAction::Hold:
			if (IsPieceControllable()) { DoHold(Out); }
			break;
		case ENSInputAction::SoftDrop:
		default:
			break; // soft drop é tratado pelo estado "segurando"
		}
	}
}

void FNSGameSimulation::UpdateAutoShift(float Dt, const FNSInputFrame& Input, TArray<FNSGameEvent>& Out)
{
	// 1. Direção ativa: última pressionada; ao soltar, volta para a outra (com DAS reiniciado)
	const bool bActiveHeld = (ShiftDir < 0 && Input.bLeftHeld) || (ShiftDir > 0 && Input.bRightHeld);
	if (!bActiveHeld)
	{
		const int32 Other = Input.bLeftHeld ? -1 : (Input.bRightHeld ? 1 : 0);
		if (Other != ShiftDir)
		{
			ResetShift();
			ShiftDir = Other;
		}
	}
	if (ShiftDir == 0)
	{
		return;
	}

	// 2. Carregar o DAS
	DasTimer += Dt;
	if (!bDasCharged)
	{
		if (DasTimer < Config.DAS)
		{
			return;
		}
		bDasCharged = true;
		ArrTimer = DasTimer - Config.DAS; // sobra do frame
		if (IsPieceControllable())
		{
			TryMove(ShiftDir, Out);       // primeiro passo automático
		}
	}
	else
	{
		ArrTimer += Dt;
	}

	if (!IsPieceControllable())
	{
		return;                           // durante a limpeza só carrega
	}

	// 3. Repetição
	if (Config.ARR <= 0.f)
	{
		while (TryMove(ShiftDir, Out)) {}
		return;
	}
	while (ArrTimer >= Config.ARR)
	{
		ArrTimer -= Config.ARR;
		if (!TryMove(ShiftDir, Out))
		{
			ArrTimer = 0.f;
			break;
		}
	}
}

void FNSGameSimulation::UpdateGravity(float Dt, const FNSInputFrame& Input, TArray<FNSGameEvent>& Out)
{
	if (IsGrounded())
	{
		GravityTimer = 0.f;
		return;
	}

	const float G = Scoring.GetGravityInterval(Config);
	const bool bSoft = Input.bSoftDropHeld;
	const float Interval = bSoft ? FMath::Min(G / Config.SoftDropFactor, Config.SoftDropMaxInterval) : G;

	GravityTimer += Dt;
	int32 Fallen = 0;
	while (GravityTimer >= Interval)
	{
		if (!Board.CanPlace(Active.Moved(0, -1)))
		{
			GravityTimer = 0.f;
			break;
		}
		Active = Active.Moved(0, -1);
		GravityTimer -= Interval;
		++Fallen;
	}

	if (Fallen > 0)
	{
		if (bSoft)
		{
			Scoring.AddDropPoints(Fallen, false, Config);
			EmitScore(Out);
		}
		EmitPieceEvent(ENSGameEventType::PieceMoved, Out, 0, /*bByGravity*/ true);
	}
}

void FNSGameSimulation::UpdateLock(float Dt, TArray<FNSGameEvent>& Out)
{
	if (!IsGrounded())
	{
		if (State == ENSSimState::Grounded)
		{
			State = ENSSimState::Falling;
			LockTimer = 0.f;
			bLockWarned = false;
		}
		return;
	}

	if (State == ENSSimState::Falling)
	{
		State = ENSSimState::Grounded;
		LockTimer = 0.f;
		bLockWarned = false;
	}

	LockTimer += Dt;
	const float Remaining = Config.LockDelay - LockTimer;
	if (!bLockWarned && Remaining <= Config.LockWarningTime)
	{
		bLockWarned = true;
		FNSGameEvent E;
		E.Type = ENSGameEventType::LockWarning;
		E.FloatA = FMath::Max(0.f, Remaining);
		Out.Add(MoveTemp(E));
	}
	if (LockTimer >= Config.LockDelay)
	{
		LockPiece(Out);
	}
}

void FNSGameSimulation::OnSuccessfulManipulation()
{
	if (State == ENSSimState::Grounded && LockResets < Config.MaxLockResets)
	{
		LockTimer = 0.f;
		++LockResets;
		bLockWarned = false;
	}
}

void FNSGameSimulation::ResetShift()
{
	ShiftDir = 0;
	DasTimer = 0.f;
	ArrTimer = 0.f;
	bDasCharged = false;
}

// ---------------------------------------------------------------------------
// Ações
// ---------------------------------------------------------------------------

bool FNSGameSimulation::TryMove(int32 Dx, TArray<FNSGameEvent>& Out)
{
	const FNSActivePiece Next = Active.Moved(Dx, 0);
	if (!Board.CanPlace(Next))
	{
		return false;
	}
	Active = Next;
	OnSuccessfulManipulation();
	EmitPieceEvent(ENSGameEventType::PieceMoved, Out, 0, false);
	return true;
}

bool FNSGameSimulation::TryRotate(int32 Dir, TArray<FNSGameEvent>& Out)
{
	if (!FNSPieceTables::CanRotate(Active.Type))
	{
		return false;
	}

	TArray<FIntPoint, TInlineAllocator<6>> Kicks;
	FNSPieceTables::GetKicks(Active.Type, Config.bAllowUpKick, Kicks);
	const FNSActivePiece Rotated = Active.Rotated(Dir);

	for (int32 i = 0; i < Kicks.Num(); ++i)
	{
		const FNSActivePiece Candidate = Rotated.Moved(Kicks[i].X, Kicks[i].Y);
		if (Board.CanPlace(Candidate))
		{
			Active = Candidate;
			OnSuccessfulManipulation();
			EmitPieceEvent(ENSGameEventType::PieceRotated, Out, i);
			return true;
		}
	}
	return false;
}

void FNSGameSimulation::DoHardDrop(TArray<FNSGameEvent>& Out)
{
	const int32 FromY = Active.Origin.Y;
	Active = Board.GetDropPosition(Active);
	const int32 Distance = FromY - Active.Origin.Y;

	Scoring.AddDropPoints(Distance, true, Config);
	EmitPieceEvent(ENSGameEventType::PieceHardDropped, Out, FromY);
	Out.Last().IntB = Active.Origin.Y;
	EmitScore(Out);
	LockPiece(Out);
}

void FNSGameSimulation::DoHold(TArray<FNSGameEvent>& Out)
{
	if (!bCanHold)
	{
		EmitSimple(ENSGameEventType::HoldDenied, Out);
		return;
	}

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
	EmitHold(Out);
}

// ---------------------------------------------------------------------------
// Ciclo de vida da peça
// ---------------------------------------------------------------------------

void FNSGameSimulation::SpawnPiece(ENSPieceType Type, TArray<FNSGameEvent>& Out)
{
	Active.Type = Type;
	Active.Rotation = 0;
	Active.Origin = FNSPieceTables::GetSpawnOrigin(Type, Config.TotalHeight());

	if (!Board.CanPlace(Active))
	{
		EnterGameOver(Out); // block out
		return;
	}
	if (Board.CanPlace(Active.Moved(0, -1)))
	{
		Active = Active.Moved(0, -1); // desce 1 linha ao nascer
	}

	State = ENSSimState::Falling;
	GravityTimer = 0.f;
	LockTimer = 0.f;
	LockResets = 0;
	bLockWarned = false;
	EmitPieceEvent(ENSGameEventType::PieceSpawned, Out);
}

void FNSGameSimulation::SpawnNextFromQueue(TArray<FNSGameEvent>& Out)
{
	SpawnPiece(Randomizer.Pop(), Out);
	EmitQueue(Out);
}

void FNSGameSimulation::LockPiece(TArray<FNSGameEvent>& Out)
{
	FIntPoint Cells[4];
	Active.GetCells(Cells);

	bool bAllHidden = true; // lock out: peça inteira na zona oculta
	for (const FIntPoint& C : Cells)
	{
		bAllHidden &= (C.Y >= Config.VisibleHeight);
	}

	// Emitir antes de gravar na grade (o ghost seria calculado errado depois)
	EmitPieceEvent(ENSGameEventType::PieceLocked, Out);

	TArray<int32> Touched;
	Board.Lock(Active, Touched);
	++BoardRevision;
	EmitSimple(ENSGameEventType::BoardChanged, Out);

	Active = FNSActivePiece();
	bCanHold = true;
	EmitHold(Out);

	if (bAllHidden)
	{
		EnterGameOver(Out);
		return;
	}

	TArray<int32> Full;
	Board.FindFullRows(Touched, Full);
	const FNSScoring::FLockResult Result = Scoring.ApplyLock(Full.Num(), Config);

	if (Full.Num() > 0)
	{
		FNSGameEvent E;
		E.Type = ENSGameEventType::LinesCleared;
		E.Rows = Full;
		E.IntA = Full.Num();
		E.IntB = Scoring.Combo;
		E.IntC = Result.Gained;
		Out.Add(MoveTemp(E));
		EmitScore(Out);

		if (Result.bLevelUp)
		{
			FNSGameEvent L;
			L.Type = ENSGameEventType::LevelUp;
			L.IntA = Scoring.Level;
			Out.Add(MoveTemp(L));
		}

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
	EmitSimple(ENSGameEventType::BoardChanged, Out);
	SpawnNextFromQueue(Out);
}

void FNSGameSimulation::EnterGameOver(TArray<FNSGameEvent>& Out)
{
	State = ENSSimState::GameOver;
	Active = FNSActivePiece();

	FNSGameEvent E;
	E.Type = ENSGameEventType::GameOver;
	E.Score = Scoring.Score;
	E.IntA = Scoring.Lines;
	E.IntB = Scoring.Level;
	Out.Add(MoveTemp(E));
}

// ---------------------------------------------------------------------------
// Eventos
// ---------------------------------------------------------------------------

void FNSGameSimulation::EmitPieceEvent(ENSGameEventType Type, TArray<FNSGameEvent>& Out, int32 IntA, bool bFlag) const
{
	FNSGameEvent E;
	E.Type = Type;
	E.Piece = Active.Type;
	E.IntA = IntA;
	E.bFlag = bFlag;

	FIntPoint Cells[4];
	Active.GetCells(Cells);
	E.Cells.Append(Cells, 4);

	FIntPoint Ghost[4];
	GetGhost().GetCells(Ghost);
	E.Ghost.Append(Ghost, 4);

	Out.Add(MoveTemp(E));
}

void FNSGameSimulation::EmitSimple(ENSGameEventType Type, TArray<FNSGameEvent>& Out) const
{
	FNSGameEvent E;
	E.Type = Type;
	Out.Add(MoveTemp(E));
}

void FNSGameSimulation::EmitScore(TArray<FNSGameEvent>& Out) const
{
	FNSGameEvent E;
	E.Type = ENSGameEventType::ScoreChanged;
	E.Score = Scoring.Score;
	E.IntA = Scoring.Lines;
	E.IntB = Scoring.Level;
	Out.Add(MoveTemp(E));
}

void FNSGameSimulation::EmitQueue(TArray<FNSGameEvent>& Out) const
{
	FNSGameEvent E;
	E.Type = ENSGameEventType::QueueChanged;
	Randomizer.Peek(Config.PreviewCount, E.Queue);
	Out.Add(MoveTemp(E));
}

void FNSGameSimulation::EmitHold(TArray<FNSGameEvent>& Out) const
{
	FNSGameEvent E;
	E.Type = ENSGameEventType::HoldChanged;
	E.Piece = HoldPiece;
	E.bFlag = bCanHold;
	Out.Add(MoveTemp(E));
}
