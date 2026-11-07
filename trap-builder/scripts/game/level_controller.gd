extends Node2D
## Controla uma fase: máquina de estados Blueprint -> Ação -> Resultado (S06),
## o Plan do jogador e a interação no Blueprint (S07), o catálogo/HUD (S10),
## o resultado (S15), o tick loop da Simulation e o relay de sinais para GameEvents.
##
## Atalhos: 1–5 seleciona · clique posiciona/remove · dir./R gira · Delete remove
##          Espaço jogar/voltar · F velocidade · P/Esc pausar · Backspace abortar
##          Tab plano de referência (dev) · 0 limpar · N/B trocar fase

enum Phase { BLUEPRINT, TRANSICAO, ACAO, RESULTADO }

const DEBUG_EXPECTED := "res://tests/expected.json"
const MAX_STEPS_PER_FRAME: int = 8
# Layout (1280×720): HUD no topo, tabuleiro reduzido à esquerda, coluna à direita.
const BOARD_POS := Vector2(16, 66)
const BOARD_SCALE: float = 0.86

var phase: Phase = Phase.BLUEPRINT
var level: LevelData
var graph: RouteGraph
var plan: Plan = Plan.new()
var sim: Simulation
var speed: int = 1
var paused: bool = false
var selected_trap: TrapData
var selected_dir: Vector2i = Vector2i.UP

var _accum: float = 0.0
var _hover_cell: Vector2i = Vector2i(-1, -1)
var _debug_refs: Array = []
var _debug_index: int = -1

@onready var board: BoardView = $Board
@onready var hud: HudPanel = $UI/Hud
@onready var catalog: CatalogPanel = $UI/Catalog
@onready var action_bar: ActionBar = $UI/ActionBar
@onready var result_panel: ResultPanel = $UI/ResultPanel
@onready var help: Label = $UI/Help


func _ready() -> void:
	level = GameState.current_level()
	if level == null:
		help.text = "Nenhuma fase carregada. Rode export_levels.js."
		return
	graph = RouteGraph.build(level)
	board.position = BOARD_POS
	board.scale = Vector2.ONE * BOARD_SCALE
	board.setup(level, graph)
	board.set_plan(plan)
	hud.set_level(level)
	catalog.setup(level.available_traps)
	catalog.trap_chosen.connect(_select_slot)
	action_bar.pause_pressed.connect(_toggle_pause)
	action_bar.speed_pressed.connect(_toggle_speed)
	GameEvents.play_requested.connect(_on_play_requested)
	GameEvents.abort_requested.connect(_on_abort_requested)
	GameEvents.retry_requested.connect(_on_retry_requested)
	GameEvents.adjust_requested.connect(_on_adjust_requested)
	GameEvents.next_requested.connect(_on_next_requested)
	_load_debug_refs()
	_enter(Phase.BLUEPRINT)
	GameEvents.budget_changed.emit(plan.cost(), level.budget)


func _process(delta: float) -> void:
	if phase == Phase.BLUEPRINT:
		_update_hover()
		return
	if phase != Phase.ACAO or paused or sim == null:
		return
	_accum += delta * speed
	var dt: float = 1.0 / SimConst.TICK_RATE
	var steps: int = 0
	while _accum >= dt and not sim.is_finished and steps < MAX_STEPS_PER_FRAME * speed:
		_accum -= dt
		sim.step()
		steps += 1
	_refresh_ui()


func _unhandled_input(event: InputEvent) -> void:
	if level == null:
		return
	if event.is_action_pressed(&"next_level"):
		_on_next_requested()
		return
	if event.is_action_pressed(&"prev_level"):
		if GameState.current_index > 0:
			GameState.start_level(GameState.current_index - 1)
		return
	match phase:
		Phase.BLUEPRINT:
			_blueprint_input(event)
		Phase.ACAO:
			_action_input(event)
		Phase.RESULTADO:
			if event.is_action_pressed(&"play"):
				_back_to_blueprint()


func _blueprint_input(event: InputEvent) -> void:
	if event.is_action_pressed(&"play"):
		_play()
		return
	for i in InputSetup.TRAP_SLOTS:
		if event.is_action_pressed(InputSetup.select_action(i)):
			_select_slot(i)
			return
	if event.is_action_pressed(&"place"):
		_click_cell(_mouse_cell())
	elif event.is_action_pressed(&"rotate"):
		_rotate_at(_mouse_cell())
	elif event.is_action_pressed(&"remove"):
		_remove_at(_mouse_cell())
	elif event.is_action_pressed(&"debug_ref_plan"):
		_cycle_debug_plan()
	elif event.is_action_pressed(&"clear_plan"):
		plan.clear()
		_debug_index = -1
		_plan_changed()
	else:
		return
	get_viewport().set_input_as_handled()


