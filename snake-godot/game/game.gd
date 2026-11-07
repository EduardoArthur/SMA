class_name Game
extends Node2D
## Main scene controller (Technical Design §5).
## Owns the SnakeModel, runs the state machine and the tick loop, keeps the
## input buffer, and announces everything that happens on the Events bus.
## Views and UI only read from here; they never change the game state.

enum State { TITLE, READY, PLAYING, PAUSED, DYING, GAME_OVER }

@export var config: GameConfig = preload("res://config/game_config.tres")

var model: SnakeModel
var state := State.TITLE
## Progress (0..1) between the last tick and the next one. Used for smooth sliding (M4).
var tick_alpha := 0.0
## Results of the last finished run, read by the Game Over screen.
var last_run_new_best := false
var last_run_perfect := false
## True while an overlay (Settings) is open: InputHandler ignores all input.
var input_blocked := false

var _acc := 0.0
var _interval := 1.0
var _buffer: Array[Vector2i] = []
var _state_time := 0.0
var _near_miss_cd := 0.0
var _eaten_type := 0

@onready var board: BoardView = $BoardView


func _ready() -> void:
	model = SnakeModel.new(config)
	get_viewport().size_changed.connect(_on_viewport_resized)
	_enter_state(State.TITLE)


func _process(delta: float) -> void:
	_state_time += delta
	match state:
		State.READY:
			if _state_time >= config.start_delay:
				_enter_state(State.PLAYING)
		State.PLAYING:
			_near_miss_cd = maxf(_near_miss_cd - delta, 0.0)
			_run_ticks(delta)
		State.DYING:
			if _state_time >= config.death_sequence:
				_enter_state(State.GAME_OVER)


func _notification(what: int) -> void:
	if what == NOTIFICATION_APPLICATION_FOCUS_OUT and state == State.PLAYING:
		toggle_pause()


# --- Input entry points (called by InputHandler) ------------------------------

func request_turn(dir: Vector2i) -> void:
	if state == State.TITLE or state == State.GAME_OVER:
		on_tap(true) # Arrow keys also count as "press any key".
		return
	if state == State.READY:
		_enter_state(State.PLAYING) # The first direction input starts the run.
	if state != State.PLAYING:
		return
	# Compare with the last *planned* direction, so a quick U-turn works but a
	# buffered reversal does not (Technical Design §5.3).
	var last: Vector2i = _buffer.back() if not _buffer.is_empty() else model.direction
	if dir == last or dir == -last:
		return
	if _buffer.size() < config.input_buffer_size:
		_buffer.append(dir)


## A click/tap, or any key when `from_key` is true.
func on_tap(from_key := false) -> void:
	match state:
		State.TITLE:
			_enter_state(State.READY)
		State.GAME_OVER:
			if _state_time >= config.gameover_input_lock:
				_enter_state(State.READY)
		State.PAUSED:
			if not from_key: # Keys resume only through the pause action.
				toggle_pause()


func toggle_pause() -> void:
	if state == State.PLAYING:
		_enter_state(State.PAUSED)
	elif state == State.PAUSED:
		_enter_state(State.PLAYING)


# --- State machine ------------------------------------------------------------

func _enter_state(new_state: State) -> void:
	var previous := state
	state = new_state
	_state_time = 0.0
	match new_state:
		State.TITLE, State.READY:
			_new_run()
		State.PLAYING:
			if previous == State.READY:
				Events.run_started.emit()
		State.GAME_OVER:
			_finish_run()
	Events.state_changed.emit(state)


func _new_run() -> void:
	var vp := get_viewport_rect().size
	var grid := config.grid_landscape if vp.x >= vp.y else config.grid_portrait
	model.reset(grid)
	_buffer.clear()
	_acc = 0.0
	tick_alpha = 0.0
	_interval = 1.0 / model.moves_per_sec()
	_near_miss_cd = 0.0
	last_run_new_best = false
	last_run_perfect = false
	board.layout(grid)
	Events.score_changed.emit(model.score)


func _finish_run() -> void:
	if model.score > SaveData.best:
		SaveData.best = model.score
		SaveData.save_data()
		last_run_new_best = true
		Events.new_best.emit(model.score)


# --- Tick loop (fixed timestep with accumulator, Technical Design §5.2) --------

func _run_ticks(delta: float) -> void:
	_acc += delta
	var ticks := 0
	while _acc >= _interval and ticks < 2: # Cap catch-up after a lag spike.
		_acc -= _interval
		_do_tick()
		ticks += 1
		if state != State.PLAYING:
			return
	if ticks == 2:
		_acc = minf(_acc, _interval) # Drop extra lag instead of fast-forwarding.
	tick_alpha = _acc / _interval


func _do_tick() -> void:
	_eaten_type = model.fruit_type # Remember it before step() respawns the fruit.
	var turn: Vector2i = _buffer.pop_front() if not _buffer.is_empty() else Vector2i.ZERO
	var r := model.step(turn)
	_emit_events(r)


func _emit_events(r: StepResult) -> void:
	if r.died:
		Events.died.emit(r.death_cause)
		_enter_state(State.DYING)
		return
	if r.turned:
		Events.turned.emit(model.direction)
	if r.ate:
		Events.fruit_eaten.emit(model.body[0], _eaten_type)
		Events.score_changed.emit(model.score)
	if r.near_miss and _near_miss_cd <= 0.0:
		Events.near_miss.emit(r.near_miss_cell)
		_near_miss_cd = config.near_miss_cooldown
	if r.milestone > 0:
		Events.milestone_reached.emit(r.milestone)
	if r.speed_changed:
		_interval = 1.0 / model.moves_per_sec()
		Events.speed_changed.emit(model.moves_per_sec())
	if r.board_cleared:
		last_run_perfect = true
		Events.board_cleared.emit()
		_enter_state(State.GAME_OVER)


# --- Layout -------------------------------------------------------------------

func _on_viewport_resized() -> void:
	if state == State.TITLE:
		_new_run() # Nothing is in progress: pick the grid for the new shape.
		return
	board.layout(model.grid_size)
	var vp := get_viewport_rect().size
	var wants_landscape := vp.x >= vp.y
	var is_landscape := model.grid_size.x >= model.grid_size.y
	if state == State.PLAYING and wants_landscape != is_landscape:
		toggle_pause() # The grid stays fixed until the next run (GDD §3.1).
