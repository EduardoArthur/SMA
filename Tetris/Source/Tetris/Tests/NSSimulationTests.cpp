// Neon Stack - Automation Tests da simulação (core-mechanics §13, T1-T12).
// Rodar: Tools > Session Frontend > Automation, filtrar por "NeonStack".

#include "Misc/AutomationTest.h"
#include "Simulation/NSGameSimulation.h"

#if WITH_DEV_AUTOMATION_TESTS

namespace NSTest
{
	constexpr EAutomationTestFlags Flags = EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter;

	/** Simulação com config padrão, já iniciada com seed fixa. */
	static void MakeSim(FNSGameSimulation& Sim, int32 Seed = 1234)
	{
		Sim.Configure(FNSSimConfig());
		TArray<FNSGameEvent> Ev;
		Sim.Start(Seed, Ev);
	}

	static void Step(FNSGameSimulation& Sim, float Dt, const FNSInputFrame& Input, TArray<FNSGameEvent>& Ev)
	{
		Ev.Reset();
		Sim.Tick(Dt, Input, Ev);
	}

	/** Uma borda de input, sem avançar o tempo. */
	static void Press(FNSGameSimulation& Sim, ENSInputAction Action, TArray<FNSGameEvent>& Ev)
	{
		FNSInputFrame F;
		F.Pressed.Add(Action);
		Step(Sim, 0.f, F, Ev);
	}

	/** Avança o tempo sem input. */
	static void Wait(FNSGameSimulation& Sim, float Seconds, TArray<FNSGameEvent>& Ev)
	{
		Step(Sim, Seconds, FNSInputFrame(), Ev);
	}

	static FNSActivePiece Piece(ENSPieceType Type, int32 Rotation, int32 X, int32 Y)
	{
		FNSActivePiece P;
		P.Type = Type;
		P.Rotation = Rotation;
		P.Origin = FIntPoint(X, Y);
		return P;
	}

	static void FillRow(FNSBoardGrid& Board, int32 Y, int32 HoleX = INDEX_NONE)
	{
		for (int32 X = 0; X < Board.GetWidth(); ++X)
		{
			if (X != HoleX)
			{
				Board.Test_SetCell(X, Y, static_cast<uint8>(ENSPieceType::O));
			}
		}
	}

	static const FNSGameEvent* Find(const TArray<FNSGameEvent>& Ev, ENSGameEventType Type)
	{
		return Ev.FindByPredicate([Type](const FNSGameEvent& E) { return E.Type == Type; });
	}
}

using namespace NSTest;

// T1 - Peça T encostada na parede esquerda gira usando o chute +1
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FNSTest_T01_RotateWallKick, "NeonStack.Simulation.T01_RotateWallKick", NSTest::Flags)
bool FNSTest_T01_RotateWallKick::RunTest(const FString& Parameters)
{
	FNSGameSimulation Sim; MakeSim(Sim);
	TArray<FNSGameEvent> Ev;

	// Estado R (vertical na coluna 0 com a ponta para a direita); girar para o estado 2 colide com a parede
	Sim.Test_SetActive(Piece(ENSPieceType::T, 1, -1, 10));
	TestTrue(TEXT("Posição inicial válida"), Sim.GetBoard().CanPlace(Sim.GetActive()));

	Press(Sim, ENSInputAction::RotateCW, Ev);
	TestEqual(TEXT("Rotação = 2"), Sim.GetActive().Rotation, 2);
	TestEqual(TEXT("Origem deslocada +1"), Sim.GetActive().Origin.X, 0);
	const FNSGameEvent* E = Find(Ev, ENSGameEventType::PieceRotated);
	TestNotNull(TEXT("Evento de rotação"), E);
	if (E) { TestEqual(TEXT("Kick índice 2 (+1)"), E->IntA, 2); }
	return true;
}

