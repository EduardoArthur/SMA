# Neon Stack - UE5 Architecture

> **Autor:** Ulysses (UE5 Architect) 🏗️
> **Data:** 2026-11-07 · **Versão:** 1.0
> **Engine:** Unreal Engine 5.8 (projeto `Tetris.uproject`, módulo C++ `Tetris`)
> **Entrada:** [design-handoff.md](../02-design/design-handoff.md) · [core-mechanics.md](../02-design/core-design/mechanics/core-mechanics.md) · [levels.md](../02-design/level-design/levels.md)
> **Próximo:** Priscilla, especificação das classes C++ (`*classspec`)

---

## 1. Princípios da arquitetura

1. **Simulação pura em C++, sem UObject:** as regras do jogo vivem em structs/classes C++ comuns (`FNS…`), sem dependência de rendering, tempo do mundo ou input. São determinísticas (seed) e testáveis por Automation Tests.
2. **Actors finos:** os Actors só alimentam a simulação (tempo + input) e **traduzem eventos** em delegates para os Blueprints.
3. **O visual nunca escreve no estado** (pilar do design). Blueprints só escutam.
4. **Dados fora do código:** todo número 🔧 do design vem de DataAssets.
5. **Simples para iniciante:** sem GAS, sem subsistemas exóticos, sem multiplayer. Uma classe = uma responsabilidade.

### Decisões descartadas
| Opção | Por que não |
|-------|-------------|
| GAS (Gameplay Ability System) | Não há habilidades, atributos ou efeitos; seria complexidade gratuita |
| Lógica em UObject/Actor | Mais difícil de testar; mistura regra com engine |
| Multiplayer/replicação | Fora do escopo (anti-pilar) |
| Lógica em Blueprint | Decisão do usuário: C++ para a lógica |

---

## 2. Visão em camadas

```
┌──────────────────────────────────────────────────────────────────────┐
│ BLUEPRINTS (conteúdo, juice)                                         │
│  BP_NeonStackGameMode · BP_Board · BP_FXManager · BP_AudioDirector   │
│  BP_Background · WBP_HUD / MainMenu / Pause / Options / Results      │
└───────────────▲───────────────────────────────────────▲──────────────┘
                │ delegates (BlueprintAssignable)        │ BlueprintCallable
┌───────────────┴───────────────────────────────────────┴──────────────┐
│ C++ GAMEPLAY (UObjects/Actors finos)                                  │
│  ANSGameMode · ANSPlayerController · ANSBoardActor                    │
│  UNSBoardViewComponent · UNSSaveSubsystem · UNSGameConfig (DataAsset) │
└───────────────▲──────────────────────────────────────────────────────┘
                │ Tick(dt, input) → TArray<FNSGameEvent>
┌───────────────┴──────────────────────────────────────────────────────┐
│ C++ SIMULAÇÃO PURA (sem UObject)                                      │
│  FNSGameSimulation ─┬─ FNSBoardGrid                                   │
│                     ├─ FNSActivePiece + FNSPieceTables (formas/kicks) │
│                     ├─ FNSRandomizer (saco de 7, FRandomStream)       │
│                     ├─ FNSScoring (pontos, combo, nível, gravidade)   │
│                     └─ FNSInputState / DAS-ARR                        │
└──────────────────────────────────────────────────────────────────────┘
```

---

## 3. Classes

### 3.1 Simulação pura (`Source/Tetris/Simulation/`)

