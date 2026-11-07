# Trap Builder - Godot 4 Architecture

> **Autor:** Guilherme 🟢 (Godot Architect) · **Versão:** 0.1 · **Data:** 2026-11-07
> **Engine:** Godot 4.7 · GL Compatibility · GDScript tipado
> **Entradas:** `02-design/design-handoff.md` (backlog S01–S22 no lugar de sprint-plan/stories), `technical-design.md`, `levels/tools/sim.js`
> **Complemento:** `scene-tree-overview.md` (árvore detalhada de cada cena)

---

## 1. Princípios da Arquitetura

| # | Princípio | Por quê |
|---|-----------|---------|
| A1 | **Simulação = código puro (`RefCounted`), sem Nodes** | Determinismo, testes headless, port 1:1 do `sim.js` |
| A2 | **Visual só lê, nunca escreve** na simulação | Os sprites interpolam; nenhuma lógica depende de posição de sprite |
| A3 | **Uma fonte de verdade por dado** | Fases: `levels.js` → exportado para `.tres`. Tilemap **gerado** a partir do `LevelData`, nunca pintado à mão |
| A4 | **Sinais para desacoplar**: Simulation → LevelController → `GameEvents` → UI/Áudio/Lobo | A simulação não conhece o autoload e continua testável |
| A5 | **Poucos autoloads** (3), só para estado de fato global | Evita "God objects" |
| A6 | **Dados em Resources** com `@export` | Balanceamento no Inspector, sem tocar em código |

```
┌───────────────── CAMADA DE DADOS (Resources .tres) ─────────────────┐
│ LevelData · TrapData · PigData · RouteNode · RouteEdge · SpawnEntry  │
└──────────────┬───────────────────────────────────────────────────────┘
               │ lido por
┌──────────────▼──────────────── CAMADA DE SIMULAÇÃO (RefCounted) ─────┐
│ Simulation ── RouteGraph ── ForkHash · Plan · PigState · TrapState    │
│   step() inteiro, 60 Hz · emite sinais próprios                       │
└──────────────┬───────────────────────────────────────────────────────┘
               │ sinais + leitura de estado
┌──────────────▼──────────────── CAMADA DE CENA (Nodes) ───────────────┐
│ LevelController (máquina de estados, tick loop) → GameEvents          │
│ BoardView · PigView · TrapView · WolfView · UI · AudioDirector         │
└──────────────────────────────────────────────────────────────────────┘
```

---

## 2. Autoloads

| Nome | Script | Responsabilidade | NÃO faz |
|------|--------|------------------|---------|
| `GameEvents` | `scripts/game/game_events.gd` | Barramento de sinais globais (§4) | Não guarda estado |
| `GameState` | `scripts/game/game_state.gd` | Fase atual, lista de `LevelData`, melhor estrela por fase (em memória), troca de tela | Não roda simulação |
| `AudioDirector` | `scripts/audio/audio_director.gd` | Escuta `GameEvents` e toca SFX e música; RNG próprio de pitch | Não é chamado pela simulação |

> **Não são autoloads:** `SimConst` e `ForkHash` são `class_name` com `const` e `static func`. Não precisam de instância.

---

## 3. Árvore de Cenas (visão geral)

```
Main (Node)                          main.tscn — raiz, troca a tela ativa
└── [tela ativa]
    ├── MenuScreen (Control)         menu_screen.tscn — título + seleção de fase + estrelas
    └── LevelScreen (Node2D)         level_screen.tscn — script: LevelController
        ├── Board (Node2D)           BoardView
        │   ├── GroundAction (TileMapLayer)
        │   ├── GroundBlueprint (TileMapLayer)
        │   ├── RouteOverlay (Node2D)        _draw(): rotas, prévia, tiles válidos
        │   ├── Traps (Node2D)               instâncias de TrapView
        │   ├── Pigs (Node2D)                instâncias de PigView (y_sort)
        │   ├── FX (Node2D)
        │   └── PlacementCursor (Node2D)
        ├── Wolf (Node2D)            WolfView
        ├── Camera2D
        └── UI (CanvasLayer)
            ├── HUD · Catalog · ActionBar · ResultPanel
```
Detalhes de cada cena, nós e scripts em `scene-tree-overview.md`.

---

## 4. Mapa de Sinais

