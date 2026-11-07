class_name PigState
extends RefCounted
## Estado de runtime de um porco. Localização: num nó (node >= 0) ou numa
## aresta (node == -1, edge + idx). `sub` = progresso inteiro dentro do tile.

enum State { PENDING, WALK, BACK, LOOK, EAT, CLIMB, STUN, PUSH, CAPTURED, ESCAPED }

var id: int = 0
var data: PigData
var state: State = State.PENDING
var timer: int = 0
var node: int = -1
var edge: int = -1
var idx: int = 0
var sub: int = 0
## Array[int] (tipo referência) de propósito: mutado com append().
var pushed_by: Array[int] = []
var arrive_pending: bool = false
## Arestas percorridas (para destacar o caminho da fuga).
var path_edges: Array[int] = []


func _init(p_id: int = 0, p_data: PigData = null) -> void:
	id = p_id
	data = p_data


func is_active() -> bool:
	return state != State.PENDING and state != State.CAPTURED and state != State.ESCAPED


func is_moving() -> bool:
	return state == State.WALK or state == State.BACK
