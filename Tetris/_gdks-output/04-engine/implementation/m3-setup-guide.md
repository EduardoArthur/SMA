# M3 - Guia de montagem no editor (Jogo completo)

> **Autora:** Priscilla 💻 · **Data:** 2026-11-08
> **Objetivo do M3:** fluxo completo: menu → contagem 3-2-1 → jogo com HUD (pontos, nível, linhas, recorde, hold, próximas 3) → pausa (com opções) → game over → resultados → jogar de novo/menu. Recorde e opções salvos.
> **Pré-requisito:** M1 montado e funcionando ([m1-setup-guide.md](m1-setup-guide.md)).
> ⚠️ Código escrito **sem compilar**. Mande o log de erros se houver.

---

## 1. O que entrou no código

| Arquivo | Função |
|---------|--------|
| `Data/NSSaveGame.h` | `FNSSettings` (volumes, FX, shake, DAS/ARR) + recorde |
| `Systems/NSSaveSubsystem` | Carrega/salva o slot `NeonStack_Save`; `SubmitResult` → novo recorde |
| `Game/NSBoardActor` (alterado) | Aplica DAS/ARR das opções; envia o resultado ao save no game over |
| `Game/NSGameMode` (reescrito) | Contagem → Jogando ⇄ Pausado → Game over → (1,5 s) → Resultados |
| `Game/NSMenuGameMode` | GameMode do menu principal |
| `Game/NSPlayerController` (alterado) | Cria os widgets e troca input/telas conforme o fluxo |
| `UI/NSPieceIconWidget` | Desenha uma peça (hold/preview) sem precisar de imagens |
| `UI/NSHUDWidget`, `NSCountdownWidget`, `NSPauseWidget`, `NSResultsWidget`, `NSOptionsWidget`, `NSMainMenuWidget` | Lógica das telas; o visual é feito nos WBP |
| `Tetris.Build.cs` | + `UMG`, `Slate`, `SlateCore` |

**Compile com o editor fechado** (mudaram headers e o Build.cs). Depois rode de novo os testes `NeonStack` para garantir que nada quebrou.

---

## 2. Como os WBP se ligam ao C++

Cada tela é um **Widget Blueprint** cujo *Parent Class* é a classe C++. O C++ encontra os elementos **pelo nome**: o nome do widget no designer precisa ser **idêntico** ao da tabela (e marcado como *Is Variable*, que já é o padrão para botões e textos criados no designer).

Para criar: Content Browser → botão direito → **User Interface → Widget Blueprint** → em *Pick Parent Class*, procure a classe `NS…`.

Se faltar um widget obrigatório, o WBP **não compila** e o erro diz qual nome falta. Isso é esperado e ajuda a montar.

Pasta sugerida: `Content/NeonStack/UI/`.

---

## 3. Widgets

### 3.1 `WBP_PieceIcon` (pai: `NSPieceIconWidget`)
- Designer **vazio** (a peça é desenhada por código).
- Usado dentro de um **Size Box** nas outras telas: hold e 1ª próxima com 120×60, próximas 2 e 3 com 90×45.

### 3.2 `WBP_HUD` (pai: `NSHUDWidget`)
| Nome | Tipo | Obrigatório |
|------|------|:-----------:|
| `ScoreText` | Text | ✅ |
| `LevelText` | Text | ✅ |
| `LinesText` | Text | ✅ |
| `HighScoreText` | Text | – |
| `ComboText` | Text | – |
| `HoldIcon` | WBP_PieceIcon | ✅ |
| `Next0` | WBP_PieceIcon | ✅ |
| `Next1`, `Next2` | WBP_PieceIcon | – |

Layout (GDD §7.5): **Canvas Panel**; coluna esquerda com "HOLD" + `HoldIcon`, e abaixo os rótulos PONTOS/NÍVEL/LINHAS/RECORDE com os textos; coluna direita com "NEXT" + `Next0..2`. O centro fica livre (o poço é 3D). `ComboText` perto do poço.

### 3.3 `WBP_Countdown` (pai: `NSCountdownWidget`)
| `CountText` | Text (grande, centralizado) | ✅ |
|---|---|---|

### 3.4 `WBP_Pause` (pai: `NSPauseWidget`)
| Nome | Tipo | Obrigatório |
|------|------|:-----------:|
| `ContinueButton` | Button (com Text "CONTINUAR" dentro) | ✅ |
| `RestartButton` | Button "REINICIAR" | ✅ |
| `OptionsButton` | Button "OPÇÕES" | – |
| `MenuButton` | Button "MENU" | ✅ |

**Cubra o poço** (GDD §7.3): um **Border** ou **Image** preta opaca ocupando a área central da tela, atrás dos botões, com o texto "PAUSA".