### 4.1 Sinais da `Simulation` (locais, RefCounted)
| Sinal | Payload |
|-------|---------|
| `pig_spawned(pig_id: int)` | — |
| `pig_state_changed(pig_id: int, state: int)` | `PigState.State` |
| `pig_pushed(pig_id: int, to_cell: Vector2i)` | — |
| `pig_captured(pig_id: int, cause: StringName)` | `&"anvil" \| &"tnt" \| &"ravine"` |
| `pig_escaped(pig_id: int, path_edges: PackedInt32Array)` | Arestas percorridas (para o destaque) |
| `trap_triggered(trap_id: int)` | — |
| `trap_resolved(trap_id: int)` | Bigorna caiu / TNT explodiu / isca acabou |
| `edge_blocked(edge_id: int)` | — |
| `finished(result: SimResult)` | capturados, fugas, ticks, estrelas |

### 4.2 `GameEvents` (global)
| Sinal | Emissor | Consumidores |
|-------|---------|--------------|
| `phase_changed(state: int)` | LevelController | UI, BoardView, AudioDirector, WolfView |
| `budget_changed(spent: int, budget: int)` | LevelController | HUD, Catalog |
| `trap_selected(trap: TrapData)` | Catalog | LevelController, PlacementCursor |
| `trap_placed(trap: TrapData, cell: Vector2i)` | LevelController | AudioDirector, RouteOverlay |
| `trap_removed(trap: TrapData, cell: Vector2i)` | LevelController | AudioDirector, RouteOverlay |
| `trap_triggered(trap_id: int, kind: StringName, cell: Vector2i)` | LevelController (relay) | AudioDirector, FX |
| `pig_captured(pig_id: int, cause: StringName)` | LevelController (relay) | **WolfView**, HUD, AudioDirector |
| `pig_escaped(pig_id: int, path_edges: PackedInt32Array)` | LevelController (relay) | **WolfView**, HUD, AudioDirector, RouteOverlay |
| `wave_finished(result: SimResult)` | LevelController | ResultPanel, GameState, AudioDirector, WolfView |
| `speed_changed(mult: int)` / `paused_changed(paused: bool)` | ActionBar | LevelController |
| `play_requested()` / `abort_requested()` / `retry_requested()` / `adjust_requested()` / `next_requested()` | UI | LevelController / GameState |

**Regra:** um sinal de `Simulation` nunca vai direto para a UI. O `LevelController` faz o relay, assim a simulação continua sem dependências.

---

## 5. Hierarquia de Scripts

### 5.1 Dados (`extends Resource`)
```
TrapData     id, display_name, cost, valid_tiles(flags), has_direction, persistent, icon, scene, params
PigData      id, speed(int/tick), immune_anvil, scene
RouteNode    id, cell: Vector2i, kind: Kind {SPAWN, FORK, JOIN, EXIT}
RouteEdge    id, from_id, to_id, via: Array[Vector2i]          ← cantos, igual ao levels.js
SpawnEntry   pig: PigData
LevelData    level_id, title, seed, budget, par, escape_limit,
             nodes: Array[RouteNode], edges: Array[RouteEdge], spawns: Array[SpawnEntry],
             ravine: Array[Vector2i], marked: Array[Vector2i], decor_seed,
             available_traps: Array[TrapData]
```
> As arestas guardam só os **cantos** (`via`), como no `levels.js`. O `RouteGraph` expande para células.

### 5.2 Simulação (`extends RefCounted`, sem Nodes) — port do `sim.js`
```
SimConst      (class_name, só const)     TICK_RATE, TILE_UNITS, SPAWN_INTERVAL, SHADOW, CLIMB, FUSE, EAT, PUSH, FAN_RANGE, RAVINE_CAP
ForkHash      (static func)              fork_hash(), choose_edge()
RouteGraph                               build(level) → células por aresta, cell_index, tipos de tile, outs ordenados
Plan                                     placements: Array[Placement]; cost(); can_place(trap, cell); add/remove
  Placement                              trap: TrapData, cell: Vector2i, dir: Vector2i
PigState                                 id, data, loc (node|edge+idx), sub, state: State enum, timer, pushed_by
TrapState                                id, data, cell, dir, triggered, fallen, consumed, timer, victim, eater, current
Simulation                               _init(level, graph, plan); step(); is_finished(); result
                                         get_pig_render_pos(id) -> Vector2 (float, só para o visual)
SimResult                                captured, escaped, ticks, cost, stars, escape_paths
```
**Comportamento das armadilhas:** `match trap.data.id` dentro da `Simulation`, como no `sim.js`, em vez de uma classe por armadilha.
> *Por quê, se eu prefiro composição?* A simulação é um **motor de regras** que precisa de paridade linha a linha com a referência. Polimorfismo espalharia a ordem de resolução entre arquivos e dificultaria a auditoria. No **visual**, as armadilhas usam composição normalmente (5.3).

