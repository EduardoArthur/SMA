# C++ Spec 02 - Gameplay, dados e persistência

> **Autora:** Priscilla (UE5 Programmer Lead) 💻
> **Data:** 2026-11-07 · **Versão:** 1.0
> **Base:** [ue5-architecture.md](../ue5-architecture.md) §3.2, §4-8 · [01-simulation.md](01-simulation.md)
> **Marcos:** M1 (`ANSBoardActor`, `UNSBoardViewComponent`, `ANSPlayerController`) · M3 (`UNSGameConfig`, `UNSSaveGame`, `UNSSaveSubsystem`, `ANSGameMode`)

Convenções:
- Export macro do módulo: **`TETRIS_API`**.
- Categoria das propriedades: `"NeonStack"` (+ subcategorias).
- Headers de UE incluídos só no `.cpp` quando possível (forward declarations no `.h`).

---

## 0. `Tetris.Build.cs`

Já contém `Core, CoreUObject, Engine, InputCore, EnhancedInput`. Adicionar:

```csharp
PublicIncludePaths.Add(ModuleDirectory);   // permite #include "Simulation/NSTypes.h"
```
Nenhum outro módulo é necessário para o C++ (UMG, Niagara e MetaSounds ficam nos Blueprints).

---

## 1. `UNSGameConfig` (`Data/NSGameConfig.h`): M3 (crie já no M1 com os campos do M1)

```cpp
#pragma once
#include "Engine/DataAsset.h"
#include "Simulation/NSTypes.h"
#include "NSGameConfig.generated.h"

UCLASS(BlueprintType)
class TETRIS_API UNSGameConfig : public UPrimaryDataAsset
{
	GENERATED_BODY()
public:
	// Board
	UPROPERTY(EditAnywhere, Category="NeonStack|Board", meta=(ClampMin=4))  int32 BoardWidth = 10;
	UPROPERTY(EditAnywhere, Category="NeonStack|Board", meta=(ClampMin=4))  int32 VisibleHeight = 20;
	UPROPERTY(EditAnywhere, Category="NeonStack|Board", meta=(ClampMin=1))  int32 HiddenRows = 2;
	UPROPERTY(EditAnywhere, Category="NeonStack|Board", meta=(ClampMin=0, ClampMax=6)) int32 PreviewCount = 3;

	// Handling (padrões; as opções do jogador sobrescrevem DAS/ARR)
	UPROPERTY(EditAnywhere, Category="NeonStack|Handling", meta=(Units="s")) float DAS = 0.170f;
	UPROPERTY(EditAnywhere, Category="NeonStack|Handling", meta=(Units="s")) float ARR = 0.050f;
	UPROPERTY(EditAnywhere, Category="NeonStack|Handling") float SoftDropFactor = 20.f;
	UPROPERTY(EditAnywhere, Category="NeonStack|Handling", meta=(Units="s")) float SoftDropMaxInterval = 0.033f;

	// Lock
	UPROPERTY(EditAnywhere, Category="NeonStack|Lock", meta=(Units="s")) float LockDelay = 0.5f;
	UPROPERTY(EditAnywhere, Category="NeonStack|Lock") int32 MaxLockResets = 15;
	UPROPERTY(EditAnywhere, Category="NeonStack|Lock", meta=(Units="s")) float LockWarningTime = 0.2f;
	UPROPERTY(EditAnywhere, Category="NeonStack|Lock") bool bAllowUpKick = true;
	UPROPERTY(EditAnywhere, Category="NeonStack|Lock", meta=(Units="s")) float LineClearDelay = 0.25f;

	// Score
	UPROPERTY(EditAnywhere, Category="NeonStack|Score", EditFixedSize) TArray<int32> LineScore = { 0, 100, 300, 500, 800 };
	UPROPERTY(EditAnywhere, Category="NeonStack|Score") int32 ComboBonus = 50;
	UPROPERTY(EditAnywhere, Category="NeonStack|Score") int32 SoftDropPointsPerCell = 1;
	UPROPERTY(EditAnywhere, Category="NeonStack|Score") int32 HardDropPointsPerCell = 2;

	// Progression
	UPROPERTY(EditAnywhere, Category="NeonStack|Progression") int32 LinesPerLevel = 10;
	UPROPERTY(EditAnywhere, Category="NeonStack|Progression") int32 StartLevel = 1;
	UPROPERTY(EditAnywhere, Category="NeonStack|Progression") float GravityBase = 0.8f;
	UPROPERTY(EditAnywhere, Category="NeonStack|Progression") float GravityLevelStep = 0.007f;
	UPROPERTY(EditAnywhere, Category="NeonStack|Progression", meta=(Units="s")) float MinGravity = 0.05f;
	UPROPERTY(EditAnywhere, Category="NeonStack|Progression", meta=(ToolTip="0 = sem teto")) int32 MaxLevel = 0;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="NeonStack|Progression") int32 PaletteEveryNLevels = 5;

	// Debug
	UPROPERTY(EditAnywhere, Category="NeonStack|Debug", meta=(ToolTip="0 = aleatória")) int32 RandomSeed = 0;

	/** Converte para a struct da simulação. */
	FNSSimConfig ToSimConfig() const;
};
```
`ToSimConfig()` copia campo a campo; para `LineScore`, copiar até 5 itens com `FMath::Min(LineScore.Num(), 5)`.