| Classe | Tipo | Responsabilidade | Spec de design |
|--------|------|------------------|----------------|
| `ENSPieceType` | `UENUM` (uint8) | None, I, O, T, S, Z, J, L | core §3 |
| `FNSPieceTables` | static | Formas pré-calculadas (7 × 4 rotações × 4 células), spawn, ordem de kicks | core §3-4 |
| `FNSBoardGrid` | struct | Grade 10×22 (`TArray<uint8>`), `IsCellFree`, `CanPlace`, `Lock`, `FindFullRows`, `ClearRows` | core §2, §8 |
| `FNSActivePiece` | struct | Tipo, rotação, posição; `GetCells()` | core §3 |
| `FNSRandomizer` | class | Saco de 7 com `FRandomStream`, fila ≥ preview+1 | core §7.1 |
| `FNSScoring` | struct | Score, linhas, nível, combo, `GravityInterval()`, `OnLinesCleared(k)` | core §8.2, progressão §3 |
| `FNSInputState` | struct | Teclas seguradas (←, →, ↓) + ações pontuais (girar, hard drop, hold) | core §5 |
| `FNSGameEvent` | `USTRUCT` | Tipo do evento + payload (células, linhas, pontos…) | core §10 |
| `FNSGameSimulation` | class | Máquina de estados; `Tick(float Dt, const FNSInputState&)` → eventos; `GetSnapshot()` | core §6, §9 |

**Regras da simulação:**
- Tudo em **tempo acumulado** (segundos), nunca em frames.
- `Dt` limitado a **0,1 s** por tick (evita saltos após travadas/alt-tab).
- Toda mudança de estado gera um `FNSGameEvent` na lista de saída do tick.
- Configuração recebida por **cópia** de um struct `FNSSimConfig` (montado a partir do DataAsset), para a simulação não depender de UObject.

### 3.2 Gameplay (`Source/Tetris/Game/`)

| Classe | Base | Responsabilidade |
|--------|------|------------------|
| `ANSBoardActor` | `AActor` | Possui `FNSGameSimulation`; no `Tick` monta o input, roda a simulação, faz broadcast dos eventos nos delegates; expõe `StartGame`, `SetPaused`, `Restart`, `GetSnapshot` |
| `UNSBoardViewComponent` | `USceneComponent` | 3 `UInstancedStaticMeshComponent` (fixos, ativa, ghost); sincroniza instâncias e `PerInstanceCustomData` (cor, padrão, pulso, alpha) a partir do snapshot. Mesh/material configuráveis no BP |
| `ANSPlayerController` | `APlayerController` | Enhanced Input (IMC + Input Actions); repassa pressionar/soltar ao `ANSBoardActor`; navegação de UI |
| `ANSGameMode` | `AGameModeBase` | Fluxo da partida: pronto → jogando → pausado → resultados; liga o board, a UI e o save |
| `UNSGameConfig` | `UPrimaryDataAsset` | Todos os 🔧 de core-mechanics §11; `ToSimConfig()` |
| `UNSSaveGame` | `USaveGame` | Recorde (score, linhas, nível, data) + opções |
| `UNSSaveSubsystem` | `UGameInstanceSubsystem` | Carrega/salva o slot `NeonStack_Save`; `SubmitResult()` → `bNewHighScore`; get/set de opções; delegate `OnSettingsChanged` |

### 3.3 Blueprints (`Content/NeonStack/`)

| Asset | Pai | Responsabilidade |
|-------|-----|------------------|
| `BP_NeonStackGameMode` | `ANSGameMode` | Classes de widget, referências de config |
| `BP_Board` | `ANSBoardActor` | Mesh/material dos blocos; liga delegates ao FX e ao áudio |
| `BP_FXManager` | `AActor` | Niagara, camera shakes, flash, pontos flutuantes (art-bible §6) |
| `BP_AudioDirector` | `AActor` | MetaSound da música + SFX afinados (audio-bible §7) |
| `BP_Background` | `AActor` | Planos do céu, sol e chão; paleta e rolagem (levels §2-3) |
| `WBP_*` | `UUserWidget` | HUD, menu, pausa, opções, resultados |

---

## 4. Fluxo de dados por frame

```
ANSPlayerController (Enhanced Input)
   │ Started/Completed → BoardActor->PressAction / ReleaseAction
   ▼
ANSBoardActor::Tick(Dt)
   │ 1. Input = InputState acumulado (ações pontuais são consumidas)
   │ 2. Events = Simulation.Tick(min(Dt, 0.1), Input)
   │ 3. ViewComponent->Sync(Simulation.GetSnapshot())   (só se houve evento)
   │ 4. for each Event: Broadcast no delegate correspondente
   ▼
BP_FXManager / BP_AudioDirector / WBP_HUD (reagem)
```

