class_name SnakeModel
extends RefCounted
## Pure game rules for Snake (GDD §3, Technical Design §4).
## No nodes, no drawing, no input: Game calls step() once per tick and
## the views only read the public fields below.

const NO_FRUIT := Vector2i(-1, -1)
const FRUIT_TYPE_COUNT := 3 ## 0 apple, 1 cherry, 2 grapes (cosmetic only).

var config: GameConfig
var rng := RandomNumberGenerator.new()

var grid_size := Vector2i.ZERO
var body: Array[Vector2i] = [] ## body[0] is the head, the last item is the tail.
var prev_body: Array[Vector2i] = [] ## body before the last step, for interpolation.
var direction := Vector2i.RIGHT
var fruit := NO_FRUIT
var fruit_type := 0
var fruits_eaten := 0
var score := 0


## Pass a seed >= 0 for a repeatable fruit sequence (tests), or -1 for random.
func _init(cfg: GameConfig = null, seed_value: int = -1) -> void:
	config = cfg if cfg != null else GameConfig.new()
	if seed_value >= 0:
		rng.seed = seed_value
	else:
		rng.randomize()


## Starts a new run on the given grid (GDD §3.1-3.2).
func reset(size: Vector2i) -> void:
	grid_size = size
	direction = Vector2i.RIGHT if size.x >= size.y else Vector2i.UP
	body.clear()
	var head := size / 2
	for i in config.start_length:
		body.append(head - direction * i)
	prev_body = body.duplicate()
	fruits_eaten = 0
	score = 0
	spawn_fruit()


## Advances the snake by one cell. `turn` is Vector2i.ZERO when there is no input.
func step(turn: Vector2i) -> StepResult:
	var r := StepResult.new()
	prev_body = body.duplicate()

	# 1. Apply the turn: never a reversal, never the same direction.
	var prev_dir := direction
	if turn != Vector2i.ZERO and turn != direction and turn != -direction:
		direction = turn
		r.turned = true

	var head := body[0]
	var new_head := head + direction
	var growing := new_head == fruit

	# 2. Death checks (GDD §3.4). The body does not move on death,
	#    so the view can freeze the snake exactly where it crashed.
	if not _in_bounds(new_head):
		r.died = true
		r.death_cause = "wall"
		return r
	if _is_blocked_by_body(new_head, growing):
		r.died = true
		r.death_cause = "self"
		return r

	# 3. Near-miss (GDD §6.2): only on a turn, never on the same tick as eating.
	if r.turned and not growing:
		var would_have := head + prev_dir
		if not _in_bounds(would_have) or _is_blocked_by_body(would_have, false):
			r.near_miss = true
			r.near_miss_cell = would_have

	# 4. Move (and grow).
	body.push_front(new_head)
	if growing:
		r.ate = true
		fruits_eaten += 1
		score += config.points_per_fruit
		r.speed_changed = config.moves_per_sec(fruits_eaten) > config.moves_per_sec(fruits_eaten - 1)
		if body.size() in config.milestones:
			r.milestone = body.size()
		if not spawn_fruit():
			r.board_cleared = true
	else:
		body.pop_back()
	return r


## Places the fruit on a random free cell. Returns false when the board is full.
func spawn_fruit() -> bool:
	var free: Array[Vector2i] = []
	for y in grid_size.y:
		for x in grid_size.x:
			var c := Vector2i(x, y)
			if not body.has(c):
				free.append(c)
	if free.is_empty():
		fruit = NO_FRUIT
		return false
	fruit = free[rng.randi_range(0, free.size() - 1)]
	fruit_type = rng.randi_range(0, FRUIT_TYPE_COUNT - 1)
	return true


func moves_per_sec() -> float:
	return config.moves_per_sec(fruits_eaten)


func _in_bounds(c: Vector2i) -> bool:
	return c.x >= 0 and c.y >= 0 and c.x < grid_size.x and c.y < grid_size.y


## True if a body segment occupies `c`. The current tail is ignored unless the
## snake is growing, because the tail leaves its cell on the same tick.
func _is_blocked_by_body(c: Vector2i, growing: bool) -> bool:
	var last := body.size() if growing else body.size() - 1
	for i in last:
		if body[i] == c:
			return true
	return false