**Asset:** `Content/NeonStack/Data/DA_GameConfig` (botão direito → Miscellaneous → Data Asset → `NSGameConfig`).

---

## 2. `UNSSaveGame` (`Data/NSSaveGame.h`): M3

```cpp
#pragma once
#include "GameFramework/SaveGame.h"
#include "NSSaveGame.generated.h"

UENUM(BlueprintType)
enum class ENSFxIntensity : uint8 { Low, Medium, High };

USTRUCT(BlueprintType)
struct FNSSettings
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(ClampMin=0, ClampMax=100)) int32 MusicVolume = 70;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(ClampMin=0, ClampMax=100)) int32 SfxVolume = 80;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) ENSFxIntensity FxIntensity = ENSFxIntensity::High;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bScreenShake = true;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(ClampMin=50, ClampMax=300)) int32 DasMs = 170;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(ClampMin=0, ClampMax=100))  int32 ArrMs = 50;
};

UCLASS()
class TETRIS_API UNSSaveGame : public USaveGame
{
	GENERATED_BODY()
public:
	UPROPERTY() int32 SaveVersion = 1;
	UPROPERTY() int64 HighScore = 0;
	UPROPERTY() int32 HighScoreLines = 0;
	UPROPERTY() int32 HighScoreLevel = 0;
	UPROPERTY() FDateTime HighScoreDate;
	UPROPERTY() FNSSettings Settings;
};
```

---

## 3. `UNSSaveSubsystem` (`Systems/NSSaveSubsystem.h`): M3

```cpp
#pragma once
#include "Subsystems/GameInstanceSubsystem.h"
#include "Data/NSSaveGame.h"
#include "NSSaveSubsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FNSOnSettingsChanged, const FNSSettings&, Settings);

UCLASS()
class TETRIS_API UNSSaveSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	UFUNCTION(BlueprintPure, Category="NeonStack|Save") const FNSSettings& GetSettings() const;
	UFUNCTION(BlueprintCallable, Category="NeonStack|Save") void SetSettings(const FNSSettings& NewSettings);

	UFUNCTION(BlueprintPure, Category="NeonStack|Save") int64 GetHighScore() const;
	UFUNCTION(BlueprintPure, Category="NeonStack|Save") int32 GetHighScoreLines() const;
	UFUNCTION(BlueprintPure, Category="NeonStack|Save") int32 GetHighScoreLevel() const;

	/** Registra o resultado; retorna true se é um novo recorde. Salva no disco. */
	UFUNCTION(BlueprintCallable, Category="NeonStack|Save") bool SubmitResult(int64 Score, int32 Lines, int32 Level);

	UPROPERTY(BlueprintAssignable, Category="NeonStack|Save") FNSOnSettingsChanged OnSettingsChanged;

private:
	void Save();
	UPROPERTY() TObjectPtr<UNSSaveGame> Data;
	static const FString SlotName;   // "NeonStack_Save", UserIndex 0
};
```

