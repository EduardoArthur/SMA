class_name RouteGraph
extends RefCounted
## Grafo de rotas expandido em células (port de buildLevel() do sim.js).
## Células não declaradas são BLOCKED.

enum TileType { BLOCKED, ROUTE, NODE, MARKED, RAVINE }

var level: LevelData
var node_cells: Array[Vector2i] = []
var node_kinds: PackedInt32Array = PackedInt32Array()
## Arestas de saída de cada nó, ordenadas por id.
var node_outs: Array[PackedInt32Array] = []
## Células de cada aresta, sem as células dos nós das pontas.
var edge_cells: Array[Array] = []
var edge_from: PackedInt32Array = PackedInt32Array()
var edge_to: PackedInt32Array = PackedInt32Array()
var spawn_node: int = -1
var route_cells: Array[Vector2i] = []
var marked_cells: Array[Vector2i] = []

var _tile: Dictionary[Vector2i, int] = {}
var _cell_node: Dictionary[Vector2i, int] = {}
## célula -> Vector2i(edge_id, índice na aresta)
var _cell_edge: Dictionary[Vector2i, Vector2i] = {}


static func build(p_level: LevelData) -> RouteGraph:
	var g := RouteGraph.new()
	g.level = p_level
	g._build()
	return g


func _build() -> void:
	var node_count: int = level.nodes.size()
	node_cells.resize(node_count)
	node_kinds.resize(node_count)
	node_outs.resize(node_count)
	# Packed arrays dentro de Array são valores (cópia): montamos com Array[int]
	# e só convertemos no fim.
	var outs: Array[Array] = []
	outs.resize(node_count)
	for i in node_count:
		outs[i] = []
	for n in level.nodes:
		assert(n.id >= 0 and n.id < node_count, "ids de nó devem ser 0..n-1")
		node_cells[n.id] = n.cell
		node_kinds[n.id] = n.kind
		_tile[n.cell] = TileType.NODE
		_cell_node[n.cell] = n.id
		if n.kind == RouteNode.Kind.SPAWN:
			spawn_node = n.id

	var edge_count: int = level.edges.size()
	edge_cells.resize(edge_count)
	edge_from.resize(edge_count)
	edge_to.resize(edge_count)
	for e in level.edges:
		var points: Array[Vector2i] = [node_cells[e.from_id]]
		points.append_array(e.via)
		points.append(node_cells[e.to_id])
		var full := _expand(points)
		var cells: Array[Vector2i] = []
		for i in range(1, full.size() - 1):
			cells.append(full[i])
		if cells.is_empty():
			push_error("Aresta %d sem células" % e.id)
		for idx in cells.size():
			var c: Vector2i = cells[idx]
			if _tile.has(c):
				push_error("Célula repetida %s (aresta %d)" % [c, e.id])
			_tile[c] = TileType.ROUTE
			_cell_edge[c] = Vector2i(e.id, idx)
		edge_cells[e.id] = cells
		edge_from[e.id] = e.from_id
		edge_to[e.id] = e.to_id
		outs[e.from_id].append(e.id)

	for i in node_count:
		var list: Array = outs[i]
		list.sort()
		node_outs[i] = PackedInt32Array(list)
	for c in level.marked:
		_tile[c] = TileType.MARKED
	for c in level.ravine:
		_tile[c] = TileType.RAVINE
	for c in _tile:
		if _tile[c] == TileType.ROUTE:
			route_cells.append(c)
		elif _tile[c] == TileType.MARKED:
			marked_cells.append(c)


static func _expand(points: Array[Vector2i]) -> Array[Vector2i]:
	var cells: Array[Vector2i] = []
	for i in points.size() - 1:
		var p: Vector2i = points[i]
		var target: Vector2i = points[i + 1]
		var step := Vector2i(signi(target.x - p.x), signi(target.y - p.y))
		assert(step.x == 0 or step.y == 0, "segmento diagonal em %s" % p)
		if i == 0:
			cells.append(p)
		while p != target:
			p += step
			cells.append(p)
	return cells


func tile_type(cell: Vector2i) -> int:
	return _tile.get(cell, TileType.BLOCKED)


func is_node_cell(cell: Vector2i) -> bool:
	return _cell_node.has(cell)


func node_at(cell: Vector2i) -> int:
	return _cell_node.get(cell, -1)


## Vector2i(edge_id, índice) ou Vector2i(-1, -1).
func edge_at(cell: Vector2i) -> Vector2i:
	return _cell_edge.get(cell, Vector2i(-1, -1))


## Nó cuja escolha uma isca nesta célula controla (o `from` da aresta, se ele
## tiver 2+ saídas), ou -1 se a isca só servir de âncora (S12).
func bait_controlled_node(cell: Vector2i) -> int:
	var loc: Vector2i = edge_at(cell)
	if loc.x < 0:
		return -1
	var from: int = edge_from[loc.x]
	return from if node_outs[from].size() > 1 else -1


func cell_of_edge(edge_id: int, idx: int) -> Vector2i:
	return edge_cells[edge_id][idx]


func edge_length(edge_id: int) -> int:
	return edge_cells[edge_id].size()
