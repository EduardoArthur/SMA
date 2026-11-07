class_name SnakeView
extends Node2D
## Placeholder snake for M2: outlined squares that snap from cell to cell.
## M4 replaces this with the smooth, rounded, expressive snake (Art Bible §4-5).

@onready var game: Game = owner as Game
@onready var board: BoardView = get_parent() as BoardView


func _process(_delta: float) -> void:
	queue_redraw()


func _draw() -> void:
	var model := game.model
	if model == null or model.body.is_empty():
		return
	var c := board.cell
	var outline := maxf(2.0, c * 0.08)
	var size := Vector2(c, c) * 0.86

	# Tail first, so the head is drawn on top.
	for i in range(model.body.size() - 1, -1, -1):
		var center := board.cell_to_pos(Vector2(model.body[i]))
		var rect := Rect2(center - size / 2.0, size)
		draw_rect(rect.grow(outline), Palette.OUTLINE)
		draw_rect(rect, Palette.SNAKE_BODY if i > 0 else Palette.SNAKE_BELLY)

	# Two eyes on the head, looking where the snake is going.
	var head := board.cell_to_pos(Vector2(model.body[0]))
	var fwd := Vector2(model.direction)
	var side := Vector2(-fwd.y, fwd.x)
	for s: float in [-1.0, 1.0]:
		var eye: Vector2 = head + fwd * c * 0.15 + side * c * 0.2 * s
		draw_circle(eye, c * 0.14, Palette.EYE_WHITE)
		draw_circle(eye + fwd * c * 0.05, c * 0.07, Palette.PUPIL)