- `Initialize`: `if (UGameplayStatics::DoesSaveGameExist(SlotName, 0)) Data = Cast<UNSSaveGame>(LoadGameFromSlot(...));` → se nulo, `CreateSaveGameObject(UNSSaveGame::StaticClass())`.
- `SetSettings`: grava, `Save()`, `OnSettingsChanged.Broadcast(...)`.
- `SubmitResult`: `if (Score > Data->HighScore)` atualiza os 4 campos (data = `FDateTime::Now()`), `Save()`, retorna `true`.
- `Save()`: `UGameplayStatics::SaveGameToSlot(Data, SlotName, 0)` (síncrono é aceitável; o arquivo é minúsculo).

---

## 4. `ANSBoardActor` (`Game/NSBoardActor.h`): M1

### 4.1 Delegates
```cpp
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FNSOnPieceSpawned, ENSPieceType, Type, const TArray<FIntPoint>&, Cells);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FNSOnPieceMoved, const TArray<FIntPoint>&, Cells, const TArray<FIntPoint>&, Ghost, bool, bByGravity);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FNSOnPieceRotated, const TArray<FIntPoint>&, Cells, const TArray<FIntPoint>&, Ghost, int32, KickIndex);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FNSOnPieceHardDropped, int32, FromY, int32, ToY, const TArray<FIntPoint>&, Cells);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FNSOnPieceLocked, ENSPieceType, Type, const TArray<FIntPoint>&, Cells);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FNSOnLinesCleared, const TArray<int32>&, Rows, int32, Count, int32, Combo, int32, ScoreGained);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FNSOnScoreChanged, int64, Score, int32, Lines, int32, Level);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FNSOnLevelUp, int32, NewLevel);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FNSOnHoldChanged, ENSPieceType, Held, bool, bCanHold);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FNSOnHoldDenied);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FNSOnQueueChanged, const TArray<ENSPieceType>&, Next);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FNSOnLockWarning, float, Remaining);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FNSOnPausedChanged, bool, bPaused);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FNSOnGameOver, int64, Score, int32, Lines, int32, Level, bool, bNewHighScore);
```
Em relação à arquitetura §5, **`OnHoldDenied` foi adicionado** (necessário para o `SFX_HoldDenied` da audio-bible). O `BoardChanged` não vira delegate: é consumido internamente pela view.

