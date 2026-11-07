class_name Simulation
extends RefCounted
## Simulação determinística de uma onda (port 1:1 de simulate() em
## _gdks-output/02-design/levels/tools/sim.js; spec: technical-design.md §2 e §4).
##
## Regras de ouro:
##  - só inteiros; 1 chamada de step() = 1 tick (60 Hz);
##  - ordem fixa: spawn -> timers (armadilhas, depois porcos) -> movimento (por id)
##    -> gatilhos de entrada -> ventiladores -> fim;
##  - nada de Nodes, física, delta ou RNG aqui. O visual só LÊ este objeto.
## Paridade verificada por tests/run_tests.gd contra tests/expected.json.

signal pig_spawned(pig_id: int)
signal pig_state_changed(pig_id: int, state: int)
signal pig_pushed(pig_id: int, to_cell: Vector2i)
signal pig_captured(pig_id: int, cause: StringName)
signal pig_escaped(pig_id: int, path_edges: PackedInt32Array)
signal trap_triggered(trap_id: int)
signal trap_resolved(trap_id: int)
signal edge_blocked(edge_id: int)
signal finished(result: SimResult)


var level: LevelData
var graph: RouteGraph
var pigs: Array[PigState] = []
var traps: Array[TrapState] = []
var tick: int = 0
var captured: int = 0
var escaped: int = 0
var is_finished: bool = false
var result: SimResult

var _trap_at: Dictionary[Vector2i, TrapState] = {}
var _blocked: Array[bool] = []
var _ravine_fill: Dictionary[Vector2i, int] = {}
var _next_spawn: int = 0
var _next_spawn_tick: int = 0
var _cost: int = 0


func _init(p_level: LevelData, p_graph: RouteGraph, plan: Plan) -> void:
	level = p_level
	graph = p_graph
	_blocked.resize(graph.edge_cells.size())
	_blocked.fill(false)
	for i in plan.placements.size():
		var t := TrapState.new(i, plan.placements[i])
		traps.append(t)
		_trap_at[t.cell] = t
		_cost += t.data.cost
	for t in traps:
		if t.kind == &"fan":
			var c: Vector2i = t.cell
			for i in SimConst.FAN_RANGE:
				c += t.dir
				var ty: int = graph.tile_type(c)
				if ty == RouteGraph.TileType.BLOCKED or ty == RouteGraph.TileType.MARKED:
					break
				t.current[c] = true
	for i in level.spawns.size():
		pigs.append(PigState.new(i, level.spawns[i].pig))


## Avança um tick. Não faz nada depois de terminar.
func step() -> void:
	if is_finished:
		return
	_spawn()
	var entered: Array[PigState] = []
	_update_trap_timers()
	_update_pig_timers(entered)
	_move_pigs(entered)
	_process_entered(entered)
	if _next_spawn == pigs.size() and captured + escaped == pigs.size():
		_finish(false)
		return
	tick += 1
	if tick >= SimConst.MAX_TICKS:
		_finish(true)


func run_to_end() -> SimResult:
	while not is_finished:
		step()
	return result


# --- consultas (usadas também pelo visual) -----------------------------------

func cell_of(p: PigState) -> Vector2i:
	if p.node >= 0:
		return graph.node_cells[p.node]
	return graph.cell_of_edge(p.edge, p.idx)


## Posição em unidades de tile (float, só para desenhar). Não altera estado.
func get_pig_render_pos(pig_id: int) -> Vector2:
	var p: PigState = pigs[pig_id]
	if p.state == PigState.State.PENDING:
		return Vector2(graph.node_cells[graph.spawn_node])
	var here := Vector2(cell_of(p))
	if not p.is_moving():
		return here
	var next_cell: Vector2i = _peek_next_cell(p)
	var t: float = clampf(float(p.sub) / float(SimConst.TILE_UNITS), 0.0, 1.0)
	return here.lerp(Vector2(next_cell), t)


func trap_by_cell(cell: Vector2i) -> TrapState:
	return _trap_at.get(cell)


func is_edge_blocked(edge_id: int) -> bool:
	return _blocked[edge_id]


## Quantos porcos já caíram neste tile de barranco (D014).
func ravine_fill(cell: Vector2i) -> int:
	return _ravine_fill.get(cell, 0)


# --- passos do tick ----------------------------------------------------------

func _spawn() -> void:
	if _next_spawn < pigs.size() and tick >= _next_spawn_tick:
		var spawn_cell: Vector2i = graph.node_cells[graph.spawn_node]
		if not _occupied(spawn_cell, null):
			var p: PigState = pigs[_next_spawn]
			_next_spawn += 1
			p.node = graph.spawn_node
			p.edge = -1
			p.sub = 0
			_set_state(p, PigState.State.WALK)
			_next_spawn_tick = tick + SimConst.SPAWN_INTERVAL
			pig_spawned.emit(p.id)


