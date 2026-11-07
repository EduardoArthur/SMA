class_name Plan
extends RefCounted
## O plano do jogador: armadilhas posicionadas no Blueprint.
## A ordem da lista define o id da armadilha na simulação (resolução por id).

var placements: Array[Placement] = []


func cost() -> int:
	var total: int = 0
	for p in placements:
		total += p.trap.cost
	return total


func at(cell: Vector2i) -> Placement:
	for p in placements:
		if p.cell == cell:
			return p
	return null


## Tile compatível (ROTA fora de nós / MARCADO), sem outra armadilha no tile
## e cabendo no orçamento (budget < 0 ignora o orçamento).
func can_place(graph: RouteGraph, trap: TrapData, cell: Vector2i, budget: int = -1) -> bool:
	if at(cell) != null:
		return false
	if budget >= 0 and cost() + trap.cost > budget:
		return false
	var tile: int = graph.tile_type(cell)
	if tile == RouteGraph.TileType.ROUTE:
		return (trap.valid_tiles & TrapData.TileFlag.ROUTE) != 0
	if tile == RouteGraph.TileType.MARKED:
		return (trap.valid_tiles & TrapData.TileFlag.MARKED) != 0
	return false


func add(trap: TrapData, cell: Vector2i, dir: Vector2i = Vector2i.RIGHT) -> Placement:
	var p := Placement.new(trap, cell, dir)
	placements.append(p)
	return p


func remove_at(cell: Vector2i) -> Placement:
	var p := at(cell)
	if p != null:
		placements.erase(p)
	return p


func clear() -> void:
	placements.clear()


func duplicate_plan() -> Plan:
	var copy := Plan.new()
	for p in placements:
		copy.add(p.trap, p.cell, p.dir)
	return copy