### 4.2 Classe
```cpp
#pragma once
#include "GameFramework/Actor.h"
#include "Simulation/NSGameSimulation.h"
#include "NSBoardActor.generated.h"

class UNSGameConfig;
class UNSBoardViewComponent;

UCLASS()
class TETRIS_API ANSBoardActor : public AActor
{
	GENERATED_BODY()
public:
	ANSBoardActor();

	// --- Configuração ---
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="NeonStack") TObjectPtr<UNSGameConfig> Config;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="NeonStack") TObjectPtr<UNSBoardViewComponent> View;

	// --- Controle (chamado pelo GameMode / PlayerController) ---
	UFUNCTION(BlueprintCallable, Category="NeonStack") void StartGame();
	UFUNCTION(BlueprintCallable, Category="NeonStack") void StopGame();
	UFUNCTION(BlueprintCallable, Category="NeonStack") void SetPaused(bool bInPaused);
	UFUNCTION(BlueprintCallable, Category="NeonStack") void PressAction(ENSInputAction Action);
	UFUNCTION(BlueprintCallable, Category="NeonStack") void ReleaseAction(ENSInputAction Action);
	UFUNCTION(BlueprintCallable, Category="NeonStack") void SetHandling(int32 DasMs, int32 ArrMs);

	// --- Leitura ---
	UFUNCTION(BlueprintPure, Category="NeonStack") bool IsRunning() const { return bRunning; }
	UFUNCTION(BlueprintPure, Category="NeonStack") bool IsPaused() const { return bPaused; }
	UFUNCTION(BlueprintPure, Category="NeonStack") ENSSimState GetSimState() const { return Sim.GetState(); }
	UFUNCTION(BlueprintPure, Category="NeonStack") int64 GetScore() const { return Sim.GetScoring().Score; }
	UFUNCTION(BlueprintPure, Category="NeonStack") int32 GetLines() const { return Sim.GetScoring().Lines; }
	UFUNCTION(BlueprintPure, Category="NeonStack") int32 GetGameLevel() const { return Sim.GetScoring().Level; }  // "GetLevel" colide com AActor::GetLevel
	UFUNCTION(BlueprintPure, Category="NeonStack") int32 GetPaletteIndex() const;   // (Level-1)/PaletteEveryNLevels
	UFUNCTION(BlueprintPure, Category="NeonStack") FVector CellToWorld(FIntPoint Cell) const;   // para FX
	const FNSGameSimulation& GetSimulation() const { return Sim; }

	// --- Eventos ---
	UPROPERTY(BlueprintAssignable, Category="NeonStack|Events") FNSOnPieceSpawned OnPieceSpawned;
	UPROPERTY(BlueprintAssignable, Category="NeonStack|Events") FNSOnPieceMoved OnPieceMoved;
	UPROPERTY(BlueprintAssignable, Category="NeonStack|Events") FNSOnPieceRotated OnPieceRotated;
	UPROPERTY(BlueprintAssignable, Category="NeonStack|Events") FNSOnPieceHardDropped OnPieceHardDropped;
	UPROPERTY(BlueprintAssignable, Category="NeonStack|Events") FNSOnPieceLocked OnPieceLocked;
	UPROPERTY(BlueprintAssignable, Category="NeonStack|Events") FNSOnLinesCleared OnLinesCleared;
	UPROPERTY(BlueprintAssignable, Category="NeonStack|Events") FNSOnScoreChanged OnScoreChanged;
	UPROPERTY(BlueprintAssignable, Category="NeonStack|Events") FNSOnLevelUp OnLevelUp;
	UPROPERTY(BlueprintAssignable, Category="NeonStack|Events") FNSOnHoldChanged OnHoldChanged;
	UPROPERTY(BlueprintAssignable, Category="NeonStack|Events") FNSOnHoldDenied OnHoldDenied;
	UPROPERTY(BlueprintAssignable, Category="NeonStack|Events") FNSOnQueueChanged OnQueueChanged;
	UPROPERTY(BlueprintAssignable, Category="NeonStack|Events") FNSOnLockWarning OnLockWarning;
	UPROPERTY(BlueprintAssignable, Category="NeonStack|Events") FNSOnPausedChanged OnPausedChanged;
	UPROPERTY(BlueprintAssignable, Category="NeonStack|Events") FNSOnGameOver OnGameOver;

	virtual void Tick(float DeltaSeconds) override;

protected:
	virtual void BeginPlay() override;

private:
	void ProcessEvents();                         // sync da view + broadcast
	void BroadcastEvent(const FNSGameEvent& E);
	void ApplyHandlingFromSave();

	FNSGameSimulation Sim;
	FNSInputFrame PendingInput;
	TArray<FNSGameEvent> EventBuffer;             // reutilizado entre ticks
	bool bRunning = false;
	bool bPaused = false;

	static constexpr float MaxDeltaTime = 0.1f;
};
```

### 4.3 Comportamento
| Função | Lógica |
|--------|--------|
| Construtor | `PrimaryActorTick.bCanEverTick = true; PrimaryActorTick.TickGroup = TG_PrePhysics;` cria `RootComponent` (`USceneComponent`) e `View` (`CreateDefaultSubobject<UNSBoardViewComponent>`), anexado à root |
| `BeginPlay` | `check(Config)` com mensagem clara ("Atribua DA_GameConfig no BP_Board"); `View->InitializeView(Config)` |
| `StartGame` | `Sim.Configure(Config->ToSimConfig())`; `ApplyHandlingFromSave()`; `PendingInput = {}`; `bRunning = true; bPaused = false;` `Sim.Start(Config->RandomSeed, EventBuffer)`; `ProcessEvents()` |
| `StopGame` | `bRunning = false` |
| `SetPaused(b)` | Se mudou: `bPaused = b`; ao pausar, limpar `PendingInput.Pressed` e os "held"; `OnPausedChanged.Broadcast(b)` |
| `PressAction(A)` | Se `!bRunning \|\| bPaused` → ignora. Atualiza o held (`MoveLeft → bLeftHeld = true`, etc.) e `PendingInput.Pressed.Add(A)` |
| `ReleaseAction(A)` | Atualiza o held (`false`). Funciona mesmo pausado |
| `SetHandling` | `Sim.SetHandling(DasMs / 1000.f, ArrMs / 1000.f)` |
| `ApplyHandlingFromSave` | Lê `UNSSaveSubsystem::GetSettings()` (via `GetGameInstance()->GetSubsystem<>()`) e chama `SetHandling` |
| `Tick` | `if (!bRunning \|\| bPaused) return;` `EventBuffer.Reset(); Sim.Tick(FMath::Min(Dt, MaxDeltaTime), PendingInput, EventBuffer); PendingInput.Pressed.Reset(); ProcessEvents();` |
| `ProcessEvents` | Se `EventBuffer` não vazio: `View->Sync(Sim)`; depois `for (E : EventBuffer) BroadcastEvent(E)` (**sync antes do broadcast**, para o BP achar a view atualizada) |
| `BroadcastEvent` | `switch (E.Type)` → delegate correspondente (tabela §4.4). Em `GameOver`: `bRunning = false`; `bNew = SaveSubsystem->SubmitResult(...)`; `OnGameOver.Broadcast(E.Score, E.IntA, E.IntB, bNew)` |
| `CellToWorld` | `View->GetComponentTransform().TransformPosition(View->CellToLocal(Cell))` |