func _update_trap_timers() -> void:
	for t in traps:
		if t.kind == &"anvil" and t.triggered and not t.fallen:
			t.timer -= 1
			if t.timer <= 0:
				t.fallen = true
				var v: PigState = t.victim
				if v != null and v.is_active():
					if v.data.immune_anvil:
						_set_state(v, PigState.State.STUN, SimConst.HELMET_STUN)
					else:
						_capture(v, &"anvil")
				var loc: Vector2i = graph.edge_at(t.cell)
				var e: int = loc.x
				_blocked[e] = true
				edge_blocked.emit(e)
				trap_resolved.emit(t.id)
				for q in pigs:
					if q.is_active() and q.node == -1 and q.edge == e and q.idx < loc.y \
							and (q.state == PigState.State.WALK or q.state == PigState.State.PUSH):
						q.sub = 0
						_set_state(q, PigState.State.BACK)
		elif t.kind == &"tnt" and t.triggered and not t.consumed:
			t.timer -= 1
			if t.timer <= 0:
				t.consumed = true
				for q in pigs:
					if q.is_active():
						var d: Vector2i = (cell_of(q) - t.cell).abs()
						if maxi(d.x, d.y) <= SimConst.TNT_RADIUS:
							_capture(q, &"tnt")
				trap_resolved.emit(t.id)


func _update_pig_timers(entered: Array[PigState]) -> void:
	for p in pigs:
		if not p.is_active():
			continue
		match p.state:
			PigState.State.EAT:
				p.timer -= 1
				if p.timer <= 0:
					var bait: TrapState = _trap_at[cell_of(p)]
					bait.consumed = true
					_set_state(p, PigState.State.WALK)
					trap_resolved.emit(bait.id)
			PigState.State.CLIMB, PigState.State.STUN:
				p.timer -= 1
				if p.timer <= 0:
					_set_state(p, PigState.State.WALK)
			PigState.State.PUSH:
				p.timer -= 1
				if p.timer <= 0:
					_set_state(p, PigState.State.WALK)
					if p.arrive_pending:
						p.arrive_pending = false
						entered.append(p)


func _move_pigs(entered: Array[PigState]) -> void:
	for p in pigs:
		if not p.is_active() or not p.is_moving():
			continue
		var here_trap: TrapState = _trap_at.get(cell_of(p))
		var speed: int = p.data.speed
		if here_trap != null and here_trap.kind == &"tar":
			@warning_ignore("integer_division")
			speed = speed / SimConst.TAR_DIV
		p.sub += speed
		if p.sub < SimConst.TILE_UNITS:
			continue

		if p.state == PigState.State.BACK:
			p.sub -= SimConst.TILE_UNITS
			if p.idx > 0:
				p.idx -= 1
			else:
				p.node = graph.edge_from[p.edge]
				p.edge = -1
				_set_state(p, PigState.State.WALK)
			_enter(p, entered)
			continue

		var n_node: int = -1
		var n_edge: int = -1
		var n_idx: int = 0
		if p.node >= 0:
			n_edge = _choose_edge(p, p.node)
		elif p.idx < graph.edge_length(p.edge) - 1:
			n_edge = p.edge
			n_idx = p.idx + 1
		else:
			n_node = graph.edge_to[p.edge]
		var next_cell: Vector2i = graph.node_cells[n_node] if n_node >= 0 else graph.cell_of_edge(n_edge, n_idx)
		if _occupied(next_cell, p):
			p.sub = SimConst.TILE_UNITS - 1
			continue
		p.sub -= SimConst.TILE_UNITS
		_relocate(p, n_node, n_edge, n_idx)
		_enter(p, entered)


