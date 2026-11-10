class_name TetrisGame
extends Control

## Coordinates one Tetris match while BoardState owns the board rules.

const PIECE_I: int = 0
const PIECE_O: int = 1
const PIECE_T: int = 2
const PIECE_S: int = 3
const PIECE_Z: int = 4
const PIECE_J: int = 5
const PIECE_L: int = 6
const NO_PIECE_ID: int = -1

const INITIAL_FALL_INTERVAL: float = 1.0
const MINIMUM_FALL_INTERVAL: float = 0.2
const FALL_INTERVAL_STEP: float = 0.1
const FALL_INTERVAL_STEP_SECONDS: float = 30.0
const POINTS_BY_CLEARED_LINE_COUNT: Array[int] = [0, 100, 300, 500, 800]

signal board_changed(grid: Array[PackedInt32Array], active_cells: Array[Vector2i], active_piece_id: int, drop_distance: int)
signal score_changed(score: int)
signal high_score_changed(high_score: int)
signal hold_changed(held_piece_id: int, is_locked: bool)
signal rows_pending_clear(rows: Array[int])
signal game_over

@export_range(0.05, 1.0, 0.05) var horizontal_initial_repeat_delay: float = 0.18
@export_range(0.02, 0.5, 0.01) var horizontal_repeat_interval: float = 0.08
@export_range(0.02, 1.0, 0.01) var soft_drop_interval: float = 0.05
@export_range(0.1, 1.0, 0.05) var lock_delay: float = 0.45

@onready var _fall_timer: Timer = $FallTimer
@onready var _line_clear_sound: AudioStreamPlayer = $LineClearSound
@onready var _board_view: TetrisBoardView = $Board
@onready var _hud: TetrisHud = $HUD
@onready var _game_over_view: Control = $GameOver

var _board_state: BoardState = BoardState.new()
var _random: RandomNumberGenerator = RandomNumberGenerator.new()
var _next_piece_id: int = NO_PIECE_ID
var _active_piece_id: int = NO_PIECE_ID
var _held_piece_id: int = NO_PIECE_ID
var _is_hold_locked: bool = false
var _is_waiting_for_clear: bool = false
var _is_finished: bool = false
var _score: int = 0
var _high_score: int = 0
const SCORE_SAVE_PATH: String = "user://scores.cfg"
var _elapsed_seconds: float = 0.0
var _held_horizontal_direction: int = 0
var _horizontal_repeat_remaining: float = 0.0
var _is_soft_drop_held: bool = false
var _pending_rows: Array[int] = []
var _piece_bag: Array[int] = []
var _grounded_time: float = 0.0
var _lock_resets_remaining: int = 0


func _ready() -> void:
	_random.randomize()
	_fall_timer.one_shot = false
	if not _fall_timer.timeout.is_connected(_on_fall_timer_timeout):
		_fall_timer.timeout.connect(_on_fall_timer_timeout)
	board_changed.connect(_board_view.set_board_state)
	rows_pending_clear.connect(_board_view.play_clear_flash)
	_board_view.clear_flash_finished.connect(confirm_clear_animation)
	score_changed.connect(_hud.set_score)
	high_score_changed.connect(_hud.set_high_score)
	var scores: ConfigFile = ConfigFile.new()
	if scores.load(SCORE_SAVE_PATH) == OK:
		_high_score = maxi(0, int(scores.get_value("scores", "high_score", 0)))
	high_score_changed.emit(_high_score)
	hold_changed.connect(_update_hold_preview)
	game_over.connect(_on_game_over)
	_game_over_view.hide()
	_start_match()


func _process(delta: float) -> void:
	if _is_finished or _is_waiting_for_clear:
		return
	_elapsed_seconds += delta
	_update_fall_timer_interval()
	_process_horizontal_repeat(delta)
	if not _board_state.can_move(Vector2i.DOWN):
		_grounded_time += delta
		if _grounded_time >= lock_delay:
			_lock_active_piece()


func _notification(what: int) -> void:
	if what == NOTIFICATION_PAUSED:
		_held_horizontal_direction = 0
		_is_soft_drop_held = false


