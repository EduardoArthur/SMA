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

	# Hedge (the deadly wall): an outlined band around the grid with a bushy edge
	# of dark half-circles (Art Bible §7). Bumps are drawn first so the band
	# covers their inner halves.
	var outer := Rect2(Vector2(-hedge, -hedge), board_px + Vector2(hedge, hedge) * 2.0)
	var bumps := _bump_centers(outer, cell * 0.5)
	var bump_r := cell * 0.25
	for p in bumps:
		draw_circle(p, bump_r + outline, Palette.OUTLINE, true, -1.0, true)
	draw_rect(outer.grow(outline), Palette.OUTLINE)
	for p in bumps:
		draw_circle(p, bump_r, Palette.HEDGE_DARK, true, -1.0, true)
	draw_rect(outer, Palette.HEDGE)

	# Lawn checkerboard.
	for y in grid_size.y:
		for x in grid_size.x:
			var color := Palette.LAWN_LIGHT if (x + y) % 2 == 0 else Palette.LAWN_DARK
			draw_rect(Rect2(Vector2(x, y) * cell, Vector2(cell, cell)), color)


## Evenly spaced points along the edge of `rect`, about `step` apart.
func _bump_centers(rect: Rect2, step: float) -> PackedVector2Array:
	var pts := PackedVector2Array()
	var corners := [rect.position, Vector2(rect.end.x, rect.position.y), rect.end, Vector2(rect.position.x, rect.end.y)]
	for i in 4:
		var a: Vector2 = corners[i]
		var b: Vector2 = corners[(i + 1) % 4]
		var count := maxi(1, roundi(a.distance_to(b) / step))
		for j in count:
			pts.append(a.lerp(b, j / float(count)))
	return pts
