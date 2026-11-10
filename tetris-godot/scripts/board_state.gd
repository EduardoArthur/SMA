class_name BoardState
extends RefCounted

## Holds the rules-only state for one 10×20 Tetris board.

const COLUMN_COUNT: int = 10
const ROW_COUNT: int = 20
const EMPTY_CELL: int = -1
const ACTIVE_CELL_COUNT: int = 4
const NO_PIECE_ID: int = -1
const ROTATION_OFFSETS: Array[Vector2i] = [
	Vector2i.ZERO, Vector2i.LEFT, Vector2i.RIGHT, Vector2i(-2, 0),
	Vector2i(2, 0), Vector2i.UP, Vector2i(-1, -1), Vector2i(1, -1),
	Vector2i(0, -2), Vector2i.DOWN
]

var _grid: Array[PackedInt32Array] = []
var _active_cells: Array[Vector2i] = []
var _active_piece_id: int = NO_PIECE_ID
var _game_over: bool = false


## Resets the board to an empty state and clears the active piece.
func reset() -> void:
	_grid.clear()
	for row_index: int in range(ROW_COUNT):
		_grid.append(_make_empty_row())
	_active_cells.clear()
	_active_piece_id = NO_PIECE_ID
	_game_over = false


## Places a new active piece when all of its cells fit in empty board cells.
func spawn_piece(piece_id: int, cells: Array[Vector2i]) -> bool:
	if _grid.is_empty():
		reset()
	if cells.size() != ACTIVE_CELL_COUNT or not _can_place(cells):
		_game_over = true
		return false

	_active_piece_id = piece_id
	_active_cells = _copy_cells(cells)
	_game_over = false
	return true


## Returns a copy of the fixed cells in the board grid.
func get_grid() -> Array[PackedInt32Array]:
	var grid_copy: Array[PackedInt32Array] = []
	for row_index: int in range(_grid.size()):
		grid_copy.append(_grid[row_index].duplicate())
	return grid_copy


## Returns a copy of the currently falling piece's cells.
func get_active_cells() -> Array[Vector2i]:
	return _copy_cells(_active_cells)


## Reports whether the active piece can move by the requested cell offset.
func can_move(offset: Vector2i) -> bool:
	if not _has_active_piece():
		return false
	return _can_place(_offset_cells(_active_cells, offset))


## Moves the active piece by the requested offset if the move is valid.
func try_move(offset: Vector2i) -> bool:
	if not can_move(offset):
		return false
	_active_cells = _offset_cells(_active_cells, offset)
	return true


## Rotates the active piece clockwise around its first cell if the result is valid.
func try_rotate_clockwise() -> bool:
	if not _has_active_piece():
		return false

	var pivot: Vector2i = _active_cells[0]
	var rotated_cells: Array[Vector2i] = []
	for cell: Vector2i in _active_cells:
		var relative_cell: Vector2i = cell - pivot
		rotated_cells.append(pivot + Vector2i(-relative_cell.y, relative_cell.x))

	for offset: Vector2i in ROTATION_OFFSETS:
		var candidate: Array[Vector2i] = _offset_cells(rotated_cells, offset)
		if _can_place(candidate):
			_active_cells = candidate
			return true
	return false


## Drops the active piece to its lowest valid position and returns its distance.
func hard_drop() -> int:
	var dropped_rows: int = get_drop_distance()
	if dropped_rows > 0:
		_active_cells = _offset_cells(_active_cells, Vector2i(0, dropped_rows))
	return dropped_rows


## Returns how many rows the active piece can drop without changing the board.
func get_drop_distance() -> int:
	var distance: int = 0
	while can_move(Vector2i(0, distance + 1)):
		distance += 1
	return distance


## Fixes the active piece to the grid and returns every completed row index.
func lock_active_piece() -> Array[int]:
	var completed_rows: Array[int] = []
	if not _has_active_piece():
		return completed_rows

	for cell: Vector2i in _active_cells:
		_grid[cell.y][cell.x] = _active_piece_id
	_active_cells.clear()
	_active_piece_id = NO_PIECE_ID

	for row_index: int in range(ROW_COUNT):
		if _is_row_full(row_index):
			completed_rows.append(row_index)
	return completed_rows


## Removes the selected complete rows and pulls every higher row downward.
func clear_rows(rows: Array[int]) -> void:
	if rows.is_empty():
		return

	var write_row: int = ROW_COUNT - 1
	for read_row: int in range(ROW_COUNT - 1, -1, -1):
		if rows.has(read_row):
			continue
		_grid[write_row] = _grid[read_row].duplicate()
		write_row -= 1

	while write_row >= 0:
		_grid[write_row] = _make_empty_row()
		write_row -= 1


## Reports whether the most recent attempt to spawn a piece failed.
func is_game_over() -> bool:
	return _game_over


func _make_empty_row() -> PackedInt32Array:
	var row: PackedInt32Array = PackedInt32Array()
	row.resize(COLUMN_COUNT)
	for column_index: int in range(COLUMN_COUNT):
		row[column_index] = EMPTY_CELL
	return row


func _has_active_piece() -> bool:
	return _active_piece_id != NO_PIECE_ID and _active_cells.size() == ACTIVE_CELL_COUNT


func _can_place(cells: Array[Vector2i]) -> bool:
	if cells.size() != ACTIVE_CELL_COUNT:
		return false
	for cell: Vector2i in cells:
		if not _is_inside_board(cell) or _grid[cell.y][cell.x] != EMPTY_CELL:
			return false
	return true


func _is_inside_board(cell: Vector2i) -> bool:
	return cell.x >= 0 and cell.x < COLUMN_COUNT and cell.y >= 0 and cell.y < ROW_COUNT


func _offset_cells(cells: Array[Vector2i], offset: Vector2i) -> Array[Vector2i]:
	var offset_cells: Array[Vector2i] = []
	for cell: Vector2i in cells:
		offset_cells.append(cell + offset)
	return offset_cells


func _copy_cells(cells: Array[Vector2i]) -> Array[Vector2i]:
	var copied_cells: Array[Vector2i] = []
	for cell: Vector2i in cells:
		copied_cells.append(cell)
	return copied_cells


func _is_row_full(row_index: int) -> bool:
	for column_index: int in range(COLUMN_COUNT):
		if _grid[row_index][column_index] == EMPTY_CELL:
			return false
	return true
