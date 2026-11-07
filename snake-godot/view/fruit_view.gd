class_name FruitView
extends Node2D
## The fruit: apple / cherry / grapes drawn in code, with a tiny face, an idle
## bounce and a pop-in when it spawns (Art Bible §2.3, §6).
## The panic reaction is added in M6 (GDD F16).

@onready var game: Game = owner as Game
@onready var board: BoardView = get_parent() as BoardView

var _time := 0.0
var _spawn_t := 1.0 ## Time since the fruit appeared (for the pop-in).
var _last_fruit := Vector2i(-2, -2)


func _process(delta: float) -> void:
	_time += delta
	_spawn_t += delta
	if game.model != null and game.model.fruit != _last_fruit:
		_last_fruit = game.model.fruit
		_spawn_t = 0.0
	queue_redraw()


func _draw() -> void:
	var model := game.model
	if model == null or model.fruit == SnakeModel.NO_FRUIT:
		return
	var c := board.cell
	var o := maxf(2.0, c * 0.08)

	# Idle bounce (period 1.2 s) and pop-in (0 -> 1.15 -> 1.0 over 0.2 s).
	var wave := (sin(_time * TAU / 1.2) + 1.0) * 0.5
	var squash := Vector2(1.0 - 0.03 * wave, 1.0 + 0.06 * wave)
	var k := clampf(_spawn_t / 0.2, 0.0, 1.0)
	var pop := lerpf(0.0, 1.15, k / 0.7) if k < 0.7 else lerpf(1.15, 1.0, (k - 0.7) / 0.3)
	draw_set_transform(board.cell_to_pos(Vector2(model.fruit)), 0.0, squash * pop)

	var color: Color = Palette.FRUIT[model.fruit_type]
	match model.fruit_type:
		1: # Cherry: two small circles joined by a V of stems.
			var left := Vector2(-0.16, 0.12) * c
			var right := Vector2(0.17, 0.16) * c
			var top := Vector2(0.02, -0.32) * c
			draw_line(left, top, Palette.OUTLINE, o * 2.2, true)
			draw_line(right, top, Palette.OUTLINE, o * 2.2, true)
			draw_line(left, top, Palette.LEAF, o, true)
			draw_line(right, top, Palette.LEAF, o, true)
			_circles([left, right], 0.2 * c, color, o)
			_face(right, 0.2 * c)
		2: # Grapes: a cluster of 6 small circles in a triangle.
			var g := 0.13 * c
			var cells: Array[Vector2] = [
				Vector2(-0.24, -0.14), Vector2(0.0, -0.14), Vector2(0.24, -0.14),
				Vector2(-0.12, 0.07), Vector2(0.12, 0.07), Vector2(0.0, 0.28)]
			var centers: Array[Vector2] = []
			for p in cells:
				centers.append(p * c)
			_leaf(Vector2(0.0, -0.3) * c, c, o)
			_circles(centers, g, color, o)
			_face(Vector2(0.0, 0.07) * c, 0.2 * c)
		_: # Apple: one big circle with a stem and a leaf on top.
			var rad := 0.38 * c
			var stem_from := Vector2(0.0, -rad * 0.7)
			var stem_to := Vector2(0.03 * c, -rad * 1.25)
			draw_line(stem_from, stem_to, Palette.OUTLINE, o * 2.2, true)
			draw_line(stem_from, stem_to, Palette.STEM, o, true)
			_leaf(Vector2(0.16 * c, -rad * 1.05), c, o)
			_circles([Vector2.ZERO], rad, color, o)
			_face(Vector2.ZERO, rad)

	draw_set_transform(Vector2.ZERO, 0.0, Vector2.ONE)


## Outline pass for all circles first, then fills and shines, so overlaps look clean.
func _circles(centers: Array, radius: float, color: Color, o: float) -> void:
	for p: Vector2 in centers:
		draw_circle(p, radius + o, Palette.OUTLINE, true, -1.0, true)
	for p: Vector2 in centers:
		draw_circle(p, radius, color, true, -1.0, true)
		draw_circle(p + Vector2(-0.35, -0.35) * radius, radius * 0.25, Color(1, 1, 1, 0.7), true, -1.0, true)


func _leaf(at: Vector2, c: float, o: float) -> void:
	var size := Vector2(0.18, 0.09) * c
	var pts := PackedVector2Array([at + Vector2(-size.x, 0.0), at + Vector2(0.0, -size.y), at + Vector2(size.x, 0.0), at + Vector2(0.0, size.y)])
	draw_colored_polygon(pts, Palette.LEAF)
	pts.append(pts[0])
	draw_polyline(pts, Palette.OUTLINE, o * 0.7, true)


## Tiny happy face: two dot eyes and a small smile (Art Bible §6).
func _face(center: Vector2, radius: float) -> void:
	var eye := radius * 0.12
	draw_circle(center + Vector2(-0.3, -0.05) * radius, eye, Palette.PUPIL, true, -1.0, true)
	draw_circle(center + Vector2(0.3, -0.05) * radius, eye, Palette.PUPIL, true, -1.0, true)
	draw_arc(center + Vector2(0.0, 0.05) * radius, radius * 0.25, PI * 0.2, PI * 0.8, 8, Palette.PUPIL, maxf(1.5, radius * 0.08), true)
