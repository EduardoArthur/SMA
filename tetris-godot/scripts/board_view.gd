class_name TetrisBoardView
extends Control

## Draws the board snapshot supplied by TetrisGame and flashes pending rows.

signal clear_flash_finished

const COLUMN_COUNT: int = 10
const ROW_COUNT: int = 20
const EMPTY_CELL: int = -1
const PIECE_COLORS: Array[Color] = [
	Color("41e7ff"), Color("ffe66d"), Color("c77dff"), Color("6cff8f"),
	Color("ff5c8a"), Color("4f8cff"), Color("ff9f43")
]

@export_range(16.0, 64.0, 1.0) var cell_size: float = 28.0
@export_range(0.05, 1.0, 0.05) var clear_flash_duration: float = 0.22

var _grid: Array[PackedInt32Array] = []
var _active_cells: Array[Vector2i] = []
var _active_piece_id: int = EMPTY_CELL
var _drop_distance: int = 0
var _rows_to_flash: Array[int] = []

@onready var _flash_timer: Timer = $ClearFlashTimer


func _ready() -> void:
	_flash_timer.one_shot = true
	_flash_timer.timeout.connect(_on_clear_flash_timeout)
	queue_redraw()


## Receives a complete immutable snapshot of the board and active piece.
func set_board_state(
	grid: Array[PackedInt32Array],
	active_cells: Array[Vector2i],
	active_piece_id: int,
	drop_distance: int
) -> void:
	_grid = grid
	_active_cells = _copy_cells(active_cells)
	_active_piece_id = active_piece_id
	_drop_distance = drop_distance
	queue_redraw()


## Starts the visible flash that precedes removal of completed rows.
func play_clear_flash(rows: Array[int]) -> void:
	_rows_to_flash = _copy_rows(rows)
	_flash_timer.start(clear_flash_duration)
	queue_redraw()


func _draw() -> void:
	var board_size: Vector2 = Vector2(COLUMN_COUNT * cell_size, ROW_COUNT * cell_size)
	var board_origin: Vector2 = (size - board_size) * 0.5
	var board_rect: Rect2 = Rect2(board_origin, board_size)
	draw_rect(board_rect.grow(15.0), Color(0.1, 0.9, 1.0, 0.1))
	draw_rect(board_rect.grow(8.0), Color("080d23"))
	draw_rect(board_rect.grow(8.0), Color("18e6ff"), false, 3.0)
	draw_rect(board_rect, Color("091225"))

	for row_index: int in range(ROW_COUNT):
		for column_index: int in range(COLUMN_COUNT):
			_draw_cell_border(board_origin, column_index, row_index)
			if row_index < _grid.size() and column_index < _grid[row_index].size():
				var piece_id: int = _grid[row_index][column_index]
				if piece_id != EMPTY_CELL:
					_draw_piece_cell(board_origin, Vector2i(column_index, row_index), piece_id)

	if _drop_distance > 0 and _active_piece_id >= 0 and _active_piece_id < PIECE_COLORS.size():
		var outline: Color = PIECE_COLORS[_active_piece_id]
		outline.a = 0.6
		for cell: Vector2i in _active_cells:
			var landing_cell: Vector2i = cell + Vector2i(0, _drop_distance)
			if _active_cells.has(landing_cell):
				continue
			var position: Vector2 = board_origin + Vector2(landing_cell) * cell_size
			var ghost_rect: Rect2 = Rect2(position, Vector2.ONE * cell_size).grow(-3.0)
			draw_rect(ghost_rect, Color(outline, 0.12))
			draw_rect(ghost_rect, outline, false, 2.0)

	for cell: Vector2i in _active_cells:
		_draw_piece_cell(board_origin, cell, _active_piece_id)

	for row_index: int in _rows_to_flash:
		var flash_rect: Rect2 = Rect2(
			board_origin + Vector2(0.0, row_index * cell_size),
			Vector2(board_size.x, cell_size)
		)
		draw_rect(flash_rect, Color(1.0, 1.0, 1.0, 0.7), true)


func _draw_cell_border(board_origin: Vector2, column_index: int, row_index: int) -> void:
	var position: Vector2 = board_origin + Vector2(column_index * cell_size, row_index * cell_size)
	draw_rect(Rect2(position, Vector2(cell_size, cell_size)), Color("162946"), false, 1.0)


func _draw_piece_cell(board_origin: Vector2, cell: Vector2i, piece_id: int) -> void:
	if piece_id < 0 or piece_id >= PIECE_COLORS.size():
		return
	var position: Vector2 = board_origin + Vector2(cell.x * cell_size, cell.y * cell_size)
	draw_block(self, position, cell_size, PIECE_COLORS[piece_id])


static func draw_block(canvas: CanvasItem, position: Vector2, block_size: float, color: Color) -> void:
	var inset: float = 2.0
	var block_rect: Rect2 = Rect2(
		position + Vector2(inset, inset),
		Vector2.ONE * (block_size - inset * 2.0)
	)
	canvas.draw_rect(block_rect, color.darkened(0.45))
	canvas.draw_rect(block_rect.grow(-3.0), color)
	canvas.draw_line(block_rect.position, Vector2(block_rect.end.x, block_rect.position.y), color.lightened(0.6), 2.0)
	canvas.draw_line(block_rect.position, Vector2(block_rect.position.x, block_rect.end.y), color.lightened(0.35), 2.0)


func _on_clear_flash_timeout() -> void:
	_rows_to_flash.clear()
	queue_redraw()
	clear_flash_finished.emit()


func _copy_cells(cells: Array[Vector2i]) -> Array[Vector2i]:
	var copied_cells: Array[Vector2i] = []
	for cell: Vector2i in cells:
		copied_cells.append(cell)
	return copied_cells


func _copy_rows(rows: Array[int]) -> Array[int]:
	var copied_rows: Array[int] = []
	for row_index: int in rows:
		copied_rows.append(row_index)
	return copied_rows
