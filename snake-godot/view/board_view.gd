class_name BoardView
extends Node2D
## Fits the grid on screen and draws the lawn and the hedge (Technical Design §7.1,
## Art Bible §7). Its position is the top-left corner of cell (0, 0), so child
## views can use cell_to_pos() in local coordinates.

const HUD_HEIGHT := 80.0 ## Screen band reserved for the score at the top.
const MARGIN := 16.0

var cell := 32.0 ## Pixels per cell. Every size in the art bible is relative to this.
var grid_size := Vector2i(20, 15)


func layout(size: Vector2i) -> void:
	grid_size = size
	var vp := get_viewport_rect().size
	var area := Rect2(Vector2(MARGIN, HUD_HEIGHT), vp - Vector2(MARGIN * 2.0, HUD_HEIGHT + MARGIN))
	# +1 cell on each axis leaves room for the hedge (half a cell on every side).
	cell = maxf(4.0, floorf(minf(area.size.x / (size.x + 1), area.size.y / (size.y + 1))))
	var board_px := Vector2(size) * cell
	position = (area.position + (area.size - board_px) / 2.0).floor()
	queue_redraw()


## Center of a grid cell, in this node's local coordinates.
func cell_to_pos(c: Vector2) -> Vector2:
	return c * cell + Vector2(cell, cell) * 0.5


func _draw() -> void:
	var board_px := Vector2(grid_size) * cell
	var hedge := cell * 0.5
	var outline := maxf(2.0, cell * 0.08)

	# Hedge (the deadly wall): an outlined band around the grid.
	var outer := Rect2(Vector2(-hedge, -hedge), board_px + Vector2(hedge, hedge) * 2.0)
	draw_rect(outer.grow(outline), Palette.OUTLINE)
	draw_rect(outer, Palette.HEDGE)

	# Lawn checkerboard.
	for y in grid_size.y:
		for x in grid_size.x:
			var color := Palette.LAWN_LIGHT if (x + y) % 2 == 0 else Palette.LAWN_DARK
			draw_rect(Rect2(Vector2(x, y) * cell, Vector2(cell, cell)), color)
