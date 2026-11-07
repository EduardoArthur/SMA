class_name Placement
extends RefCounted
## Uma armadilha posicionada no plano do jogador.

var trap: TrapData
var cell: Vector2i
var dir: Vector2i = Vector2i.RIGHT


func _init(p_trap: TrapData = null, p_cell: Vector2i = Vector2i.ZERO, p_dir: Vector2i = Vector2i.RIGHT) -> void:
	trap = p_trap
	cell = p_cell
	dir = p_dir