### 5.3 Cena (`extends Node*`)
```
LevelController (Node2D)   máquina de estados, Plan, tick loop com acumulador, relay de sinais, input do Blueprint
BoardView (Node2D)         gera os tilemaps a partir do LevelData; cell↔world; entrega o RouteOverlay
RouteOverlay (Node2D)      _draw(): rotas (Blueprint), tiles válidos, arestas tracejadas, caminho da fuga
PigView (Node2D)           lê get_pig_render_pos; tweens por estado (andar, piche, empurrão, captura)
TrapView (Node2D)          base: set_blueprint_mode(bool), play_trigger(), play_resolve()
  ├── AnvilView · TntView · BaitView · FanView · TarView   (cenas herdadas, só visual/FX)
WolfView (Node2D)          troca de expressão por eventos (prioridade + janela de 60 ticks para a captura múltipla)
PlacementCursor (Node2D)   fantasma da armadilha selecionada; verde/vermelho
UI: HudPanel, CatalogPanel, CatalogItem, ActionBar, ResultPanel, MenuScreen
```

---

## 6. Fluxo de Dados de Uma Partida

```
Blueprint: input → LevelController.plan.add() → GameEvents.trap_placed → visual/áudio
JOGAR:     LevelController cria Simulation(level, graph, plan.duplicate())
           → phase_changed(TRANSICAO) → tween de 0,7 s → phase_changed(ACAO)
Ação:      _process(delta): acumulador × speed → while: sim.step()
           PigView lê sim (interpola); sinais da sim → relay → GameEvents
Fim:       sim.finished → wave_finished(result) → ResultPanel; GameState grava a melhor estrela
Ajustar:   descarta a Simulation; o Plan continua lá (D013 / plano preservado)
```
**Interpolação:** `get_pig_render_pos()` devolve `cell + direção × sub / TILE_UNITS` em float. O PigView converte para pixels com `BoardView.cell_to_world()`. Com 60 Hz de simulação, nem é preciso interpolar entre ticks no protótipo.

---

## 7. Pipeline de Fases (uma fonte de verdade)

```
levels.js ──(node validate.js)──► OK? ──(node export_levels.js)──► res://data/levels/level_0N.tres
                                                                   └► res://tests/expected.json
```
- **`export_levels.js`** (a criar, story S02/S16): converte cada fase do `levels.js` em `.tres` (texto) e grava o resultado esperado de cada referência em `expected.json`.
- **Tilemaps gerados em runtime** pelo `BoardView`:
  - Trilha: `GroundAction.set_cells_terrain_connect(route_cells, 0, TERRAIN_TRAIL)`. O autotile das curvas sai de graça.
  - Barranco e marcado: `set_cell` com o atlas correspondente.
  - Decoração (árvores e arbustos): células não-rota escolhidas com um RNG de `decor_seed`. É só visual e não interfere na simulação.
  - O Blueprint usa as mesmas células com o TileSet de traço branco.
- **Resultado:** você **nunca pinta uma fase à mão**, e o mapa visual não tem como divergir do que a simulação usa.

---

## 8. Testes (headless)

```
res://tests/run_tests.gd    (extends SceneTree)
  godot --headless -s res://tests/run_tests.gd
```
| Teste | O que verifica |
|-------|----------------|
| `test_fork_hash` | Valores fixos iguais aos do JS (T4) |
| `test_parity` | Para cada fase e referência em `expected.json`: capturados, fugas, ticks e estrelas idênticos (T1) |
| `test_speed_independence` | O resultado não depende de quantos `step()` rodam por frame (T2) |
| `test_anvil_isolated` | Escalar sem deadlock (T5) |

> A **paridade de ticks** com o JS é o teste mais forte: se o tick final bater, a ordem de processamento está certa.

---

## 9. Input Map

| Ação | Teclas / mouse |
|------|----------------|
| `place` | Botão esquerdo |
| `rotate` | Botão direito, `R` |
| `remove` | `Delete`, `Backspace`, clicar numa armadilha posicionada com a mesma selecionada |
| `play` | `Espaço`, `Enter` |
| `toggle_speed` | `F` |
| `pause` | `P`, `Esc` |
| `abort` | `Backspace` (durante a Ação) |

> No mobile (depois): toque = `place`; segurar = `rotate`; arrastar para fora = `remove`. Usar `InputEventMouseButton`, que funciona com toque emulado, já prepara isso.

---

## 10. Estrutura de Pastas (refina o §9 da spec técnica)

