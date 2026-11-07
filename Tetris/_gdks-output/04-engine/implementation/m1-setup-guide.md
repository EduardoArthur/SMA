# M1 - Guia de montagem no editor (Núcleo jogável)

> **Autora:** Priscilla 💻 · **Data:** 2026-11-07
> **Objetivo do M1:** uma partida completa com cubos crus: peças caem, movem, giram, travam, limpam linhas, game over e reinício automático.
> **Código:** `Source/Tetris/` (Simulation, Data, Game, Tests). Specs: [cpp-specs/](../cpp-specs/)

⚠️ O código foi escrito **sem compilar** (não há Unreal no ambiente onde foi gerado). Espere alguns erros de compilação pequenos no primeiro build; a seção 8 lista os mais prováveis.

---

## 1. Compilar

1. Feche o editor da Unreal.
2. Botão direito em `Tetris.uproject` → **Generate Visual Studio project files** (ou, no Rider, só recarregue o projeto).
3. Abra a solução e compile **Development Editor / Win64** (no Rider: botão Build; no VS: `Ctrl+Shift+B`).
4. Abra o `Tetris.uproject`.

> Sempre que mudar um `.h` com `UCLASS`/`UPROPERTY`/`UFUNCTION`, feche o editor e recompile. Live Coding só é confiável para mudanças dentro de `.cpp`.

---

## 2. Rodar os testes primeiro

1. **Tools → Session Frontend → Automation**.
2. Filtre por `NeonStack`, marque tudo e clique em **Start Tests**.
3. Os 12 testes (T01-T12) devem ficar verdes. Se algum falhar, corrija antes de montar a cena; a lógica é a base de tudo.

---

## 3. Pastas

No Content Browser, crie `Content/NeonStack/` com: `Maps`, `Blueprints`, `Data`, `Input`, `Art/Materials`.

---

## 4. Config

1. `Data` → botão direito → **Miscellaneous → Data Asset** → escolha **NSGameConfig** → nome `DA_GameConfig`.
2. Os valores padrão já vêm do código. Para testar sempre a mesma sequência de peças, coloque `RandomSeed = 1234`.

---

## 5. Material de debug `M_Block_Debug`

Sem ele, todos os blocos ficam brancos (o material padrão ignora as cores por instância).

1. `Art/Materials` → **Material** → `M_Block_Debug`. Abra.
2. Adicione 3 nós **PerInstanceCustomData** com *Data Index* `0`, `1`, `2` → ligue num **Append** (ou dois Appends) para formar um vetor RGB.
3. Adicione **PerInstanceCustomData** índice `4` (Brightness).
4. **Base Color** = RGB × 0,3. **Emissive Color** = RGB × Brightness × 4.
5. (Opcional) PerInstanceCustomData índice `5` (Pulse) × 10 somado ao Emissive, para ver o pulso.
6. Salve. A versão neon final (`M_Block`) é do M4 (art-bible §3.2).

---

## 6. Input

Em `Input`:

1. Crie 8 **Input Actions** (botão direito → Input → Input Action), todas com **Value Type = Digital (bool)** e **sem Triggers**:
   `IA_MoveLeft`, `IA_MoveRight`, `IA_SoftDrop`, `IA_HardDrop`, `IA_RotateCW`, `IA_RotateCCW`, `IA_Hold`, `IA_Pause`.
2. Crie o **Input Mapping Context** `IMC_Gameplay` e mapeie:

| Ação | Teclas |
|------|--------|
| IA_MoveLeft | Left |
| IA_MoveRight | Right |
| IA_SoftDrop | Down |
| IA_HardDrop | Space Bar |
| IA_RotateCW | Up, X |
| IA_RotateCCW | Z |
| IA_Hold | C, Left Shift |
| IA_Pause | Escape |

> No PIE, o Esc também encerra o Play. Para testar a pausa, use **Shift+F1** antes, ou adicione uma tecla extra (ex.: P) ao `IA_Pause`.

---

## 7. Blueprints e mapa

### 7.1 Blueprints (`Blueprints/`)
1. **BP_NSPlayerController**: Blueprint Class → pai `NSPlayerController`. Em *Class Defaults*: `Gameplay Context = IMC_Gameplay` e as 8 Input Actions.
2. **BP_NeonStackGameMode**: pai `NSGameMode`. *Player Controller Class* = `BP_NSPlayerController`. *Default Pawn Class* = `None`.
3. **BP_Board**: pai `NSBoardActor`. Em *Class Defaults*: `Config = DA_GameConfig`. No componente **View**: `Block Material = M_Block_Debug` (o `Block Mesh` já vem como o Cube padrão).

### 7.2 Mapa `L_Game`
1. **File → New Level → Empty Level** → salve como `Maps/L_Game`.
2. Arraste **BP_Board** para a cena em `Location (0, 0, 0)`, `Rotation (0, 0, 0)`.
3. Adicione um **Camera Actor**:
   - Location `(-1500, 500, 1000)`, Rotation `(0, 0, 0)` (olhando para +X).
   - *Projection Mode* = **Orthographic**, *Ortho Width* = **4400**.
   - *Auto Activate for Player* = **Player 0**.
4. Adicione uma luz simples (**Directional Light**) ou deixe o material só emissivo.
5. **World Settings → GameMode Override** = `BP_NeonStackGameMode`.
6. (Opcional) **Project Settings → Maps & Modes → Editor/Game Default Map** = `L_Game`.

### 7.3 Jogar
**Play (Alt+P)**. O jogo começa sozinho; após o game over, reinicia em 2 s.

---

## 8. Erros de compilação mais prováveis

| Sintoma | Correção |
|---------|----------|
| `EAutomationTestFlags_ApplicationContextMask` não encontrado | Em `NSSimulationTests.cpp`, troque `NSTest::Flags` por `EAutomationTestFlags::EditorContext \| EAutomationTestFlags::ProductFilter` |
| `Cannot open include file "Simulation/NSTypes.h"` | Confirme o `PublicIncludePaths.Add(ModuleDirectory)` no `Tetris.Build.cs` e regenere os project files |
| Erro no `BindAction(..., Action)` (payload) | Use lambdas: `EIC->BindActionValueLambda(IA, ETriggerEvent::Started, [this, Action](const FInputActionValue&) { HandlePressed(Action); });` (incluir `InputActionValue.h`) |
| Aviso "enumerator name conflicts" para `ENSPieceType::T`/`I`/`O` | Renomear para `PieceI`, `PieceO`… só se o UHT reclamar |
| `SetCustomData` com outra assinatura | Usar `SetCustomDataValue(Index, k, Value)` num laço de 7 floats |

---

## 9. Critérios de aceite do M1

- [ ] T01-T12 verdes
- [ ] As peças nascem no topo, caem 1 linha/s no nível 1
- [ ] ←/→ movem; segurar repete após ~170 ms; ↑/X/Z giram (inclusive junto à parede); ↓ acelera; Espaço derruba e trava
- [ ] Ghost aparece abaixo da peça
- [ ] Linhas completas somem após ~0,25 s e as de cima descem
- [ ] C/Shift guarda a peça (1× por peça). Ainda não há HUD; o efeito é ver a peça trocar
- [ ] Game over quando o poço enche; reinício automático
- [ ] No Output Log (`LogNeonStack`), nenhum erro de Config/Input não atribuído

**Próximo:** M2 já está quase todo no código (saco de 7, hold, ghost, DAS/ARR). Falta só a UI para preview/hold, que entra no M3 com o HUD.
