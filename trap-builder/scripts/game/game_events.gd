extends Node
## Barramento global de sinais (autoload "GameEvents").
## Não guarda estado. A simulação NÃO usa este autoload: o LevelController
## repassa os sinais da Simulation para cá (godot-architecture.md §4).

# --- fases / partida ---
signal phase_changed(state: int)
signal budget_changed(spent: int, budget: int)
signal wave_finished(result: SimResult)

# --- Blueprint ---
signal trap_selected(trap: TrapData)
signal trap_placed(trap: TrapData, cell: Vector2i)
signal trap_removed(trap: TrapData, cell: Vector2i)

# --- Ação (relay da Simulation) ---
signal trap_triggered(trap_id: int, kind: StringName, cell: Vector2i)
signal pig_captured(pig_id: int, cause: StringName)
signal pig_escaped(pig_id: int, path_edges: PackedInt32Array)

# --- controles ---
signal speed_changed(mult: int)
signal paused_changed(paused: bool)
signal play_requested()
signal abort_requested()
signal retry_requested()
signal adjust_requested()
signal next_requested()