func _action_input(event: InputEvent) -> void:
	if event.is_action_pressed(&"toggle_speed"):
		_toggle_speed()
	elif event.is_action_pressed(&"pause"):
		_toggle_pause()
	elif event.is_action_pressed(&"abort"):
		_back_to_blueprint()


func _toggle_speed() -> void:
	speed = 2 if speed == 1 else 1
	GameEvents.speed_changed.emit(speed)
	_refresh_ui()


func _toggle_pause() -> void:
	if phase != Phase.ACAO:
		return
	paused = not paused
	GameEvents.paused_changed.emit(paused)
	_refresh_ui()


# --- botões (GameEvents) -----------------------------------------------------

func _on_play_requested() -> void:
	if phase == Phase.BLUEPRINT:
		_play()


func _on_abort_requested() -> void:
	if phase == Phase.ACAO:
		_back_to_blueprint()


func _on_retry_requested() -> void:
	if phase == Phase.RESULTADO:
		_back_to_blueprint()
		_play()


func _on_adjust_requested() -> void:
	if phase == Phase.RESULTADO:
		_back_to_blueprint()


func _on_next_requested() -> void:
	if GameState.has_next_level():
		GameState.start_level(GameState.current_index + 1)


# --- Blueprint: plano (S07) --------------------------------------------------

func _mouse_cell() -> Vector2i:
	return board.world_to_cell(board.get_local_mouse_position())


func _select_slot(slot: int) -> void:
	if phase != Phase.BLUEPRINT or slot >= level.available_traps.size():
		return
	var trap: TrapData = level.available_traps[slot]
	selected_trap = null if selected_trap == trap else trap
	GameEvents.trap_selected.emit(selected_trap)
	_refresh_board_cursor()
	_refresh_ui()


func _click_cell(cell: Vector2i) -> void:
	if plan.at(cell) != null:
		_remove_at(cell)
		return
	if selected_trap == null:
		return
	if not plan.can_place(graph, selected_trap, cell, level.budget):
		GameEvents.invalid_action.emit()
		return
	var dir: Vector2i = selected_dir if selected_trap.has_direction else Vector2i.RIGHT
	plan.add(selected_trap, cell, dir)
	_debug_index = -1
	GameEvents.trap_placed.emit(selected_trap, cell)
	_plan_changed()


func _remove_at(cell: Vector2i) -> void:
	var removed := plan.remove_at(cell)
	if removed == null:
		return
	_debug_index = -1
	GameEvents.trap_removed.emit(removed.trap, cell)
	_plan_changed()


func _rotate_at(cell: Vector2i) -> void:
	var placed := plan.at(cell)
	if placed != null and placed.trap.has_direction:
		placed.dir = _rotated(placed.dir)
		GameEvents.trap_rotated.emit(cell)
		_plan_changed()
	elif selected_trap != null and selected_trap.has_direction:
		selected_dir = _rotated(selected_dir)
		GameEvents.trap_rotated.emit(cell)
		_refresh_board_cursor()


## Gira 90° no sentido horário (y para baixo): direita → baixo → esquerda → cima.
func _rotated(dir: Vector2i) -> Vector2i:
	var i: int = SimConst.DIRS.find(dir)
	return SimConst.DIRS[(i + 1) % SimConst.DIRS.size()]


func _update_hover() -> void:
	var cell := _mouse_cell()
	if cell == _hover_cell:
		return
	_hover_cell = cell
	_refresh_board_cursor()


func _refresh_board_cursor() -> void:
	var valid: bool = selected_trap != null and plan.can_place(graph, selected_trap, _hover_cell, level.budget)
	board.set_cursor(selected_trap, _hover_cell, selected_dir, valid)


func _plan_changed() -> void:
	board.set_plan(plan)
	_refresh_board_cursor()
	GameEvents.budget_changed.emit(plan.cost(), level.budget)
	_refresh_ui()


# --- fases (S06) -------------------------------------------------------------

func _enter(p_phase: Phase) -> void:
	phase = p_phase
	board.set_cursor_visible(phase == Phase.BLUEPRINT)
	catalog.visible = phase == Phase.BLUEPRINT
	action_bar.visible = phase == Phase.ACAO
	if phase != Phase.RESULTADO:
		result_panel.hide()
	GameEvents.phase_changed.emit(phase)
	_refresh_ui()


func _play() -> void:
	sim = Simulation.new(level, graph, plan.duplicate_plan())
	_connect_relays(sim)
	sim.finished.connect(_on_sim_finished)
	board.attach_sim(sim)
	board.set_blueprint_mode(false)
	_accum = 0.0
	paused = false
	_enter(Phase.TRANSICAO)
	# S18: a planta azul se desfaz (0,7 s) antes de a simulação começar.
	var this_sim := sim
	await board.play_transition()
	if phase == Phase.TRANSICAO and sim == this_sim:
		_enter(Phase.ACAO)
		GameEvents.fan_count_changed.emit(_fan_count())