// T2 - Peça I vertical na coluna 0 gira usando o chute +2
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FNSTest_T02_IKick, "NeonStack.Simulation.T02_IKick", NSTest::Flags)
bool FNSTest_T02_IKick::RunTest(const FString& Parameters)
{
	FNSGameSimulation Sim; MakeSim(Sim);
	TArray<FNSGameEvent> Ev;

	Sim.Test_SetActive(Piece(ENSPieceType::I, 1, -2, 5)); // estado R: coluna cx=2 -> X=0
	TestTrue(TEXT("Posição inicial válida"), Sim.GetBoard().CanPlace(Sim.GetActive()));

	Press(Sim, ENSInputAction::RotateCW, Ev);
	TestEqual(TEXT("Rotação = 2"), Sim.GetActive().Rotation, 2);
	TestEqual(TEXT("Origem X = 0"), Sim.GetActive().Origin.X, 0);
	const FNSGameEvent* E = Find(Ev, ENSGameEventType::PieceRotated);
	if (TestNotNull(TEXT("Evento de rotação"), E)) { TestEqual(TEXT("Kick índice 4 (+2)"), E->IntA, 4); }
	return true;
}

// T3 - Peça O não gira
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FNSTest_T03_ONoRotate, "NeonStack.Simulation.T03_ONoRotate", NSTest::Flags)
bool FNSTest_T03_ONoRotate::RunTest(const FString& Parameters)
{
	FNSGameSimulation Sim; MakeSim(Sim);
	TArray<FNSGameEvent> Ev;

	Sim.Test_SetActive(Piece(ENSPieceType::O, 0, 4, 10));
	Press(Sim, ENSInputAction::RotateCW, Ev);
	TestEqual(TEXT("Rotação continua 0"), Sim.GetActive().Rotation, 0);
	TestNull(TEXT("Sem evento de rotação"), Find(Ev, ENSGameEventType::PieceRotated));
	return true;
}

// T4 - Quad com uma I vertical: 800 × nível + pontos do hard drop
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FNSTest_T04_Quad, "NeonStack.Simulation.T04_Quad", NSTest::Flags)
bool FNSTest_T04_Quad::RunTest(const FString& Parameters)
{
	FNSGameSimulation Sim; MakeSim(Sim);
	TArray<FNSGameEvent> Ev;

	for (int32 Y = 0; Y < 4; ++Y) { FillRow(Sim.Test_GetBoard(), Y, /*HoleX*/ 0); }
	Sim.Test_SetActive(Piece(ENSPieceType::I, 1, -2, 10)); // vertical na coluna 0, células Y=10..13

	Press(Sim, ENSInputAction::HardDrop, Ev);
	const int64 Expected = 800 /* Quad nível 1 */ + 2 * 10 /* hard drop de 10 linhas */;
	TestEqual(TEXT("Score"), Sim.GetScoring().Score, Expected);
	TestEqual(TEXT("Linhas"), Sim.GetScoring().Lines, 4);
	const FNSGameEvent* E = Find(Ev, ENSGameEventType::LinesCleared);
	if (TestNotNull(TEXT("Evento de limpeza"), E)) { TestEqual(TEXT("4 linhas"), E->IntA, 4); }
	TestEqual(TEXT("Estado = limpando"), Sim.GetState(), ENSSimState::ClearingLines);

	Wait(Sim, 0.3f, Ev); // termina a animação
	for (int32 X = 0; X < 10; ++X)
	{
		TestEqual(TEXT("Fundo vazio após o Quad"), (int32)Sim.GetBoard().Get(X, 0), 0);
	}
	TestNotNull(TEXT("Nova peça nasceu"), Find(Ev, ENSGameEventType::PieceSpawned));
	return true;
}

