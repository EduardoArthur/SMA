class_name RouteEdge
extends Resource
## Aresta do grafo: caminho de from_id até to_id passando pelos cantos em `via`
## (segmentos ortogonais). Igual ao formato de levels.js; o RouteGraph expande
## os cantos em células.

@export var id: int = 0
@export var from_id: int = 0
@export var to_id: int = 0
@export var via: Array[Vector2i] = []