func _process_entered(entered: Array[PigState]) -> void:
	for p in entered:
		if not p.is_active() or not p.is_moving():
			continue
		var cell: Vector2i = cell_of(p)
		var t: TrapState = _trap_at.get(cell)
		if t != null and not t.consumed:
			match t.kind:
				&"anvil":
					if t.fallen:
						_set_state(p, PigState.State.CLIMB, SimConst.ANVIL_CLIMB)
					elif not t.triggered:
						t.triggered = true
						t.timer = SimConst.ANVIL_SHADOW
						t.victim = p
						_set_state(p, PigState.State.LOOK)
						trap_triggered.emit(t.id)
				&"tnt":
					if not t.triggered:
						t.triggered = true
						t.timer = SimConst.TNT_FUSE
						trap_triggered.emit(t.id)
				&"bait":
					if t.eater == null:
						t.eater = p
						_set_state(p, PigState.State.EAT, SimConst.BAIT_EAT)
						trap_triggered.emit(t.id)
		if not p.is_moving():
			continue
		for f in traps:
			if f.kind != &"fan" or not f.current.has(cell) or p.pushed_by.has(f.id):
				continue
			p.pushed_by.append(f.id)
			var dest: Vector2i = cell + f.dir
			var ty: int = graph.tile_type(dest)
			if ty == RouteGraph.TileType.RAVINE and (SimConst.RAVINE_CAP == 0 or _ravine_fill.get(dest, 0) < SimConst.RAVINE_CAP):
				_ravine_fill[dest] = _ravine_fill.get(dest, 0) + 1
				_capture(p, &"ravine")
				break
			var at: TrapState = _trap_at.get(dest)
			var anvil_there: bool = at != null and at.kind == &"anvil" and at.fallen
			if (ty == RouteGraph.TileType.ROUTE or ty == RouteGraph.TileType.NODE) and not _occupied(dest, p) and not anvil_there:
				var n_node: int = graph.node_at(dest)
				var loc: Vector2i = graph.edge_at(dest)
				_relocate(p, n_node, loc.x, maxi(loc.y, 0))
				p.sub = 0
				_set_state(p, PigState.State.PUSH, SimConst.FAN_PUSH)
				pig_pushed.emit(p.id, dest)
				if n_node >= 0 and graph.node_kinds[n_node] == RouteNode.Kind.EXIT:
					_escape(p)
					break
				# O efeito de chegada é aplicado quando o empurrão termina (timers).
				p.arrive_pending = true
				break


# --- auxiliares --------------------------------------------------------------

func _choose_edge(p: PigState, node_id: int) -> int:
	var outs: PackedInt32Array = graph.node_outs[node_id]
	var avail := PackedInt32Array()
	for e in outs:
		if not _blocked[e]:
			avail.append(e)
	for e in avail:
		if _bait_active_on(e):
			return e
	var options: PackedInt32Array = avail if avail.size() > 0 else outs
	return ForkHash.choose_edge(level.route_seed, p.id, node_id, options)


func _bait_active_on(edge_id: int) -> bool:
	for c in graph.edge_cells[edge_id]:
		var t: TrapState = _trap_at.get(c)
		if t != null and t.kind == &"bait" and not t.consumed:
			return true
	return false


## Próxima célula para desenhar (puro: não muda estado).
func _peek_next_cell(p: PigState) -> Vector2i:
	if p.state == PigState.State.BACK:
		if p.idx > 0:
			return graph.cell_of_edge(p.edge, p.idx - 1)
		return graph.node_cells[graph.edge_from[p.edge]]
	if p.node >= 0:
		if graph.node_kinds[p.node] == RouteNode.Kind.EXIT or graph.node_outs[p.node].is_empty():
			return graph.node_cells[p.node]
		return graph.cell_of_edge(_choose_edge(p, p.node), 0)
	if p.idx < graph.edge_length(p.edge) - 1:
		return graph.cell_of_edge(p.edge, p.idx + 1)
	return graph.node_cells[graph.edge_to[p.edge]]


func _relocate(p: PigState, n_node: int, n_edge: int, n_idx: int) -> void:
	if n_node >= 0:
		p.node = n_node
		p.edge = -1
		p.idx = 0
	else:
		if p.path_edges.is_empty() or p.path_edges[-1] != n_edge:
			p.path_edges.append(n_edge)
		p.node = -1
		p.edge = n_edge
		p.idx = n_idx


func _enter(p: PigState, entered: Array[PigState]) -> void:
	if p.node >= 0 and graph.node_kinds[p.node] == RouteNode.Kind.EXIT:
		_escape(p)
		return
	entered.append(p)


func _occupied(cell: Vector2i, self_pig: PigState) -> bool:
	for q in pigs:
		if q != self_pig and q.is_active() and cell_of(q) == cell:
			return true
	return false


func _set_state(p: PigState, state: PigState.State, timer: int = -1) -> void:
	p.state = state
	if timer >= 0:
		p.timer = timer
	pig_state_changed.emit(p.id, state)


func _capture(p: PigState, cause: StringName) -> void:
	if not p.is_active():
		return
	captured += 1
	_set_state(p, PigState.State.CAPTURED)
	pig_captured.emit(p.id, cause)


func _escape(p: PigState) -> void:
	escaped += 1
	_set_state(p, PigState.State.ESCAPED)
	pig_escaped.emit(p.id, PackedInt32Array(p.path_edges))


func _finish(deadlock: bool) -> void:
	is_finished = true
	result = SimResult.make(captured, escaped, tick, _cost, deadlock, level)
	finished.emit(result)