- **Ordem:** o input é processado **antes** da gravidade no mesmo tick (pilar 1: a resposta acontece no mesmo frame).
- O `ANSBoardActor` usa `PrimaryActorTick.TickGroup = TG_PrePhysics`.
- Ações pontuais (girar, hard drop, hold) são **enfileiradas** quando o input chega e consumidas no próximo tick, então nenhum toque é perdido entre frames.

---

## 5. Delegates (contrato C++ → BP)

Declarados em `ANSBoardActor` como `DECLARE_DYNAMIC_MULTICAST_DELEGATE_*`:

| Delegate | Parâmetros |
|----------|-----------|
| `OnPieceSpawned` | `ENSPieceType Type, const TArray<FIntPoint>& Cells` |
| `OnPieceMoved` | `const TArray<FIntPoint>& Cells, const TArray<FIntPoint>& Ghost, bool bByGravity` |
| `OnPieceRotated` | `const TArray<FIntPoint>& Cells, const TArray<FIntPoint>& Ghost, int32 KickIndex` |
| `OnPieceHardDropped` | `int32 FromY, int32 ToY, const TArray<FIntPoint>& Cells` |
| `OnPieceLocked` | `ENSPieceType Type, const TArray<FIntPoint>& Cells` |
| `OnLinesCleared` | `const TArray<int32>& Rows, int32 Count, int32 Combo, int32 ScoreGained` |
| `OnScoreChanged` | `int64 Score, int32 Lines, int32 Level` |
| `OnLevelUp` | `int32 NewLevel` |
| `OnHoldChanged` | `ENSPieceType Held, bool bCanHold` |
| `OnQueueChanged` | `const TArray<ENSPieceType>& Next` |
| `OnLockWarning` | `float Remaining` *(últimos 0,2 s do lock delay)* |
| `OnPausedChanged` | `bool bPaused` |
| `OnGameOver` | `int64 Score, int32 Lines, int32 Level, bool bNewHighScore` |

> Nota: `int64` é suportado em Blueprint (Integer64); a Priscilla pode optar por `int32` na spec se simplificar o HUD.

---

## 6. Input (Enhanced Input)

| Input Action | Tipo | Teclas | Gatilho |
|--------------|------|--------|---------|
| `IA_MoveLeft` / `IA_MoveRight` | Digital (bool) | ← / → | Started + Completed (o DAS fica na simulação) |
| `IA_SoftDrop` | Digital | ↓ | Started + Completed |
| `IA_HardDrop` | Digital | Espaço | Started |
| `IA_RotateCW` | Digital | ↑, X | Started |
| `IA_RotateCCW` | Digital | Z | Started |
| `IA_Hold` | Digital | C, Shift esq. | Started |
| `IA_Pause` | Digital | Esc | Started |

- `IMC_Gameplay` ativo durante a partida; `IMC_UI` nos menus (setas, Enter, Esc).
- **Sem** Triggers de "Hold/Pulse" do Enhanced Input para movimento: o DAS/ARR é da simulação (configurável e testável).

---

## 7. Fluxo de jogo (ANSGameMode)

```
L_MainMenu ──Jogar──► L_Game: Ready (contagem 3-2-1) → Playing ⇄ Paused
                                         │
                                     GameOver → Results ──Jogar de novo──► Ready
                                                         └─Menu──► L_MainMenu
```

| Estado | Board | Input | UI |
|--------|-------|-------|----|
| Ready | criado, parado | UI | contagem |
| Playing | Tick ativo | Gameplay | HUD |
| Paused | `SetPaused(true)` (simulação congela; o mundo não, então o fundo continua rolando) | UI | WBP_Pause cobre o poço |
| GameOver | parado | nenhum (1,5 s de animação) | – |
| Results | parado | UI | WBP_Results |

- **Pausa ≠ `SetGamePaused`:** pausar a simulação mantém o fundo/música vivos (audio-bible §3.2, art-bible §8.3).
- Reiniciar = `Board->Restart()` (nova seed, estado zerado) sem recarregar o mapa.

