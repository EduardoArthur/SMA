class_name TrapState
extends RefCounted
## Estado de runtime de uma armadilha posicionada.

var id: int = 0
var data: TrapData
var kind: StringName
var cell: Vector2i
var dir: Vector2i = Vector2i.RIGHT
var triggered: bool = false
var fallen: bool = false
var consumed: bool = false
var timer: int = 0
var victim: PigState
var eater: PigState
## Ventilador: células da corrente de ar.
var current: Dictionary[Vector2i, bool] = {}


func _init(p_id: int = 0, placement: Placement = null) -> void:
	id = p_id
	if placement != null:
		data = placement.trap
		kind = placement.trap.id
		cell = placement.cell
		dir = placement.dir
