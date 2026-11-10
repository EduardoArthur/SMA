class_name TetrisHud
extends Control

## Displays score and hold state supplied by TetrisGame.

const PREVIEW_CELL: float = 18.0
var _held_id: int = -1
var _next_id: int = -1
var _held_cells: Array[Vector2i] = []
var _next_cells: Array[Vector2i] = []
var _hold_locked: bool = false

@onready var _score_label: Label = $ScoreLabel
@onready var _high_score_label: Label = $HighScoreLabel
@onready var _hold_label: Label = $HoldLabel
@onready var _hold_status_label: Label = $HoldStatusLabel


## Updates the displayed score.
func set_score(score: int) -> void:
	_score_label.text = "PONTOS\n%06d" % score


func set_high_score(high_score: int) -> void:
	_high_score_label.text = "RECORDE\n%06d" % high_score


## Updates the held piece label and whether holding is currently allowed.
func set_hold(held_piece_id: int, is_locked: bool, cells: Array[Vector2i]) -> void:
	_held_id = held_piece_id
	_held_cells = cells
	_hold_locked = is_locked
	_hold_label.text = "RESERVA"
	_hold_status_label.text = "BLOQUEADA" if is_locked else "C para trocar"
	_hold_status_label.modulate = Color("ff5c8a") if is_locked else Color("75f7ff")
	queue_redraw()


func set_next(piece_id: int, cells: Array[Vector2i]) -> void:
	_next_id = piece_id
	_next_cells = cells
	queue_redraw()


func _draw() -> void:
	_draw_preview(Vector2(78, 185), _held_id, _held_cells, _hold_locked)
	_draw_preview(Vector2(78, 315), _next_id, _next_cells, false)


func _draw_preview(center: Vector2, piece_id: int, cells: Array[Vector2i], dimmed: bool) -> void:
	var panel: Rect2 = Rect2(center - Vector2(78, 26), Vector2(156, 52))
	draw_rect(panel, Color("11132c"))
	draw_rect(panel, Color("614171"), false, 1.0)
	if cells.is_empty():
		draw_line(center - Vector2(9, 0), center + Vector2(9, 0), Color("614171"), 2.0)
		return
	var low: Vector2i = cells[0]
	var high: Vector2i = cells[0]
	for cell: Vector2i in cells:
		low = low.min(cell)
		high = high.max(cell)
	var origin: Vector2 = center - Vector2(low + high + Vector2i.ONE) * PREVIEW_CELL * 0.5
	var color: Color = TetrisBoardView.PIECE_COLORS[piece_id]
	if dimmed:
		color = color.darkened(0.35)
	for cell: Vector2i in cells:
		TetrisBoardView.draw_block(self, origin + Vector2(cell) * PREVIEW_CELL, PREVIEW_CELL, color)
