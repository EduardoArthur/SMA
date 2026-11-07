class_name SnakeView
extends Node2D
## The snake: smooth sliding (GDD §5), code-drawn art (Art Bible §3-5) and
## expressions (GDD §6.3). Only reads the model and listens to Events.

## Expression priority, low to high (GDD §6.3). "sleepy" is set by game state only.
const PRIORITY := [&"neutral", &"happy", &"proud", &"gasp", &"cool", &"faint"]
const DURATION := {&"happy": 0.4, &"gasp": 0.6, &"proud": 1.0}

@onready var game: Game = owner as Game
@onready var board: BoardView = get_parent() as BoardView

var expression: StringName = &"neutral"
var _expr_left := 0.0 ## Seconds left for a temporary expression (0 = stays).
var _locked := false ## Faint / cool / sleepy stay until the game state changes.
var _blink_in := 3.0
var _blink_left := 0.0
var _bump_t := -1.0 ## Time since death: head bump, then the body bounce (-1 = alive).
var _squash_t := -1.0 ## Time since eating, for the head squash (-1 = none).
var _rainbow_t := -1.0 ## Time since a milestone, for the rainbow wave (-1 = none).
var _time := 0.0


func _ready() -> void:
	Events.fruit_eaten.connect(_on_fruit_eaten)
	Events.near_miss.connect(func(_cell: Vector2i) -> void: _try_expression(&"gasp"))
	Events.milestone_reached.connect(_on_milestone)
	Events.died.connect(_on_died)
	Events.board_cleared.connect(func() -> void: _force(&"cool", true))
	Events.state_changed.connect(_on_state_changed)


func _process(delta: float) -> void:
	_time += delta
	if _expr_left > 0.0:
		_expr_left -= delta
		if _expr_left <= 0.0:
			_force(&"neutral", false)
	if expression == &"neutral":
		_blink_in -= delta
		if _blink_in <= 0.0:
			_blink_left = 0.1
			_blink_in = randf_range(2.0, 4.0)
	_blink_left = maxf(_blink_left - delta, 0.0)
	if _bump_t >= 0.0:
		_bump_t += delta
	if _squash_t >= 0.0:
		_squash_t = -1.0 if _squash_t > 0.4 else _squash_t + delta
	if _rainbow_t >= 0.0:
		_rainbow_t = -1.0 if _rainbow_t > 0.8 else _rainbow_t + delta
	queue_redraw()


# --- Expressions ----------------------------------------------------------------

func _try_expression(expr: StringName) -> void:
	if _locked:
		return
	# A running temporary expression is only replaced by an equal or higher one.
	if _expr_left > 0.0 and PRIORITY.find(expr) < PRIORITY.find(expression):
		return
	expression = expr
	_expr_left = DURATION.get(expr, 0.0)


func _force(expr: StringName, locked: bool) -> void:
	expression = expr
	_expr_left = 0.0
	_locked = locked


func _on_fruit_eaten(_cell: Vector2i, _type: int) -> void:
	_try_expression(&"happy")
	_squash_t = 0.0


func _on_milestone(_length: int) -> void:
	_try_expression(&"proud")
	_rainbow_t = 0.0


func _on_died(_cause: String) -> void:
	_force(&"faint", true)
	_bump_t = 0.0


func _on_state_changed(_state: int) -> void:
	match game.state:
		Game.State.TITLE, Game.State.PAUSED:
			_force(&"sleepy", true)
		Game.State.READY:
			_force(&"neutral", false)
			_bump_t = -1.0
			_squash_t = -1.0
			_rainbow_t = -1.0
		Game.State.PLAYING:
			if expression == &"sleepy":
				_force(&"neutral", false) # Waking up after a pause.
		Game.State.GAME_OVER:
			if game.last_run_new_best or game.last_run_perfect:
				_force(&"cool", true)


# --- Drawing --------------------------------------------------------------------

