# Trap Builder - Scene Tree Overview

> **Autor:** Guilherme 🟢 · **Versão:** 0.1 · **Data:** 2026-11-07
> Complemento de `godot-architecture.md`. Notação: `Nome (Tipo) [script] — nota`.

---

## main.tscn
```
Main (Node) [main.gd]
└── ScreenHost (Node)                  — recebe a tela ativa (MenuScreen ou LevelScreen)
```
- `main.gd`: escuta `GameState.screen_requested(scene: PackedScene, args)`, faz `queue_free` na tela atual e instancia a nova dentro de `ScreenHost`.
- Primeira tela: `MenuScreen`.

---

## screens/menu_screen.tscn
```
MenuScreen (Control) [menu_screen.gd]   — full rect
├── Background (TextureRect)            — papel do catálogo
├── Title (Label)                       — "TRAP BUILDER", Luckiest Guy
├── Logo (TextureRect)                  — carimbo Catapimba S.A.
└── LevelGrid (GridContainer)           — 5 botões gerados em runtime
    └── [LevelButton (Button) × 5]      — "1 · O Atalho ★★☆"
```
- Lê `GameState.levels` e `GameState.best_stars[level_id]`.
- Clique → `GameState.start_level(index)`.

---

## screens/level_screen.tscn
```
LevelScreen (Node2D) [level_controller.gd]
├── Board (Node2D) [board_view.gd]                    — instância de board/board.tscn
├── Wolf (Node2D) [wolf_view.gd]                      — instância de board/wolf_view.tscn; posição vem do LevelData
├── Camera2D                                          — centraliza o mapa 1152×640 na janela 1280×720
└── UI (CanvasLayer)
    ├── Hud (Control) [hud_panel.gd]                  — instância de ui/hud.tscn
    ├── Catalog (PanelContainer) [catalog_panel.gd]   — ui/catalog.tscn; visível só no BLUEPRINT
    ├── ActionBar (HBoxContainer) [action_bar.gd]     — ui/action_bar.tscn; visível só na AÇÃO
    ├── PlayButton (TextureButton)                    — visível só no BLUEPRINT
    └── ResultPanel (PanelContainer) [result_panel.gd] — ui/result_panel.tscn; visível só no RESULTADO
```
### `level_controller.gd`: responsabilidades
| Bloco | Conteúdo |
|-------|----------|
| `@export` | nenhum (recebe `LevelData` de `GameState.current_level`) |
| Estado | `enum Phase { BLUEPRINT, TRANSICAO, ACAO, RESULTADO }`, `level`, `graph: RouteGraph`, `plan: Plan`, `sim: Simulation`, `selected_trap`, `selected_dir`, `speed := 1`, `paused := false`, `_accum := 0.0` |
| `_ready()` | `graph = RouteGraph.build(level)`; `$Board.setup(level, graph)`; conecta os sinais de UI e `GameEvents`; `_enter(BLUEPRINT)` |
| `_unhandled_input()` | Só no BLUEPRINT: `place` / `rotate` / `remove` → `plan` → `GameEvents.trap_placed/removed`, `budget_changed` |
| `_process(delta)` | Só na AÇÃO: acumulador × `speed` → `while`: `sim.step()`; depois `$Board.sync_pigs(sim)` |
| Transições | `_enter(phase)` emite `GameEvents.phase_changed`; TRANSICAO espera o tween do Board e depois vai para AÇÃO |
| Relay | Conecta os sinais da `sim` aos sinais do `GameEvents` (ver o mapa de sinais) |

---

## board/board.tscn
```
Board (Node2D) [board_view.gd]
├── GroundAction (TileMapLayer)          — tile_set = data/tilesets/action.tres
├── GroundBlueprint (TileMapLayer)       — tile_set = data/tilesets/blueprint.tres; material = blueprint_wipe
├── RouteOverlay (Node2D) [route_overlay.gd]
├── Traps (Node2D)                       — filhos: TrapView instanciados por placement
├── Pigs (Node2D)                        — y_sort_enabled = true; filhos: PigView
├── FX (Node2D)                          — partículas e efeitos temporários
└── PlacementCursor (Node2D) [placement_cursor.gd]
    └── Ghost (Sprite2D)                 — modulate verde (válido) / vermelho (inválido)
```
### `board_view.gd`
| Função | Descrição |
|--------|-----------|
| `setup(level, graph)` | Gera o `GroundAction` (grama + trilha por terrain + barranco + marcado + decoração com `decor_seed`) e o `GroundBlueprint` (mesmas células, TileSet de traço) |
| `cell_to_world(cell: Vector2i) -> Vector2` / `world_to_cell(pos) -> Vector2i` | Conversão (tile de 64 px, origem no canto do mapa) |
| `set_blueprint_mode(on: bool)` | Liga/desliga camadas; `TrapView.set_blueprint_mode()` em todos os filhos de `Traps` |
| `play_transition() -> Signal` | Tween do `progress` do shader de 0→1 em 0,7 s; retorna `finished` |
| `spawn_trap_view(placement) / remove_trap_view(cell)` | Instancia ou remove o `TrapView` correspondente (`TrapData.scene`) |
| `sync_pigs(sim)` | Cria PigViews para porcos novos e atualiza a posição de todos via `sim.get_pig_render_pos(id)` |
| `reset_action()` | Remove PigViews e FX; restaura os TrapViews (bigorna volta ao estado "não caída") |

