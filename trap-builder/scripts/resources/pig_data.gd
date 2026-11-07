class_name PigData
extends Resource
## Dados de um tipo de porco (technical-design.md §3.2).

@export var id: StringName
## Sub-unidades por tick (TILE_UNITS = 1200, 60 ticks/s). 30 = 1,5 tiles/s.
@export var speed: int = 30
@export var immune_anvil: bool = false
@export var scene: PackedScene
