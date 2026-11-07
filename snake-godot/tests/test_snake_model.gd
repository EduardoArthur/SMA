extends SceneTree
## Headless rule tests for SnakeModel (Technical Design §10).
## Run from the project folder:
##   godot --headless --import            (first time only: builds the class cache)
##   godot --headless -s res://tests/test_snake_model.gd
## Exit code = number of failed checks (0 = all passed).

var _failed := 0
var _checks := 0


func _init() -> void:
	_test_reset()
	_t1_reversal_ignored()
	_t2_wall_kills()
	_t3_tail_cell_is_safe()
	_t4_tail_cell_kills_when_growing()
	_t5_eat_and_grow()
	_t6_near_miss_wall()
	_t6b_near_miss_body()
	_t7_no_near_miss_in_open_space()
	_t8_eating_beats_near_miss()
	_t9_board_cleared()
	_t10_speed_ramp()
	_t11_seeded_fruit_is_repeatable()
	_test_milestone()
	_test_speed_changed_flag()

	print("")
	if _failed == 0:
		print("ALL PASSED (%d checks)" % _checks)
	else:
		print("FAILED: %d of %d checks" % [_failed, _checks])
	quit(_failed)


# --- helpers -----------------------------------------------------------------

func _check(condition: bool, label: String) -> void:
	_checks += 1
	if condition:
		print("  ok    ", label)
	else:
		_failed += 1
		print("  FAIL  ", label)


## Builds a model with a hand-placed snake. The fruit defaults to a far corner
## cell so it never interferes unless a test puts it somewhere on purpose.
func _model(grid: Vector2i, cells: Array, dir: Vector2i, fruit := Vector2i(0, 0)) -> SnakeModel:
	var m := SnakeModel.new(GameConfig.new(), 1234)
	m.grid_size = grid
	m.body.clear()
	for c in cells:
		m.body.append(c)
	m.prev_body = m.body.duplicate()
	m.direction = dir
	m.fruit = fruit
	return m


# --- tests ---------------------------------------------------------------------

func _test_reset() -> void:
	print("reset")
	var m := SnakeModel.new(GameConfig.new(), 1)
	m.reset(Vector2i(20, 15))
	_check(m.body.size() == 3, "starts with 3 segments")
	_check(m.body[0] == Vector2i(10, 7), "head at the grid center")
	_check(m.direction == Vector2i.RIGHT, "landscape heads right")
	_check(m.body[2] == Vector2i(8, 7), "body trails behind the head")
	_check(m.fruit != SnakeModel.NO_FRUIT and not m.body.has(m.fruit), "fruit spawned on a free cell")
	m.reset(Vector2i(15, 20))
	_check(m.direction == Vector2i.UP, "portrait heads up")
	_check(m.body[1] == m.body[0] + Vector2i.DOWN, "portrait body trails below the head")


func _t1_reversal_ignored() -> void:
	print("T1 reversal ignored")
	var m := _model(Vector2i(20, 15), [Vector2i(10, 7), Vector2i(9, 7), Vector2i(8, 7)], Vector2i.RIGHT)
	var r := m.step(Vector2i.LEFT)
	_check(not r.turned, "turned is false")
	_check(m.direction == Vector2i.RIGHT, "still heading right")
	_check(not r.died and m.body[0] == Vector2i(11, 7), "moved forward instead")


func _t2_wall_kills() -> void:
	print("T2 wall kills")
	var m := _model(Vector2i(20, 15), [Vector2i(19, 7), Vector2i(18, 7), Vector2i(17, 7)], Vector2i.RIGHT)
	var r := m.step(Vector2i.ZERO)
	_check(r.died, "died")
	_check(r.death_cause == "wall", "cause is wall")
	_check(m.body[0] == Vector2i(19, 7), "body did not move on death")


func _t3_tail_cell_is_safe() -> void:
	print("T3 moving into the leaving tail cell is safe")
	# Square loop: head (5,5) moving up, tail at (6,5). Turning right enters the tail cell.
	var m := _model(Vector2i(20, 15), [Vector2i(5, 5), Vector2i(5, 6), Vector2i(6, 6), Vector2i(6, 5)], Vector2i.UP)
	var r := m.step(Vector2i.RIGHT)
	_check(not r.died, "not dead")
	_check(m.body[0] == Vector2i(6, 5), "head is on the old tail cell")
	_check(m.body.size() == 4, "length unchanged")


func _t4_tail_cell_kills_when_growing() -> void:
	print("T4 tail cell kills when growing")
	var m := _model(Vector2i(20, 15), [Vector2i(5, 5), Vector2i(5, 6), Vector2i(6, 6), Vector2i(6, 5)], Vector2i.UP, Vector2i(6, 5))
	var r := m.step(Vector2i.RIGHT)
	_check(r.died, "died")
	_check(r.death_cause == "self", "cause is self")


func _t5_eat_and_grow() -> void:
	print("T5 eat and grow")
	var m := _model(Vector2i(20, 15), [Vector2i(5, 5), Vector2i(4, 5), Vector2i(3, 5)], Vector2i.RIGHT, Vector2i(6, 5))
	var r := m.step(Vector2i.ZERO)
	_check(r.ate, "ate")
	_check(m.body.size() == 4, "length is 4")
	_check(m.body[3] == Vector2i(3, 5), "tail stayed in place")
	_check(m.score == 10, "score is 10")
	_check(m.fruits_eaten == 1, "fruits_eaten is 1")
	_check(m.fruit != SnakeModel.NO_FRUIT and not m.body.has(m.fruit), "new fruit on a free cell")