### `route_overlay.gd` (`_draw`)
- **BLUEPRINT:** arestas em `#FFD84D` (linha de 6 px, pelo centro dos tiles); spawn com número de porcos; tiles válidos para a armadilha selecionada; aresta com bigorna tracejada; seta da bifurcação controlada pela isca.
- **RESULTADO (derrota):** o caminho de cada fuga em `#D63A2F`.
- Chamar `queue_redraw()` quando chegam `trap_placed/removed`, `trap_selected`, `phase_changed` e `pig_escaped`.

---

## board/pig_view.tscn
```
PigView (Node2D) [pig_view.gd]
├── Shadow (Sprite2D)
├── Body (Sprite2D)                      — pig_walk_a/b; flip_h pela direção
└── Fx (Node2D)                          — estrelinhas, corações (isca)
```
- `@export var walk_frames: Array[Texture2D]`
- `apply_state(state)`: tweens por estado (andar = balanço vertical; LENTO = mais devagar; EMPURRADO = rotação + deslize; COMENDO = corações; CAPTURADO = squash e `queue_free`; FUGIU = pulo e some).

---

## board/wolf_view.tscn
```
WolfView (Node2D) [wolf_view.gd]
├── Bush (Sprite2D)
└── Wolf (Sprite2D)                      — troca de textura por expressão
```
- `@export var expressions: Dictionary[StringName, Texture2D]` → `watch, lick, cheer, angry, feast, sad`.
- Escuta `pig_captured`, `pig_escaped`, `wave_finished` e `phase_changed`. Fica invisível no BLUEPRINT (ou mostra a pose de "engenheiro", opcional).
- Captura múltipla: conta capturas numa janela de 1 s (usar o tick da sim via payload ou `Time.get_ticks_msec`; é só visual).

---

## traps/trap_view.tscn (base) + herdadas
```
TrapView (Node2D) [trap_view.gd]
├── ActionSprite (Sprite2D)
├── BlueprintSprite (Sprite2D)           — versão em traço branco
└── Anim (AnimationPlayer)               — "trigger", "resolve", "idle"
```
| Cena herdada | Extras |
|--------------|--------|
| `anvil_view.tscn` | `Shadow (Sprite2D)` escala 0→1 durante a sombra; `FallenSprite`; poeira |
| `tnt_view.tscn` | `Spark (GPUParticles2D/CPUParticles2D)`; `Smoke (Sprite2D ×3)` |
| `bait_view.tscn` | `SmellLines (Sprite2D)` animado enquanto ativa; `EmptyBowl` |
| `fan_view.tscn` | `Arrow (Sprite2D)` girada por `dir`; `Wind (Line2D ×3)` animadas sobre a corrente |
| `tar_view.tscn` | `Bubbles` (tween de escala) |

- `trap_view.gd`: `set_blueprint_mode(on)`, `set_dir(dir: Vector2i)`, `play_trigger()`, `play_resolve()`, `reset()`.
- **Use `CPUParticles2D`** (GL Compatibility e web).

---

## ui/hud.tscn
```
Hud (Control) [hud_panel.gd]             — anchors top-wide
├── Coins (HBoxContainer)                — ícone de moeda + Label "450 / 450"
├── Escapes (HBoxContainer)              — ícones de porco (fugas permitidas, cinza ao usar)
└── LevelTitle (Label)
```

## ui/catalog.tscn
```
Catalog (PanelContainer) [catalog_panel.gd]
└── VBox (VBoxContainer)
    ├── Header (Label)                   — "Catálogo Catapimba — Primavera"
    └── Items (VBoxContainer)            — CatalogItem por TrapData disponível
        └── CatalogItem (Button) [catalog_item.gd]   — ícone, nome, preço; disabled se não couber no orçamento
```

## ui/action_bar.tscn
```
ActionBar (HBoxContainer) [action_bar.gd]
├── PauseButton · SpeedButton ("1x"/"2x") · AbortButton
└── Counters (Label)                     — "🍖 3   💨 1"
```

## ui/result_panel.tscn
```
ResultPanel (PanelContainer) [result_panel.gd]
└── VBox
    ├── Stars (HBoxContainer)            — 3 × TextureRect (carimbo com tween)
    ├── Stats (Label)                    — "Gasto 300 / par 300 · Fugas 0 / 1"
    └── Buttons (HBoxContainer)          — Repetir · Ajustar · Próxima · Menu
```

---

## Ordem de Construção Sugerida

1. `main.tscn` + autoloads (S01)
2. Resources + `.tres` da Fase 1 (S02)
3. `board.tscn` com tilemap gerado (S03)
4. `simulation.gd` + `pig_view.tscn` + `level_screen.tscn` mínimo, sem UI (S04–S05)
5. Estados + Catalog mínimo + PlacementCursor (S06–S07)
6. `anvil_view.tscn` + paridade da Fase 1 (S08–S09)
7. Depois: o restante na ordem do backlog

---

*Árvore de cenas por Guilherme, GD-KS Godot Architect*