func _draw() -> void:
	var model := game.model
	if model == null or model.body.is_empty():
		return
	var c := board.cell
	var o := maxf(2.0, c * 0.08)
	var pts := _path_points(model, game.tick_alpha)
	var fwd := Vector2(model.direction)
	if _bump_t >= 0.0:
		# Death bump: the head pushes into the obstacle and back (0-0.2 s)...
		pts.set(0, pts[0] + fwd * c * 0.2 * sin(PI * clampf(_bump_t / 0.2, 0.0, 1.0)))
		# ...then, after the 0.3 s freeze, segments hop one after another, head to tail.
		var gap := minf(0.03, 0.6 / pts.size())
		for i in pts.size():
			var t := (_bump_t - 0.3 - i * gap) / 0.2
			if t > 0.0 and t < 1.0:
				pts.set(i, pts[i] + Vector2(0.0, -c * 0.15 * sin(PI * t)))
	var head := pts[0]

	# Body: outline pass, fill pass, then the belly highlight (Art Bible §4).
	if pts.size() >= 2:
		draw_polyline(pts, Palette.OUTLINE, c * 0.8 + o * 2.0, true)
	for p in pts:
		draw_circle(p, c * 0.4 + o, Palette.OUTLINE, true, -1.0, true)
	if pts.size() >= 2:
		draw_polyline(pts, Palette.SNAKE_BODY, c * 0.8, true)
	for p in pts:
		draw_circle(p, c * 0.4, Palette.SNAKE_BODY, true, -1.0, true)
	if pts.size() >= 2:
		var belly := PackedVector2Array()
		for p in pts:
			belly.append(p + Vector2(-0.06, -0.06) * c) # Light comes from the top-left.
		draw_polyline(belly, Palette.SNAKE_BELLY, c * 0.25, true)

	# Milestone rainbow: a band of color runs from head to tail (Art Bible §5.3).
	if _rainbow_t >= 0.0:
		var wave := _rainbow_t / 0.6 * (pts.size() + 6) - 3.0
		for i in pts.size():
			var k := clampf(1.0 - absf(i - wave) / 3.0, 0.0, 1.0)
			if k > 0.0:
				draw_circle(pts[i], c * 0.4, Color.from_hsv(fmod(i * 0.09 + _time, 1.0), 0.65, 1.0, k), true, -1.0, true)

	# Head on top (drawn inside the face transform so it can squash).
	_draw_face(head, model.direction, c * 0.5, o)


## Points along the snake from head to tail, following the grid path exactly.
## The head slides into its new cell and the tail slides out of its old one;
## every cell in between stays at its center, so corners stay on the grid.
func _path_points(model: SnakeModel, alpha: float) -> PackedVector2Array:
	var body := model.body
	var prev := model.prev_body
	var n := body.size()
	var pts := PackedVector2Array()
	var head_from := Vector2(prev[0]) if not prev.is_empty() else Vector2(body[0])
	_add_point(pts, board.cell_to_pos(head_from.lerp(Vector2(body[0]), alpha)))
	for i in range(1, n):
		_add_point(pts, board.cell_to_pos(Vector2(body[i])))
	if prev.size() >= n: # Not growing: the tail is still leaving its old cell.
		_add_point(pts, board.cell_to_pos(Vector2(prev[n - 1]).lerp(Vector2(body[n - 1]), alpha)))
	return pts


func _add_point(pts: PackedVector2Array, p: Vector2) -> void:
	if pts.is_empty() or pts[pts.size() - 1].distance_squared_to(p) > 0.01:
		pts.append(p)