### 4.4 Mapa evento → delegate
| `ENSGameEventType` | Broadcast |
|--------------------|-----------|
| PieceSpawned | `OnPieceSpawned(E.Piece, E.Cells)` |
| PieceMoved | `OnPieceMoved(E.Cells, E.Ghost, E.bFlag)` |
| PieceRotated | `OnPieceRotated(E.Cells, E.Ghost, E.IntA)` |
| PieceHardDropped | `OnPieceHardDropped(E.IntA, E.IntB, E.Cells)` |
| PieceLocked | `OnPieceLocked(E.Piece, E.Cells)` |
| LinesCleared | `OnLinesCleared(E.Rows, E.IntA, E.IntB, E.IntC)` |
| BoardChanged | — (só a view) |
| ScoreChanged | `OnScoreChanged(E.Score, E.IntA, E.IntB)` |
| LevelUp | `OnLevelUp(E.IntA)` |
| HoldChanged | `OnHoldChanged(E.Piece, E.bFlag)` |
| HoldDenied | `OnHoldDenied()` |
| QueueChanged | `OnQueueChanged(E.Queue)` |
| LockWarning | `OnLockWarning(E.FloatA)` |
| GameOver | ver §4.3 |

---

## 5. `UNSBoardViewComponent` (`Game/NSBoardViewComponent.h`): M1

Desenha a grade com **Instanced Static Mesh**. Não tem lógica de jogo.

```cpp
#pragma once
#include "Components/SceneComponent.h"
#include "Simulation/NSTypes.h"
#include "NSBoardViewComponent.generated.h"

class UInstancedStaticMeshComponent;
class UStaticMesh;
class UMaterialInterface;
class UNSGameConfig;
class FNSGameSimulation;

UCLASS(ClassGroup=(NeonStack), meta=(BlueprintSpawnableComponent))
class TETRIS_API UNSBoardViewComponent : public USceneComponent
{
	GENERATED_BODY()
public:
	UNSBoardViewComponent();

	// --- Aparência (preencher no BP_Board) ---
	UPROPERTY(EditAnywhere, Category="NeonStack|View") TObjectPtr<UStaticMesh> BlockMesh;          // M1: /Engine/BasicShapes/Cube
	UPROPERTY(EditAnywhere, Category="NeonStack|View") TObjectPtr<UMaterialInterface> BlockMaterial;
	UPROPERTY(EditAnywhere, Category="NeonStack|View") TObjectPtr<UMaterialInterface> GhostMaterial;
	UPROPERTY(EditAnywhere, Category="NeonStack|View") float CellSize = 100.f;
	UPROPERTY(EditAnywhere, Category="NeonStack|View") float BlockScale = 0.95f;          // pequeno vão entre blocos
	UPROPERTY(EditAnywhere, Category="NeonStack|View", EditFixedSize) TArray<FLinearColor> PieceColors;  // 7, ordem I,O,T,S,Z,J,L
	UPROPERTY(EditAnywhere, Category="NeonStack|View") float FixedBrightness = 0.7f;
	UPROPERTY(EditAnywhere, Category="NeonStack|View") float ActiveBrightness = 1.0f;
	UPROPERTY(EditAnywhere, Category="NeonStack|View") float GhostAlpha = 0.3f;
	UPROPERTY(EditAnywhere, Category="NeonStack|View") float PulseDecayTime = 0.15f;

	void InitializeView(const UNSGameConfig* Config);
	void Sync(const FNSGameSimulation& Sim);

	/** Acende os blocos fixos nessas células (Pulse = 1) e decai em PulseDecayTime. Chamado pelo BP no OnPieceLocked. */
	UFUNCTION(BlueprintCallable, Category="NeonStack|View") void PulseCells(const TArray<FIntPoint>& Cells, float Strength = 1.f);

	/** Pinta linhas inteiras de branco (início da limpeza). */
	UFUNCTION(BlueprintCallable, Category="NeonStack|View") void FlashRows(const TArray<int32>& Rows);

	UFUNCTION(BlueprintPure, Category="NeonStack|View") FVector CellToLocal(FIntPoint Cell) const;
	UFUNCTION(BlueprintPure, Category="NeonStack|View") FLinearColor GetPieceColor(ENSPieceType Type) const;

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	void RebuildFixed(const FNSGameSimulation& Sim);
	void UpdateActiveAndGhost(const FNSGameSimulation& Sim);
	void SetInstanceData(UInstancedStaticMeshComponent* ISM, int32 Index, ENSPieceType Type, float Brightness, float Pulse, float Alpha);

	UPROPERTY() TObjectPtr<UInstancedStaticMeshComponent> FixedISM;
	UPROPERTY() TObjectPtr<UInstancedStaticMeshComponent> ActiveISM;
	UPROPERTY() TObjectPtr<UInstancedStaticMeshComponent> GhostISM;

	int32 Width = 10, VisibleHeight = 20;
	uint32 LastBoardRevision = MAX_uint32;
	TArray<int32> CellToInstance;     // [Y*Width+X] → índice no FixedISM, -1 se vazio
	TArray<float> InstancePulse;      // pulso por instância do FixedISM
	bool bAnyPulse = false;
};
```

