class_name FruitView
extends Node2D
## Placeholder fruit for M2: an outlined circle in the fruit's color.
## M4 adds the real shapes, faces and panic animation (Art Bible §2.3, §6).

@onready var game: Game = owner as Game
@onready var board: BoardView = get_parent() as BoardView


func _process(_delta: float) -> void:
	queue_redraw()


func _draw() -> void:
	var model := game.model
	if model == null or model.fruit == SnakeModel.NO_FRUIT:
		return
	var c := board.cell
	var center := board.cell_to_pos(Vector2(model.fruit))
	var radius := c * 0.38
	draw_circle(center, radius + maxf(2.0, c * 0.08), Palette.OUTLINE)
	draw_circle(center, radius, Palette.FRUIT[model.fruit_type])
	draw_circle(center + Vector2(-0.35, -0.35) * radius, radius * 0.25, Color(1, 1, 1, 0.7))