func _t6_near_miss_wall() -> void:
	print("T6 near-miss: turn away from a wall")
	var m := _model(Vector2i(20, 15), [Vector2i(19, 7), Vector2i(18, 7), Vector2i(17, 7)], Vector2i.RIGHT)
	var r := m.step(Vector2i.UP)
	_check(not r.died, "not dead")
	_check(r.turned, "turned")
	_check(r.near_miss, "near_miss is true")
	_check(r.near_miss_cell == Vector2i(20, 7), "near_miss_cell is the wall cell")


func _t6b_near_miss_body() -> void:
	print("T6b near-miss: turn away from own body")
	# Head (5,5) moving right; a body segment sits at (6,5) right in front of it.
	var cells := [Vector2i(5, 5), Vector2i(4, 5), Vector2i(4, 4), Vector2i(5, 4), Vector2i(6, 4), Vector2i(6, 5), Vector2i(6, 6)]
	var m := _model(Vector2i(20, 15), cells, Vector2i.RIGHT)
	var r := m.step(Vector2i.DOWN)
	_check(not r.died, "not dead")
	_check(r.near_miss, "near_miss is true")
	_check(r.near_miss_cell == Vector2i(6, 5), "near_miss_cell is the body cell")


func _t7_no_near_miss_in_open_space() -> void:
	print("T7 no near-miss in open space")
	var m := _model(Vector2i(20, 15), [Vector2i(10, 7), Vector2i(9, 7), Vector2i(8, 7)], Vector2i.RIGHT)
	var r := m.step(Vector2i.UP)
	_check(r.turned, "turned")
	_check(not r.near_miss, "near_miss is false")


func _t8_eating_beats_near_miss() -> void:
	print("T8 eating while turning away from danger")
	var m := _model(Vector2i(20, 15), [Vector2i(19, 7), Vector2i(18, 7), Vector2i(17, 7)], Vector2i.RIGHT, Vector2i(19, 6))
	var r := m.step(Vector2i.UP)
	_check(r.ate, "ate")
	_check(not r.near_miss, "near_miss is false")


func _t9_board_cleared() -> void:
	print("T9 board cleared")
	var m := _model(Vector2i(3, 1), [Vector2i(1, 0), Vector2i(0, 0)], Vector2i.RIGHT, Vector2i(2, 0))
	var r := m.step(Vector2i.ZERO)
	_check(r.ate, "ate the last fruit")
	_check(r.board_cleared, "board_cleared is true")
	_check(m.fruit == SnakeModel.NO_FRUIT, "no fruit left")


func _t10_speed_ramp() -> void:
	print("T10 speed ramp")
	var cfg := GameConfig.new()
	_check(is_equal_approx(cfg.moves_per_sec(0), 7.0), "0 fruits -> 7.0")
	_check(is_equal_approx(cfg.moves_per_sec(2), 7.0), "2 fruits -> 7.0")
	_check(is_equal_approx(cfg.moves_per_sec(3), 7.5), "3 fruits -> 7.5")
	_check(is_equal_approx(cfg.moves_per_sec(6), 8.0), "6 fruits -> 8.0")
	_check(is_equal_approx(cfg.moves_per_sec(48), 15.0), "48 fruits -> 15.0 (cap)")
	_check(is_equal_approx(cfg.moves_per_sec(99), 15.0), "99 fruits -> 15.0 (cap)")


func _t11_seeded_fruit_is_repeatable() -> void:
	print("T11 seeded fruit sequence is repeatable")
	var a := SnakeModel.new(GameConfig.new(), 42)
	var b := SnakeModel.new(GameConfig.new(), 42)
	a.reset(Vector2i(20, 15))
	b.reset(Vector2i(20, 15))
	var same := true
	for i in 10:
		if a.fruit != b.fruit or a.fruit_type != b.fruit_type:
			same = false
		a.spawn_fruit()
		b.spawn_fruit()
	_check(same, "same seed -> same 10 fruits")


func _test_milestone() -> void:
	print("milestone at length 10")
	var cells := []
	for x in 9:
		cells.append(Vector2i(10 - x, 7)) # head (10,7) ... tail (2,7), length 9
	var m := _model(Vector2i(20, 15), cells, Vector2i.RIGHT, Vector2i(11, 7))
	var r := m.step(Vector2i.ZERO)
	_check(r.milestone == 10, "milestone is 10")


func _test_speed_changed_flag() -> void:
	print("speed_changed on the 3rd fruit only")
	var m := _model(Vector2i(20, 15), [Vector2i(5, 5), Vector2i(4, 5), Vector2i(3, 5)], Vector2i.RIGHT, Vector2i(6, 5))
	m.fruits_eaten = 1
	var r := m.step(Vector2i.ZERO)
	_check(not r.speed_changed, "2nd fruit: no speed change")
	m.fruit = m.body[0] + Vector2i.RIGHT
	r = m.step(Vector2i.ZERO)
	_check(r.speed_changed, "3rd fruit: speed changed")
	_check(is_equal_approx(m.moves_per_sec(), 7.5), "now 7.5 moves/sec")