### 3.5 `WBP_Results` (pai: `NSResultsWidget`)
| Nome | Tipo | Obrigatório |
|------|------|:-----------:|
| `ScoreText`, `LinesText`, `LevelText` | Text | ✅ |
| `NewRecordText` | Text "NOVO RECORDE" (âmbar `#FFC23D`) | – |
| `PlayAgainButton` | Button "JOGAR DE NOVO" | ✅ |
| `MenuButton` | Button "MENU" | ✅ |

### 3.6 `WBP_Options` (pai: `NSOptionsWidget`)
| Nome | Tipo | Obrigatório |
|------|------|:-----------:|
| `MusicSlider`, `SfxSlider` | Slider | ✅ |
| `DasSlider`, `ArrSlider` | Slider | ✅ |
| `FxCombo` | ComboBox (String) | ✅ |
| `ShakeCheck` | CheckBox | ✅ |
| `BackButton` | Button "VOLTAR" | ✅ |
| `MusicValueText`, `SfxValueText`, `DasValueText`, `ArrValueText` | Text | – |

Os limites dos sliders e as opções do combo (Baixa/Media/Alta) são definidos pelo C++. Use um fundo opaco: a tela abre por cima da pausa e do menu.

### 3.7 `WBP_MainMenu` (pai: `NSMainMenuWidget`)
| Nome | Tipo | Obrigatório |
|------|------|:-----------:|
| `PlayButton` | Button "JOGAR" | ✅ |
| `OptionsButton` | Button "OPÇÕES" | – |
| `QuitButton` | Button "SAIR" | ✅ |
| `HighScoreText` | Text | – |

Em **Class Defaults** do `WBP_MainMenu`: `Game Level = L_Game`, `Options Class = WBP_Options`.

> Estilo neon (fontes Orbitron/Audiowide, bordas emissivas) é do M4. Por enquanto, textos e botões padrão bastam.

---

## 4. Input de UI

1. Crie o mapping context **`IMC_UI`** contendo só **`IA_Pause`** (Escape + P).
2. No **BP_NSPlayerController** → Class Defaults:
   - `UI Context = IMC_UI`
   - `HUD Class = WBP_HUD`, `Countdown Class = WBP_Countdown`, `Pause Class = WBP_Pause`, `Results Class = WBP_Results`, `Options Class = WBP_Options`
3. Navegação nos menus: **setas** movem o foco, **Enter** confirma (padrão do UMG). Nas opções, ←/→ ajustam o slider focado.

---

## 5. GameModes e mapas

1. **BP_NeonStackGameMode** → Class Defaults → `Main Menu Level = L_MainMenu`.
2. **BP_MenuGameMode**: Blueprint com pai `NSMenuGameMode`; `Main Menu Class = WBP_MainMenu`.
3. **Mapa `L_MainMenu`**: Empty Level + um Camera Actor (Auto Activate for Player 0). World Settings → GameMode Override = `BP_MenuGameMode`. (No M4 o fundo synthwave entra aqui também.)
4. **Project Settings → Maps & Modes**: *Game Default Map* = `L_MainMenu` (e *Editor Startup Map*, se quiser).

---

## 6. Critérios de aceite do M3

- [ ] Abrir o jogo cai no menu, com o recorde visível; Enter em JOGAR abre o `L_Game`
- [ ] Contagem 3-2-1 (~1,5 s) antes de a primeira peça cair
- [ ] HUD atualiza pontos/nível/linhas; hold aparece (apagado quando bloqueado); as 3 próximas batem com as peças que nascem
- [ ] "COMBO x2" aparece em limpezas seguidas e some após 1,5 s
- [ ] Esc/P pausa: o poço fica coberto, a peça congela; CONTINUAR → contagem → retoma
- [ ] OPÇÕES na pausa: mudar DAS/ARR vale imediatamente ao voltar ao jogo; os valores persistem após fechar o jogo
- [ ] Game over → 1,5 s → resultados com os números certos; "NOVO RECORDE" só quando bate o recorde
- [ ] JOGAR DE NOVO reinicia sem recarregar o mapa; MENU volta ao `L_MainMenu`
- [ ] Recorde persiste após fechar e reabrir

---

## 7. Erros prováveis

| Sintoma | Correção |
|---------|----------|
| WBP não compila: "A required widget binding X was not found" | Renomeie o widget no designer exatamente como na tabela |
| `ToPaintGeometry`/`FSlateLayoutTransform` não compila | Trocar por `AllottedGeometry.ToPaintGeometry(BoxSize, FSlateLayoutTransform(1.f, Pos))` |
| Esc não retoma da pausa | `IMC_UI` não atribuído ou sem `IA_Pause` |
| Peças do HUD não aparecem | O `WBP_PieceIcon` está com tamanho 0: coloque-o dentro de um Size Box |
| Menu não responde ao teclado | Verifique se o `PlayButton` existe (é ele que recebe o foco inicial) |
