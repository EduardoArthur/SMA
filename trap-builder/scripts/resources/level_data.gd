class_name LevelData
extends Resource
## Uma fase do protótipo. GERADO por _gdks-output/02-design/levels/tools/export_levels.js
## a partir de levels.js (fonte única, validada pelo solver). Não edite à mão:
## mude levels.js, rode validate.js e depois export_levels.js.

@export var level_id: int = 0
@export var title: String = ""
## Seed das bifurcações (fork_hash). Não se chama "seed" para não sombrear a função global.
@export var route_seed: int = 0
@export var budget: int = 0
@export var par: int = 0
@export var escape_limit: int = 0
@export var grid_size: Vector2i = Vector2i(18, 10)
@export var nodes: Array[RouteNode] = []
@export var edges: Array[RouteEdge] = []
@export var spawns: Array[SpawnEntry] = []
@export var ravine: Array[Vector2i] = []
@export var marked: Array[Vector2i] = []
@export var available_traps: Array[TrapData] = []
## Só visual (decoração). Não afeta a simulação.
@export var decor_seed: int = 0