---

## 8. Persistência

- `UNSSaveSubsystem::Initialize` → `LoadGameFromSlot("NeonStack_Save", 0)` ou cria um novo.
- Salva em: fim de partida (`SubmitResult`) e ao fechar as opções.
- `bNewHighScore` é calculado no subsystem e repassado ao `OnGameOver`.
- As opções de áudio são aplicadas pelo `BP_AudioDirector` ouvindo `OnSettingsChanged`; DAS/ARR são aplicados no `ANSBoardActor` (sobrescrevem o padrão do config).

---

## 9. Estrutura de pastas

```
Source/Tetris/
├── Tetris.Build.cs  (+ "Niagara" opcional; "UMG" se houver widget C++)
├── Simulation/      NSTypes.h, NSPieceTables.h/.cpp, NSBoardGrid.h/.cpp,
│                    NSRandomizer.h/.cpp, NSScoring.h/.cpp, NSGameSimulation.h/.cpp
├── Game/            NSBoardActor, NSBoardViewComponent, NSPlayerController, NSGameMode
├── Data/            NSGameConfig, NSSaveGame
├── Systems/         NSSaveSubsystem
└── Tests/           NSSimulationTests.cpp  (WITH_DEV_AUTOMATION_TESTS)

Content/NeonStack/
├── Maps/      L_MainMenu, L_Game
├── Blueprints/ BP_NeonStackGameMode, BP_Board, BP_FXManager, BP_AudioDirector, BP_Background
├── Data/      DA_GameConfig, DA_Palettes, DA_Audio
├── Input/     IA_*, IMC_Gameplay, IMC_UI
├── Art/       Meshes/, Materials/, Textures/, FX/, PostProcess/
├── Audio/     Music/, SFX/, MetaSounds/
└── UI/        WBP_*, Fonts/
```

- Prefixo de código **`NS`** (Neon Stack); o módulo continua `Tetris` (renomear módulo no UE é arriscado e desnecessário).

---

## 10. Testes

- **Automation Tests** (Session Frontend → Automation, ou `-ExecCmds="Automation RunTests NeonStack"`).
- Um teste por caso de core-mechanics §13 (T1-T12), rodando `FNSGameSimulation` com **seed fixa** e input roteirizado, sem mundo/Actor.
- Helper `FNSSimTestDriver`: aplica uma sequência de inputs e avança o tempo em passos fixos (ex.: 1/60 s).

---

## 11. Ordem de implementação (marcos do design-handoff)

| Marco | Classes |
|-------|---------|
| M1 Núcleo | `NSTypes`, `FNSPieceTables`, `FNSBoardGrid`, `FNSActivePiece`, `FNSGameSimulation` (sem hold/preview), `ANSBoardActor`, `UNSBoardViewComponent`, `ANSPlayerController`, testes T1-T6, T9, T10 |
| M2 Controle | `FNSRandomizer`, hold, ghost, DAS/ARR em `FNSInputState`, testes T7, T8 |
| M3 Jogo completo | `FNSScoring`, `UNSGameConfig`, `UNSSaveGame`, `UNSSaveSubsystem`, `ANSGameMode`, widgets, testes T11, T12 |
| M4 Neon & juice | Só Blueprints/conteúdo (FX, áudio, materiais) |
| M5 Música | Só MetaSound + `BP_AudioDirector` |

---

## 12. Riscos técnicos

| Risco | Mitigação |
|-------|-----------|
| Iniciante em C++/UE (compilação, Live Coding, reflexão) | Simulação sem macros UE (só C++ comum); classes UE pequenas; fechar o editor ao mudar headers com `UPROPERTY` |
| `PerInstanceCustomData` mal configurado | `NumCustomDataFloats = 7` no ISM; material lendo `PerInstanceCustomData[0..6]` (cpp-specs/02-gameplay §5) |
| Input perdido entre frames | Fila de ações pontuais (§4) |
| Hitch no Tick | Clamp de Dt + simulação O(220 células), custo desprezível |

---

*Arquitetura por Ulysses, GD-KS UE5 Architect.*