## Draws the face in a local frame where +x is "forward" (Art Bible §5).
func _draw_face(head: Vector2, dir: Vector2i, r: float, o: float) -> void:
	var angle := 0.0
	var flip := Vector2.ONE
	if dir == Vector2i.LEFT:
		flip = Vector2(-1, 1) # Mirror instead of rotating, so the face is never upside-down.
	elif dir == Vector2i.UP:
		angle = -PI / 2.0
	elif dir == Vector2i.DOWN:
		angle = PI / 2.0
	# Eating squash: flattens along the heading, then wobbles back (about 0.3 s).
	var s := 0.0
	if _squash_t >= 0.0:
		s = exp(-_squash_t * 14.0) * cos(_squash_t * 28.0)
	draw_set_transform(head, angle, flip * Vector2(1.0 - 0.25 * s, 1.0 + 0.25 * s))

	draw_circle(Vector2.ZERO, r + o, Palette.OUTLINE, true, -1.0, true)
	draw_circle(Vector2.ZERO, r, Palette.SNAKE_BODY, true, -1.0, true)

	var line := r * 0.09
	var eyes: Array[Vector2] = [Vector2(0.2, -0.38) * r, Vector2(0.2, 0.38) * r]

	# Cheeks (all expressions except cool).
	if expression != &"cool":
		for e in eyes:
			draw_circle(Vector2(-0.05 * r, signf(e.y) * 0.62 * r), 0.16 * r, Color(Palette.SNAKE_CHEEK, 0.6), true, -1.0, true)

	match expression:
		&"happy":
			for e in eyes:
				draw_polyline(PackedVector2Array([e + Vector2(-0.12, -0.2) * r, e + Vector2(0.1, 0.0) * r, e + Vector2(-0.12, 0.2) * r]), Palette.PUPIL, line, true)
			draw_colored_polygon(_arc_points(Vector2(0.5 * r, 0.0), 0.28 * r, -PI / 2.0, PI / 2.0, 10), Palette.PUPIL)
			draw_circle(Vector2(0.58 * r, 0.08 * r), 0.11 * r, Palette.TONGUE, true, -1.0, true)
		&"gasp":
			for e in eyes:
				_circle_outlined(e, 0.39 * r, Palette.EYE_WHITE, o * 0.6)
				draw_circle(e, 0.08 * r, Palette.PUPIL, true, -1.0, true)
			draw_arc(Vector2(0.68 * r, 0.0), 0.12 * r, 0.0, TAU, 16, Palette.PUPIL, line, true)
			var drop := Vector2(-0.35, -0.8) * r
			draw_colored_polygon(PackedVector2Array([drop + Vector2(-0.28, 0.0) * r, drop + Vector2(0.0, -0.12) * r, drop + Vector2(0.0, 0.12) * r]), Palette.SWEAT)
			draw_circle(drop, 0.12 * r, Palette.SWEAT, true, -1.0, true)
		&"proud":
			for e in eyes:
				var star := _star_points(e, 0.34 * r, 0.14 * r)
				draw_colored_polygon(_star_points(e, 0.34 * r + o * 0.6, 0.14 * r + o * 0.6), Palette.OUTLINE)
				draw_colored_polygon(star, Palette.GOLD)
			draw_arc(Vector2(0.4 * r, 0.0), 0.32 * r, -PI / 2.6, PI / 2.6, 12, Palette.PUPIL, line, true)
		&"faint":
			for e in eyes:
				var d := 0.15 * r
				draw_line(e + Vector2(-d, -d), e + Vector2(d, d), Palette.PUPIL, line, true)
				draw_line(e + Vector2(-d, d), e + Vector2(d, -d), Palette.PUPIL, line, true)
			var wave := PackedVector2Array()
			for i in 6:
				wave.append(Vector2(0.62 * r + (0.05 * r if i % 2 == 0 else -0.05 * r), -0.25 * r + i * 0.1 * r))
			draw_polyline(wave, Palette.PUPIL, line * 0.8, true)
			_circle_outlined(Vector2(0.78, 0.3) * r, 0.13 * r, Palette.TONGUE, o * 0.6)
		&"cool":
			draw_rect(Rect2(Vector2(0.02, -0.66) * r, Vector2(0.42, 1.32) * r), Palette.PUPIL)
			draw_line(Vector2(0.12, -0.55) * r, Vector2(0.12, -0.2) * r, Color(Palette.EYE_WHITE, 0.8), line * 0.8, true)
			draw_arc(Vector2(0.42 * r, 0.12 * r), 0.22 * r, -PI / 6.0, PI / 2.4, 10, Palette.PUPIL, line, true)
		&"sleepy":
			for e in eyes:
				draw_line(e + Vector2(0.0, -0.2) * r, e + Vector2(0.0, 0.2) * r, Palette.PUPIL, line, true)
			draw_arc(Vector2(0.66 * r, 0.0), 0.07 * r, 0.0, TAU, 12, Palette.PUPIL, line * 0.7, true)
		_: # neutral (with blinking)
			for e in eyes:
				if _blink_left > 0.0:
					draw_line(e + Vector2(0.0, -0.22) * r, e + Vector2(0.0, 0.22) * r, Palette.PUPIL, line, true)
				else:
					_circle_outlined(e, 0.3 * r, Palette.EYE_WHITE, o * 0.6)
					draw_circle(e + Vector2(0.1 * r, 0.0), 0.15 * r, Palette.PUPIL, true, -1.0, true)
			draw_arc(Vector2(0.45 * r, 0.0), 0.25 * r, -PI / 3.0, PI / 3.0, 10, Palette.PUPIL, line, true)

	draw_set_transform(Vector2.ZERO, 0.0, Vector2.ONE)

	# Screen-space extras that should not rotate with the head.
	if expression == &"cool":
		_draw_crown(head + Vector2(0.0, -0.95 * r), r, o)
	elif expression == &"faint": # Three stars circling above the fainted head.
		for k in 3:
			var a := _time * 4.0 + k * TAU / 3.0
			var p := head + Vector2(cos(a) * 0.6 * r, -1.15 * r + sin(a) * 0.18 * r)
			draw_colored_polygon(_star_points(p, 0.2 * r + o * 0.6, 0.09 * r + o * 0.6), Palette.OUTLINE)
			draw_colored_polygon(_star_points(p, 0.2 * r, 0.09 * r), Palette.GOLD)
	elif expression == &"sleepy":
		var t := fmod(_time, 1.5) / 1.5
		var z := head + Vector2(0.5 * r, -1.1 * r - t * 0.5 * r)
		var s := 0.22 * r
		var col := Color(Palette.UI_TEXT, 1.0 - t)
		draw_polyline(PackedVector2Array([z + Vector2(-s, -s), z + Vector2(s, -s), z + Vector2(-s, s), z + Vector2(s, s)]), col, line, true)