func _unhandled_key_input(event: InputEvent) -> void:
	if _is_finished or not event is InputEventKey:
		return

	var key_event: InputEventKey = event
	if key_event.echo:
		return
	if _is_waiting_for_clear:
		if not key_event.pressed:
			if key_event.keycode == KEY_DOWN:
				_is_soft_drop_held = false
			elif key_event.keycode == KEY_LEFT:
				_handle_horizontal_input(-1, false)
			elif key_event.keycode == KEY_RIGHT:
				_handle_horizontal_input(1, false)
		return
	if key_event.keycode == KEY_LEFT:
		_handle_horizontal_input(-1, key_event.pressed)
		get_viewport().set_input_as_handled()
	elif key_event.keycode == KEY_RIGHT:
		_handle_horizontal_input(1, key_event.pressed)
		get_viewport().set_input_as_handled()
	elif key_event.keycode == KEY_DOWN:
		_is_soft_drop_held = key_event.pressed
		_update_fall_timer_interval()
		get_viewport().set_input_as_handled()
	elif key_event.pressed and not key_event.echo:
		match key_event.keycode:
			KEY_UP:
				var was_grounded: bool = not _board_state.can_move(Vector2i.DOWN)
				if _active_piece_id != PIECE_O and _board_state.try_rotate_clockwise():
					_on_piece_adjusted(was_grounded)
				get_viewport().set_input_as_handled()
			KEY_SPACE:
				_board_state.hard_drop()
				_lock_active_piece()
				get_viewport().set_input_as_handled()
			KEY_C:
				_try_hold_piece()
				get_viewport().set_input_as_handled()


## Removes the completed rows after BoardView finishes its clear-flash animation.
func confirm_clear_animation() -> void:
	if not _is_waiting_for_clear:
		return

	_board_state.clear_rows(_pending_rows)
	_line_clear_sound.play()
	_add_score_for(_pending_rows.size())
	_pending_rows.clear()
	_is_waiting_for_clear = false
	_spawn_next_piece()

func _start_match() -> void:
	_board_state.reset()
	_piece_bag.clear()
	_next_piece_id = _draw_piece_id()
	_active_piece_id = NO_PIECE_ID
	_held_piece_id = NO_PIECE_ID
	_is_hold_locked = false
	_is_waiting_for_clear = false
	_is_finished = false
	_score = 0
	_elapsed_seconds = 0.0
	_held_horizontal_direction = 0
	_is_soft_drop_held = false
	_pending_rows.clear()
	score_changed.emit(_score)
	hold_changed.emit(_held_piece_id, _is_hold_locked)
	_spawn_next_piece()


func _spawn_next_piece() -> void:
	var piece_to_spawn: int = _next_piece_id
	_next_piece_id = _draw_piece_id()
	_hud.set_next(_next_piece_id, _get_spawn_cells(_next_piece_id))
	_spawn_piece(piece_to_spawn)


func _spawn_piece(piece_id: int) -> void:
	if not _board_state.spawn_piece(piece_id, _get_spawn_cells(piece_id)):
		_finish_match()
		return

	_active_piece_id = piece_id
	_grounded_time = 0.0
	_lock_resets_remaining = 5
	_update_fall_timer_interval()
	_fall_timer.start()
	_emit_board_changed()


func _on_fall_timer_timeout() -> void:
	if _board_state.try_move(Vector2i.DOWN):
		_grounded_time = 0.0
		_emit_board_changed()


func _lock_active_piece() -> void:
	_fall_timer.stop()
	_pending_rows = _board_state.lock_active_piece()
	_active_piece_id = NO_PIECE_ID
	_is_hold_locked = false
	hold_changed.emit(_held_piece_id, _is_hold_locked)
	_emit_board_changed()

	if _pending_rows.is_empty():
		_spawn_next_piece()
		return

	_is_waiting_for_clear = true
	rows_pending_clear.emit(_pending_rows.duplicate())


func _try_hold_piece() -> void:
	if _is_hold_locked or _active_piece_id == NO_PIECE_ID:
		return

	var previous_active_piece_id: int = _active_piece_id
	_is_hold_locked = true
	if _held_piece_id == NO_PIECE_ID:
		_held_piece_id = previous_active_piece_id
		_spawn_next_piece()
	else:
		var piece_to_spawn: int = _held_piece_id
		_held_piece_id = previous_active_piece_id
		_spawn_piece(piece_to_spawn)
	hold_changed.emit(_held_piece_id, _is_hold_locked)


func _handle_horizontal_input(direction: int, is_pressed: bool) -> void:
	if is_pressed:
		if _held_horizontal_direction != direction:
			_held_horizontal_direction = direction
			_horizontal_repeat_remaining = horizontal_initial_repeat_delay
			_try_move_horizontal(direction)
		return

	if _held_horizontal_direction == direction:
		_held_horizontal_direction = 0
		_horizontal_repeat_remaining = 0.0