// T5 - Linhas 0 e 2 cheias, linha 1 não: remove 0 e 2 e a linha 1 desce para Y=0
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FNSTest_T05_NonContiguousClear, "NeonStack.Simulation.T05_NonContiguousClear", NSTest::Flags)
bool FNSTest_T05_NonContiguousClear::RunTest(const FString& Parameters)
{
	FNSBoardGrid Board;
	Board.Init(10, 22);
	FillRow(Board, 0);
	Board.Test_SetCell(3, 1, static_cast<uint8>(ENSPieceType::T));
	FillRow(Board, 2);
	Board.Test_SetCell(7, 3, static_cast<uint8>(ENSPieceType::L));

	TArray<int32> Full;
	Board.FindFullRows({ 0, 1, 2, 3 }, Full);
	TestEqual(TEXT("2 linhas cheias"), Full.Num(), 2);

	Board.ClearRows(Full);
	TestEqual(TEXT("Bloco da linha 1 desceu para Y=0"), (int32)Board.Get(3, 0), (int32)ENSPieceType::T);
	TestEqual(TEXT("Bloco da linha 3 desceu para Y=1"), (int32)Board.Get(7, 1), (int32)ENSPieceType::L);
	TestEqual(TEXT("Y=2 vazio"), (int32)Board.Get(0, 2), 0);
	return true;
}

// T6 - Lock delay: 15 resets; o 16º movimento não reinicia o timer
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FNSTest_T06_LockResets, "NeonStack.Simulation.T06_LockResets", NSTest::Flags)
bool FNSTest_T06_LockResets::RunTest(const FString& Parameters)
{
	FNSGameSimulation Sim; MakeSim(Sim);
	TArray<FNSGameEvent> Ev;

	Sim.Test_SetActive(Piece(ENSPieceType::O, 0, 4, 0)); // no chão
	Wait(Sim, 0.f, Ev);
	TestEqual(TEXT("Apoiada"), Sim.GetState(), ENSSimState::Grounded);

	for (int32 i = 0; i < 15; ++i)
	{
		Wait(Sim, 0.1f, Ev);
		Press(Sim, (i % 2 == 0) ? ENSInputAction::MoveLeft : ENSInputAction::MoveRight, Ev); // reset
	}
	Wait(Sim, 0.4f, Ev);
	TestEqual(TEXT("Ainda apoiada após 15 resets + 0,4 s"), Sim.GetState(), ENSSimState::Grounded);

	Press(Sim, ENSInputAction::MoveRight, Ev); // 16º: move, mas não reinicia o timer
	Wait(Sim, 0.11f, Ev);
	TestNotNull(TEXT("Travou"), Find(Ev, ENSGameEventType::PieceLocked));
	TestEqual(TEXT("Bloco em (4,0)"), (int32)Sim.GetBoard().Get(4, 0), (int32)ENSPieceType::O);
	TestEqual(TEXT("Bloco em (5,0)"), (int32)Sim.GetBoard().Get(5, 0), (int32)ENSPieceType::O);
	return true;
}

// T7 - Hold duas vezes na mesma peça: o segundo é negado
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FNSTest_T07_HoldTwice, "NeonStack.Simulation.T07_HoldTwice", NSTest::Flags)
bool FNSTest_T07_HoldTwice::RunTest(const FString& Parameters)
{
	FNSGameSimulation Sim; MakeSim(Sim);
	TArray<FNSGameEvent> Ev;

	const ENSPieceType First = Sim.GetActive().Type;
	Press(Sim, ENSInputAction::Hold, Ev);
	TestEqual(TEXT("Peça guardada"), Sim.GetHoldPiece(), First);
	TestFalse(TEXT("Hold bloqueado"), Sim.CanHold());

	const ENSPieceType Current = Sim.GetActive().Type;
	Press(Sim, ENSInputAction::Hold, Ev);
	TestNotNull(TEXT("HoldDenied emitido"), Find(Ev, ENSGameEventType::HoldDenied));
	TestEqual(TEXT("Hold não mudou"), Sim.GetHoldPiece(), First);
	TestEqual(TEXT("Peça ativa não mudou"), Sim.GetActive().Type, Current);
	return true;
}

// T8 - Saco de 7: cada grupo de 7 contém as 7 peças
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FNSTest_T08_SevenBag, "NeonStack.Simulation.T08_SevenBag", NSTest::Flags)
bool FNSTest_T08_SevenBag::RunTest(const FString& Parameters)
{
	FNSRandomizer R;
	R.Reset(42, 4);
	for (int32 Bag = 0; Bag < 3; ++Bag)
	{
		TSet<ENSPieceType> Seen;
		for (int32 i = 0; i < 7; ++i) { Seen.Add(R.Pop()); }
		TestEqual(FString::Printf(TEXT("Saco %d tem 7 peças distintas"), Bag), Seen.Num(), 7);
	}

	// Mesma seed = mesma sequência
	FNSRandomizer A, B;
	A.Reset(7, 4);
	B.Reset(7, 4);
	for (int32 i = 0; i < 21; ++i) { TestEqual(TEXT("Determinístico"), A.Pop(), B.Pop()); }
	return true;
}