func _draw_crown(base: Vector2, r: float, o: float) -> void:
	var pts := PackedVector2Array()
	for p: Vector2 in [Vector2(-0.45, 0.0), Vector2(-0.45, -0.45), Vector2(-0.22, -0.2), Vector2(0.0, -0.55), Vector2(0.22, -0.2), Vector2(0.45, -0.45), Vector2(0.45, 0.0)]:
		pts.append(base + p * r)
	draw_colored_polygon(pts, Palette.GOLD)
	pts.append(pts[0])
	draw_polyline(pts, Palette.OUTLINE, o, true)


func _circle_outlined(center: Vector2, radius: float, color: Color, o: float) -> void:
	draw_circle(center, radius + o, Palette.OUTLINE, true, -1.0, true)
	draw_circle(center, radius, color, true, -1.0, true)


func _arc_points(center: Vector2, radius: float, from: float, to: float, steps: int) -> PackedVector2Array:
	var pts := PackedVector2Array()
	for i in steps + 1:
		var a := lerpf(from, to, i / float(steps))
		pts.append(center + Vector2(cos(a), sin(a)) * radius)
	return pts


func _star_points(center: Vector2, outer: float, inner: float) -> PackedVector2Array:
	var pts := PackedVector2Array()
	for i in 8:
		var a := -PI / 2.0 + i * PI / 4.0
		pts.append(center + Vector2(cos(a), sin(a)) * (outer if i % 2 == 0 else inner))
	return pts
