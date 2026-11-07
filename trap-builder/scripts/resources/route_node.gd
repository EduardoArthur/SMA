class_name RouteNode
extends Resource
## Nó do grafo de rotas: spawn, bifurcação, junção ou saída.

enum Kind { SPAWN, FORK, JOIN, EXIT }

@export var id: int = 0
@export var cell: Vector2i = Vector2i.ZERO
@export var kind: Kind = Kind.JOIN
