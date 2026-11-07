class_name TrapData
extends Resource
## Dados de uma armadilha (technical-design.md §3.1 e §4).
## Os tempos da simulação ficam em SimConst para manter paridade com sim.js.

enum TileFlag { ROUTE = 1, MARKED = 2 }

@export var id: StringName
@export var display_name: String = ""
@export var cost: int = 0
@export_flags("Rota", "Marcado") var valid_tiles: int = TileFlag.ROUTE
@export var has_direction: bool = false
## Contínua durante a onda (ventilador, piche)?
@export var persistent: bool = false
@export var icon: Texture2D
@export var scene: PackedScene