func _process_horizontal_repeat(delta: float) -> void:
	if _held_horizontal_direction == 0:
		return

	_horizontal_repeat_remaining -= delta
	while _horizontal_repeat_remaining <= 0.0:
		_try_move_horizontal(_held_horizontal_direction)
		_horizontal_repeat_remaining += horizontal_repeat_interval


func _update_fall_timer_interval() -> void:
	if _is_finished:
		return
	var speed_steps: int = int(_elapsed_seconds / FALL_INTERVAL_STEP_SECONDS)
	var interval: float = maxf(
		MINIMUM_FALL_INTERVAL,
		INITIAL_FALL_INTERVAL - float(speed_steps) * FALL_INTERVAL_STEP
	)
	if _is_soft_drop_held:
		interval = soft_drop_interval
	if not is_equal_approx(_fall_timer.wait_time, interval):
		_fall_timer.wait_time = interval
		_fall_timer.start()


func _add_score_for(cleared_line_count: int) -> void:
	if cleared_line_count < 1 or cleared_line_count >= POINTS_BY_CLEARED_LINE_COUNT.size():
		return
	_score += POINTS_BY_CLEARED_LINE_COUNT[cleared_line_count]
	score_changed.emit(_score)
	if _score > _high_score:
		_high_score = _score
		high_score_changed.emit(_high_score)
		var scores: ConfigFile = ConfigFile.new()
		scores.set_value("scores", "high_score", _high_score)
		if scores.save(SCORE_SAVE_PATH) != OK:
			push_warning("Não foi possível salvar o recorde.")


func _emit_board_changed() -> void:
	board_changed.emit(
		_board_state.get_grid(),
		_board_state.get_active_cells(),
		_active_piece_id,
		_board_state.get_drop_distance()
	)


func _finish_match() -> void:
	_is_finished = true
	_fall_timer.stop()
	game_over.emit()


func _on_game_over() -> void:
	_game_over_view.show()
	$PauseButton.hide()


func _try_move_horizontal(direction: int) -> void:
	var was_grounded: bool = not _board_state.can_move(Vector2i.DOWN)
	if _board_state.try_move(Vector2i(direction, 0)):
		_on_piece_adjusted(was_grounded)


func _on_piece_adjusted(was_grounded: bool) -> void:
	if was_grounded and _lock_resets_remaining > 0:
		_lock_resets_remaining -= 1
		_grounded_time = 0.0
	_emit_board_changed()


func _update_hold_preview(piece_id: int, is_locked: bool) -> void:
	_hud.set_hold(piece_id, is_locked, _get_spawn_cells(piece_id))


func _draw_piece_id() -> int:
	if _piece_bag.is_empty():
		_piece_bag = [PIECE_I, PIECE_O, PIECE_T, PIECE_S, PIECE_Z, PIECE_J, PIECE_L]
		for index: int in range(_piece_bag.size() - 1, 0, -1):
			var other: int = _random.randi_range(0, index)
			var piece: int = _piece_bag[index]
			_piece_bag[index] = _piece_bag[other]
			_piece_bag[other] = piece
	return _piece_bag.pop_back()


func _get_spawn_cells(piece_id: int) -> Array[Vector2i]:
	match piece_id:
		PIECE_I:
			return [Vector2i(4, 1), Vector2i(3, 1), Vector2i(5, 1), Vector2i(6, 1)]
		PIECE_O:
			return [Vector2i(4, 0), Vector2i(5, 0), Vector2i(4, 1), Vector2i(5, 1)]
		PIECE_T:
			return [Vector2i(4, 1), Vector2i(3, 1), Vector2i(5, 1), Vector2i(4, 0)]
		PIECE_S:
			return [Vector2i(4, 1), Vector2i(3, 1), Vector2i(4, 0), Vector2i(5, 0)]
		PIECE_Z:
			return [Vector2i(4, 1), Vector2i(3, 0), Vector2i(4, 0), Vector2i(5, 1)]
		PIECE_J:
			return [Vector2i(4, 1), Vector2i(3, 1), Vector2i(5, 1), Vector2i(3, 0)]
		PIECE_L:
			return [Vector2i(4, 1), Vector2i(3, 1), Vector2i(5, 1), Vector2i(5, 0)]
	return []