```
res://
├── art/            tiles/ pigs/ wolf/ traps/ ui/ fx/
├── audio/          music/ sfx/ ui/
├── data/
│   ├── traps/      anvil.tres tnt.tres bait.tres fan.tres tar.tres
│   ├── pigs/       pig.tres pig_helmet.tres
│   ├── levels/     level_01.tres … level_05.tres        ← gerados
│   └── tilesets/   action.tres blueprint.tres
├── scenes/
│   ├── main.tscn
│   ├── screens/    menu_screen.tscn level_screen.tscn
│   ├── board/      board.tscn pig_view.tscn wolf_view.tscn placement_cursor.tscn
│   ├── traps/      trap_view.tscn anvil_view.tscn tnt_view.tscn bait_view.tscn fan_view.tscn tar_view.tscn
│   └── ui/         hud.tscn catalog.tscn catalog_item.tscn action_bar.tscn result_panel.tscn
├── scripts/
│   ├── resources/  trap_data.gd pig_data.gd level_data.gd route_node.gd route_edge.gd spawn_entry.gd
│   ├── sim/        sim_const.gd fork_hash.gd route_graph.gd plan.gd placement.gd
│   │               pig_state.gd trap_state.gd simulation.gd sim_result.gd
│   ├── game/       game_events.gd game_state.gd level_controller.gd main.gd
│   ├── board/      board_view.gd route_overlay.gd pig_view.gd trap_view.gd wolf_view.gd placement_cursor.gd
│   ├── ui/         hud_panel.gd catalog_panel.gd catalog_item.gd action_bar.gd result_panel.gd menu_screen.gd
│   └── audio/      audio_director.gd
├── shaders/        blueprint_wipe.gdshader
└── tests/          run_tests.gd expected.json
```

---

## 11. Configuração do Projeto (`project.godot`)

| Chave | Valor |
|-------|-------|
| `display/window/size/viewport_width × height` | 1280 × 720 |
| `display/window/stretch/mode` / `aspect` | `canvas_items` / `expand` (já estão) |
| `rendering/textures/canvas_textures/default_texture_filter` | Linear |
| `application/run/main_scene` | `res://scenes/main.tscn` |
| Autoloads | `GameEvents`, `GameState`, `AudioDirector` (nessa ordem) |
| Audio buses | Master, Music (−10 dB), SFX (−4 dB), UI (−6 dB) |
| `debug/gdscript/warnings/untyped_declaration` | Warn (força a tipagem) |

---

## 12. Mapeamento Backlog → Arquitetura

| Story | Arquivos principais |
|-------|---------------------|
| S01 | `project.godot`, `game_events.gd`, `game_state.gd`, `main.tscn` |
| S02 | `scripts/resources/*`, `export_levels.js`, `data/` |
| S03 | `board_view.gd`, `data/tilesets/*` |
| S04 | `route_graph.gd`, `fork_hash.gd`, `tests/run_tests.gd` |
| S05 | `simulation.gd`, `pig_state.gd`, `pig_view.gd`, `level_controller.gd` (tick loop) |
| S06–S07 | `level_controller.gd` (estados, input), `plan.gd`, `placement_cursor.gd`, `route_overlay.gd` |
| S08–S09 | `simulation.gd` (bigorna), `trap_state.gd`, `anvil_view.tscn`, `test_parity` |
| S10–S15 | `catalog_*`, `simulation.gd` (TNT/isca/ventilador/piche), `*_view.tscn`, `result_panel` |
| S16 | `export_levels.js` (todas as fases), `test_parity` |
| S17–S22 | `wolf_view`, `blueprint_wipe.gdshader`, `route_overlay`, `audio_director`, art, export |

---

## 13. Riscos de Arquitetura

| Risco | Mitigação |
|-------|-----------|
| `int` do GDScript é 64 bits com sinal; o hash usa `& 0xFFFFFFFF` | Os valores cabem (produto máximo < 2⁵⁹); validado pelo `test_fork_hash` |
| `set_cells_terrain_connect` exige terrains configurados no TileSet | Configurar 1 terrain set "Trilha" com *match sides*; fallback: 6 tiles manuais por vizinhança |
| Divisão inteira: `/` entre ints no GDScript já é inteira (cuidado com floats) | Tipar tudo como `int`; usar `@warning_ignore("integer_division")` onde for intencional |
| Ordem de iteração de `Dictionary` | A simulação itera **Arrays ordenados por id**, nunca dicionários |

---

*Arquitetura por Guilherme, GD-KS Godot Architect*
*Próximo: Gina (`*scene-specs`, `*resource-design`) e Gabi (`*class-specs-godot`) — ou começar a implementar S01–S05*