// T9 - Block out: a nova peça nasce sobre blocos -> game over
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FNSTest_T09_BlockOut, "NeonStack.Simulation.T09_BlockOut", NSTest::Flags)
bool FNSTest_T09_BlockOut::RunTest(const FString& Parameters)
{
	FNSGameSimulation Sim; MakeSim(Sim);
	TArray<FNSGameEvent> Ev;

	for (int32 Y = 10; Y < 22; ++Y) { FillRow(Sim.Test_GetBoard(), Y, /*HoleX*/ 9); } // não completas
	Sim.Test_SetActive(Piece(ENSPieceType::O, 0, 0, 0));

	Press(Sim, ENSInputAction::HardDrop, Ev);
	TestEqual(TEXT("Game over"), Sim.GetState(), ENSSimState::GameOver);
	TestNotNull(TEXT("Evento de game over"), Find(Ev, ENSGameEventType::GameOver));
	return true;
}

// T10 - Hard drop de Y=18 até Y=0: +36 pontos e trava na hora
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FNSTest_T10_HardDropPoints, "NeonStack.Simulation.T10_HardDropPoints", NSTest::Flags)
bool FNSTest_T10_HardDropPoints::RunTest(const FString& Parameters)
{
	FNSGameSimulation Sim; MakeSim(Sim);
	TArray<FNSGameEvent> Ev;

	Sim.Test_SetActive(Piece(ENSPieceType::O, 0, 4, 18));
	Press(Sim, ENSInputAction::HardDrop, Ev);
	TestEqual(TEXT("Score = 2 × 18"), Sim.GetScoring().Score, (int64)36);
	TestNotNull(TEXT("Travou"), Find(Ev, ENSGameEventType::PieceLocked));
	TestEqual(TEXT("Bloco em (4,0)"), (int32)Sim.GetBoard().Get(4, 0), (int32)ENSPieceType::O);
	return true;
}

// T11 - Combo: 2ª limpeza seguida soma +50, 3ª soma +100 (nível 1)
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FNSTest_T11_Combo, "NeonStack.Simulation.T11_Combo", NSTest::Flags)
bool FNSTest_T11_Combo::RunTest(const FString& Parameters)
{
	const FNSSimConfig Cfg;
	FNSScoring S;
	S.Reset(Cfg);

	TestEqual(TEXT("Single sem combo"), S.ApplyLock(1, Cfg).Gained, 100);
	TestEqual(TEXT("Double + combo 1"), S.ApplyLock(2, Cfg).Gained, 300 + 50);
	TestEqual(TEXT("Single + combo 2"), S.ApplyLock(1, Cfg).Gained, 100 + 100);
	S.ApplyLock(0, Cfg);
	TestEqual(TEXT("Combo zerado"), S.Combo, -1);
	return true;
}

// T12 - Level up: a limpeza que sobe de nível pontua com o nível antigo
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FNSTest_T12_LevelUp, "NeonStack.Simulation.T12_LevelUp", NSTest::Flags)
bool FNSTest_T12_LevelUp::RunTest(const FString& Parameters)
{
	const FNSSimConfig Cfg;
	FNSScoring S;
	S.Reset(Cfg);
	S.Lines = 9;

	const FNSScoring::FLockResult R = S.ApplyLock(1, Cfg);
	TestEqual(TEXT("Pontuou com o nível 1"), R.Gained, 100);
	TestTrue(TEXT("Subiu de nível"), R.bLevelUp);
	TestEqual(TEXT("Nível 2"), S.Level, 2);
	TestEqual(TEXT("Gravidade nível 1 = 1,0 s"), FNSScoring().GetGravityInterval(Cfg), 1.0f);
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