### Pontos de implementação
- **Construtor:** cria os 3 ISM com `CreateDefaultSubobject`, `SetupAttachment(this)`, `NumCustomDataFloats = 7`, `SetCollisionEnabled(NoCollision)`, `SetCastShadow(false)`. `PrimaryComponentTick.bCanEverTick = true; bStartWithTickEnabled = false` (liga só enquanto houver pulso).
- **`InitializeView`:** aplica mesh/material aos ISMs (o Ghost usa `GhostMaterial` se houver, senão `BlockMaterial`); lê `Width`/`VisibleHeight` do config; dimensiona `CellToInstance`.
- **Custom data (7 floats), muda em relação à arquitetura/art-bible (6):** `[0..2] = RGB`, `[3] = PatternIndex (0..6)`, `[4] = Brightness`, `[5] = Pulse`, `[6] = Alpha`. O `M_Block` lê com `PerInstanceCustomData` índices 0-6.
- **`CellToLocal(X, Y)`** = `FVector(0, X * CellSize + CellSize/2, Y * CellSize + CellSize/2)` (Y = horizontal, Z = vertical; levels §2.2). Escala da instância = `(CellSize/100) * BlockScale` para o Cube padrão de 100 uu.
- **`Sync`:**
  1. Se `Sim.GetBoardRevision() != LastBoardRevision` → `RebuildFixed` (limpa e recria até 200 instâncias **visíveis**; `Y >= VisibleHeight` não é desenhado) e zera os pulsos.
  2. `UpdateActiveAndGhost`: se `Sim.GetActive().IsValid()`, 4 instâncias em cada ISM (cria uma vez, depois `UpdateInstanceTransform` com `bMarkRenderStateDirty` só na última); células com `Y >= VisibleHeight` ficam com escala 0. Se a peça for inválida (limpeza/game over) → escala 0 em todas.
  - ⚠️ Nunca chamar `Sim.GetGhost()` com a peça inválida (a tabela de formas faz `check`).
- **`TickComponent`:** decai `InstancePulse` (`-= Dt / PulseDecayTime`), atualiza o custom data `[5]` e desliga o tick quando todos chegam a 0.
- `ClearInstances` + `AddInstances(Transforms, false)` em lote no rebuild; `MarkRenderStateDirty()` uma vez.

---

## 6. `ANSPlayerController` (`Game/NSPlayerController.h`): M1