## Ajustar / abortar / repetir: descarta a simulação e mantém o plano.
func _back_to_blueprint() -> void:
	GameEvents.fan_count_changed.emit(0)
	board.detach_sim()
	sim = null
	paused = false
	board.set_blueprint_mode(true)
	_enter(Phase.BLUEPRINT)
	_refresh_board_cursor()


func _on_sim_finished(result: SimResult) -> void:
	GameState.record_result(level.level_id, result.stars)
	GameEvents.wave_finished.emit(result)
	_enter(Phase.RESULTADO)
	result_panel.show_result(result, level, GameState.has_next_level())
	_center_result_panel()


func _center_result_panel() -> void:
	result_panel.reset_size()
	var board_rect := Rect2(BOARD_POS, board.board_size_px() * BOARD_SCALE)
	result_panel.position = (board_rect.get_center() - result_panel.size / 2.0).floor()


func _connect_relays(s: Simulation) -> void:
	s.pig_captured.connect(GameEvents.pig_captured.emit)
	s.pig_escaped.connect(GameEvents.pig_escaped.emit)
	s.pig_spawned.connect(GameEvents.pig_spawned.emit)
	s.pig_pushed.connect(_relay_pig_pushed)
	s.pig_state_changed.connect(_relay_pig_state)
	s.trap_triggered.connect(_relay_trap_triggered)
	s.trap_resolved.connect(_relay_trap_resolved)


func _relay_trap_triggered(trap_id: int) -> void:
	if sim == null:
		return
	var t: TrapState = sim.traps[trap_id]
	GameEvents.trap_triggered.emit(trap_id, t.kind, t.cell)


func _relay_trap_resolved(trap_id: int) -> void:
	if sim == null:
		return
	var t: TrapState = sim.traps[trap_id]
	GameEvents.trap_resolved.emit(trap_id, t.kind, t.cell)


func _relay_pig_pushed(pig_id: int, _to_cell: Vector2i) -> void:
	GameEvents.pig_pushed.emit(pig_id)


func _relay_pig_state(pig_id: int, state: int) -> void:
	if state == PigState.State.BACK:
		GameEvents.pig_turned_back.emit(pig_id)


func _fan_count() -> int:
	var n: int = 0
	for p in plan.placements:
		if p.trap.id == &"fan":
			n += 1
	return n


# --- planos de referência (atalho de desenvolvimento) ------------------------

func _load_debug_refs() -> void:
	_debug_refs.clear()
	var text := FileAccess.get_file_as_string(DEBUG_EXPECTED)
	var data: Variant = JSON.parse_string(text) if not text.is_empty() else null
	if typeof(data) != TYPE_DICTIONARY:
		return
	for lv: Dictionary in data.get("levels", []):
		if int(lv.level_id) == level.level_id:
			for ref: Dictionary in lv.refs:
				if not (ref.plan as Array).is_empty():
					_debug_refs.append(ref)


func _cycle_debug_plan() -> void:
	if _debug_refs.is_empty():
		return
	_debug_index = (_debug_index + 1) % _debug_refs.size()
	var ref: Dictionary = _debug_refs[_debug_index]
	plan.clear()
	for p: Dictionary in ref.plan:
		var trap := load("res://data/traps/%s.tres" % p.type) as TrapData
		plan.add(trap, Vector2i(int(p.cell[0]), int(p.cell[1])), Vector2i(int(p.dir[0]), int(p.dir[1])))
	_plan_changed()


# --- UI ----------------------------------------------------------------------

func _refresh_ui() -> void:
	if level == null:
		return
	hud.set_budget(plan.cost(), level.budget, level.par)
	if sim != null:
		hud.set_counts(true, sim.captured, sim.escaped, sim.pigs.size())
	else:
		hud.set_counts(false)
	var status := ""
	if phase == Phase.ACAO:
		status = "PAUSADO" if paused else ("%dx" % speed)
	if _debug_index >= 0 and _debug_index < _debug_refs.size():
		status += "  plano ref.: %s" % _debug_refs[_debug_index].name
	hud.set_status(status)
	catalog.refresh(plan.cost(), level.budget, selected_trap)
	action_bar.refresh(paused, speed)
	match phase:
		Phase.BLUEPRINT:
			help.text = "Selecione no catálogo (1–5) · clique posiciona/remove · botão direito/R gira o ventilador · Espaço joga · N/B troca de fase"
		Phase.ACAO:
			help.text = "Assista à emboscada! F velocidade · P pausar · Backspace abortar"
		_:
			help.text = "Espaço volta ao Blueprint com o mesmo plano · N/B troca de fase"