```cpp
#pragma once
#include "GameFramework/PlayerController.h"
#include "Simulation/NSTypes.h"
#include "NSPlayerController.generated.h"

class UInputAction;
class UInputMappingContext;
class ANSBoardActor;
struct FInputActionValue;

UCLASS()
class TETRIS_API ANSPlayerController : public APlayerController
{
	GENERATED_BODY()
public:
	UPROPERTY(EditDefaultsOnly, Category="NeonStack|Input") TObjectPtr<UInputMappingContext> GameplayContext;
	UPROPERTY(EditDefaultsOnly, Category="NeonStack|Input") TObjectPtr<UInputMappingContext> UIContext;

	UPROPERTY(EditDefaultsOnly, Category="NeonStack|Input") TObjectPtr<UInputAction> IA_MoveLeft;
	UPROPERTY(EditDefaultsOnly, Category="NeonStack|Input") TObjectPtr<UInputAction> IA_MoveRight;
	UPROPERTY(EditDefaultsOnly, Category="NeonStack|Input") TObjectPtr<UInputAction> IA_SoftDrop;
	UPROPERTY(EditDefaultsOnly, Category="NeonStack|Input") TObjectPtr<UInputAction> IA_HardDrop;
	UPROPERTY(EditDefaultsOnly, Category="NeonStack|Input") TObjectPtr<UInputAction> IA_RotateCW;
	UPROPERTY(EditDefaultsOnly, Category="NeonStack|Input") TObjectPtr<UInputAction> IA_RotateCCW;
	UPROPERTY(EditDefaultsOnly, Category="NeonStack|Input") TObjectPtr<UInputAction> IA_Hold;
	UPROPERTY(EditDefaultsOnly, Category="NeonStack|Input") TObjectPtr<UInputAction> IA_Pause;

	UFUNCTION(BlueprintCallable, Category="NeonStack") void SetBoard(ANSBoardActor* InBoard) { Board = InBoard; }

	/** true = IMC de gameplay + input só de jogo; false = IMC de UI + cursor/foco de UI. */
	UFUNCTION(BlueprintCallable, Category="NeonStack") void SetGameplayInputEnabled(bool bEnabled);

protected:
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;

private:
	void BindPressRelease(class UEnhancedInputComponent* EIC, UInputAction* IA, ENSInputAction Action);
	void OnPause(const FInputActionValue& Value);

	TWeakObjectPtr<ANSBoardActor> Board;
};
```

- **`SetupInputComponent`:** `Cast<UEnhancedInputComponent>(InputComponent)`; para cada ação de jogo: `BindAction(IA, ETriggerEvent::Started, ...)` → `Board->PressAction(A)` e `ETriggerEvent::Completed` → `Board->ReleaseAction(A)`. Pausa: `Started` → `OnPause`.
  - Forma simples: `EIC->BindAction(IA, ETriggerEvent::Started, this, &ThisClass::HandlePressed, ENSInputAction::MoveLeft);` com `void HandlePressed(ENSInputAction Action)` (o Enhanced Input aceita parâmetros extras de payload). Alternativa: `BindActionValueLambda`.
- **`OnPause`:** `if (ANSGameMode* GM = GetWorld()->GetAuthGameMode<ANSGameMode>()) GM->TogglePause();`
- **`SetGameplayInputEnabled`:** `UEnhancedInputLocalPlayerSubsystem` → `ClearAllMappings()`, `AddMappingContext(bEnabled ? GameplayContext : UIContext, 0)`; `SetInputMode(FInputModeGameOnly())` ou `FInputModeGameAndUI()`; cursor oculto em ambos (navegação por teclado).
- **Input Actions (assets):** todas `Value Type = Digital (bool)`, **sem triggers** (o padrão já dispara Started/Completed). DAS/ARR estão na simulação.

---

## 7. `ANSGameMode` (`Game/NSGameMode.h`): M3 (no M1 basta chamar `StartGame` no `BeginPlay`)

```cpp
#pragma once
#include "GameFramework/GameModeBase.h"
#include "NSGameMode.generated.h"

class ANSBoardActor;

UENUM(BlueprintType)
enum class ENSFlowState : uint8 { Countdown, Playing, Paused, GameOver, Results };

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FNSOnFlowStateChanged, ENSFlowState, NewState);

UCLASS()
class TETRIS_API ANSGameMode : public AGameModeBase
{
	GENERATED_BODY()
public:
	ANSGameMode();   // PlayerControllerClass = ANSPlayerController; DefaultPawnClass = nullptr

	UPROPERTY(EditDefaultsOnly, Category="NeonStack") TSoftObjectPtr<UWorld> MainMenuLevel;
	UPROPERTY(EditDefaultsOnly, Category="NeonStack") float GameOverToResultsDelay = 1.5f;

	UFUNCTION(BlueprintPure, Category="NeonStack") ENSFlowState GetFlowState() const { return FlowState; }
	UFUNCTION(BlueprintPure, Category="NeonStack") ANSBoardActor* GetBoard() const { return Board.Get(); }

	/** Chamado pelo widget da contagem ao terminar (3-2-1). Inicia a partida ou retoma da pausa. */
	UFUNCTION(BlueprintCallable, Category="NeonStack") void FinishCountdown();
	UFUNCTION(BlueprintCallable, Category="NeonStack") void TogglePause();
	UFUNCTION(BlueprintCallable, Category="NeonStack") void PauseGame();
	UFUNCTION(BlueprintCallable, Category="NeonStack") void ResumeGame();     // → Countdown
	UFUNCTION(BlueprintCallable, Category="NeonStack") void RestartGame();    // → Countdown (nova partida)
	UFUNCTION(BlueprintCallable, Category="NeonStack") void ReturnToMenu();

	UPROPERTY(BlueprintAssignable, Category="NeonStack") FNSOnFlowStateChanged OnFlowStateChanged;

protected:
	virtual void BeginPlay() override;

private:
	void SetFlowState(ENSFlowState NewState);
	UFUNCTION() void HandleGameOver(int64 Score, int32 Lines, int32 Level, bool bNewHighScore);
	void ShowResults();

	TWeakObjectPtr<ANSBoardActor> Board;
	ENSFlowState FlowState = ENSFlowState::Countdown;
	bool bStartPending = true;     // a contagem atual é de início (true) ou de retomada (false)
	FTimerHandle ResultsTimer;
};
```

| Função | Lógica |
|--------|--------|
| `BeginPlay` | Acha o board (`TActorIterator<ANSBoardActor>`; `ensureMsgf` se não houver); `Board->OnGameOver.AddDynamic(this, &ANSGameMode::HandleGameOver)`; passa o board ao PC (`SetBoard`); `SetFlowState(Countdown)` |
| `SetFlowState` | Grava, ajusta o input do PC (`Playing` → gameplay; demais → UI), `OnFlowStateChanged.Broadcast` (o HUD/BP mostra o widget certo) |
| `FinishCountdown` | `if (bStartPending) Board->StartGame(); else Board->SetPaused(false);` → `Playing` |
| `PauseGame` | Só se `Playing`: `Board->SetPaused(true)` → `Paused` |
| `ResumeGame` | Só se `Paused`: `bStartPending = false` → `Countdown` (o board continua pausado até o `FinishCountdown`) |
| `TogglePause` | `Playing` → `PauseGame`; `Paused` → `ResumeGame` |
| `RestartGame` | `Board->StopGame(); Board->SetPaused(false); bStartPending = true` → `Countdown` |
| `HandleGameOver` | → `GameOver`; timer `GameOverToResultsDelay` → `ShowResults()` → `Results` |
| `ReturnToMenu` | `UGameplayStatics::OpenLevelBySoftObjectPtr(this, MainMenuLevel)` |

> A pausa **não** usa `SetGamePaused`: só a simulação congela (fundo e música continuam), como pede a arquitetura §7.

---

## 8. Checklist de aceite (camada de gameplay)

**M1**
- [ ] `BP_Board` no `L_Game` com `DA_GameConfig` e o Cube padrão: as peças caem, movem, giram, travam, limpam
- [ ] Segurar ← move 1, espera ~170 ms e repete; soltar e segurar → reinicia
- [ ] Input não se perde: apertar girar e hard drop no mesmo frame executa os dois em ordem

**M3**
- [ ] Recorde e opções persistem após fechar e reabrir
- [ ] Pausa congela a peça; o fundo continua; a contagem 3-2-1 retoma
- [ ] Game over → 1,5 s → resultados; "Jogar de novo" reinicia sem recarregar o mapa

---

*Spec por Priscilla, GD-KS UE5 Programmer Lead.*
