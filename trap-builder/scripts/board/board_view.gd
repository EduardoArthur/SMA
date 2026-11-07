class_name BoardView
extends Node2D
## Tabuleiro: desenha o mapa a partir do LevelData/RouteGraph e sincroniza os
## PigViews com a Simulation. Só LÊ a simulação.
##
## PLACEHOLDER (S03): o chão é desenhado com _draw() nas cores da art bible.
## Na S21 troca para TileMapLayer gerado (set_cells_terrain_connect) com arte real.

const TILE: int = 64
const PIG_VIEW: PackedScene = preload("res://scenes/board/pig_view.tscn")

# Paletas (02-design/art/art-bible.md §3)
const BP_BG := Color("1f4e8c")
const BP_GRID := Color("3a6fb0")
const BP_LINE := Color("f2f6ff")
const BP_ROUTE := Color("ffd84d")
const BP_DANGER := Color("ff6b5a")
const BP_VALID := Color("7fe0ff")
const GRASS := Color("7bc950")
const GRASS_ALT := Color("74c14a")
const TRAIL := Color("d9a066")
const TRAIL_EDGE := Color("b07c45")
const TREE := Color("2f6b3a")
const BUSH := Color("3f8a4a")
const RAVINE_EDGE := Color("6b4a2e")
const RAVINE_DEEP := Color("2b1d14")
const MARKED := Color("9c8a6e")
const INK := Color("2b1b17")
const PAPER := Color("f2e3c2")

const TRAP_COLORS: Dictionary[StringName, Color] = {
	&"anvil": Color("4a4e57"), &"tnt": Color("d63a2f"), &"bait": Color("3d7dd9"),
	&"fan": Color("9aa3ad"), &"tar": Color("1e1a24"),
}
const TRAP_LETTERS: Dictionary[StringName, String] = {
	&"anvil": "A", &"tnt": "T", &"bait": "I", &"fan": "F", &"tar": "p",
}

var level: LevelData
var graph: RouteGraph
var plan: Plan
var sim: Simulation
var blueprint_mode: bool = true

var _decor: Dictionary[Vector2i, int] = {}
var _pig_views: Dictionary[int, PigView] = {}

# cursor de posicionamento (S07)
var _cursor_trap: TrapData
var _cursor_cell: Vector2i = Vector2i(-1, -1)
var _cursor_dir: Vector2i = Vector2i.UP
var _cursor_valid: bool = false
var _cursor_visible: bool = true

# texturas SVG das armadilhas (S21), por nome
var _art: Dictionary[String, Texture2D] = {}

# transição (S18): -1 = parada; 0..1 = progresso do wipe
var _transition: float = -1.0
# célula -> direções para as células vizinhas na rota (para desenhar por tile)
var _route_links: Dictionary[Vector2i, Array] = {}
# caminhos das fugas (S19): Array de Array[int] (ids de aresta)
var _escape_paths: Array = []

# efeitos temporários (só visual): {kind, pos, t0, dur}
var _fx: Array = []  # sem tipo: recebe o retorno de filter()

@onready var _pigs_root: Node2D = $Pigs


func setup(p_level: LevelData, p_graph: RouteGraph) -> void:
	level = p_level
	graph = p_graph
	_build_decor()
	_build_route_links()
	_load_art()
	queue_redraw()


func set_plan(p_plan: Plan) -> void:
	plan = p_plan
	queue_redraw()


func set_blueprint_mode(on: bool) -> void:
	blueprint_mode = on
	queue_redraw()


## Armadilha selecionada + célula sob o mouse. `valid` já considera o orçamento.
func set_cursor(trap: TrapData, cell: Vector2i, dir: Vector2i, valid: bool) -> void:
	_cursor_trap = trap
	_cursor_cell = cell
	_cursor_dir = dir
	_cursor_valid = valid
	queue_redraw()


func set_cursor_visible(on: bool) -> void:
	_cursor_visible = on
	queue_redraw()


func attach_sim(p_sim: Simulation) -> void:
	detach_sim()
	sim = p_sim
	sim.trap_resolved.connect(_on_sim_trap_resolved)
	sim.pig_captured.connect(_on_sim_pig_captured)
	sim.pig_escaped.connect(_on_sim_pig_escaped)


func detach_sim() -> void:
	sim = null
	_fx.clear()
	_escape_paths.clear()
	_transition = -1.0
	for view in _pig_views.values():
		if is_instance_valid(view):
			view.queue_free()
	_pig_views.clear()
	queue_redraw()


func cell_to_world(cell: Vector2) -> Vector2:
	return (cell + Vector2(0.5, 0.5)) * TILE


func world_to_cell(pos: Vector2) -> Vector2i:
	return Vector2i(floori(pos.x / TILE), floori(pos.y / TILE))


func board_size_px() -> Vector2:
	return Vector2(level.grid_size * TILE) if level != null else Vector2.ZERO


func _process(_delta: float) -> void:
	if sim != null:
		_sync_pigs()
		_update_fx()
		queue_redraw()


func _sync_pigs() -> void:
	for p in sim.pigs:
		if p.state == PigState.State.PENDING:
			continue
		if not p.is_active():
			# Capturado/fugiu: tira do dicionário ANTES da animação de saída
			# (o PigView se libera sozinho ao fim do tween).
			if _pig_views.has(p.id):
				var leaving: PigView = _pig_views[p.id]
				_pig_views.erase(p.id)
				leaving.set_state(p.state)
			continue
		if not _pig_views.has(p.id):
			var created := PIG_VIEW.instantiate() as PigView
			created.pig_id = p.id
			created.position = cell_to_world(sim.get_pig_render_pos(p.id))
			_pigs_root.add_child(created)
			_pig_views[p.id] = created
		var view: PigView = _pig_views[p.id]
		view.position = cell_to_world(sim.get_pig_render_pos(p.id))
		view.set_state(p.state)


# --- decoração (só visual; RNG próprio, nunca afeta a simulação) ------------

func _build_decor() -> void:
	_decor.clear()
	var rng := RandomNumberGenerator.new()
	rng.seed = level.decor_seed
	for y in level.grid_size.y:
		for x in level.grid_size.x:
			var c := Vector2i(x, y)
			if graph.tile_type(c) == RouteGraph.TileType.BLOCKED:
				var roll := rng.randf()
				if roll < 0.18:
					_decor[c] = 1   # árvore
				elif roll < 0.30:
					_decor[c] = 2   # arbusto


# --- desenho -----------------------------------------------------------------

func _draw() -> void:
	if level == null:
		return
	if blueprint_mode:
		_draw_blueprint()
		_draw_valid_tiles()
	else:
		_draw_action()
	_draw_traps()
	if blueprint_mode:
		_draw_cursor()
	elif _transition >= 0.0:
		# S18: a planta azul por cima, ainda não "apagada" pelo wipe diagonal
		_draw_blueprint_cells(_transition)
		_draw_nodes(BP_LINE, BP_BG, _transition)


# --- transição Blueprint -> Ação (S18) ---------------------------------------

## Anima o wipe diagonal (0,7 s). Retorna o sinal de término do Tween.
func play_transition(duration: float = 0.7) -> Signal:
	_transition = 0.0
	var tw := create_tween()
	tw.tween_method(_set_transition, 0.0, 1.0, duration).set_ease(Tween.EASE_IN_OUT).set_trans(Tween.TRANS_SINE)
	tw.tween_callback(_set_transition.bind(-1.0))
	return tw.finished


func _set_transition(value: float) -> void:
	_transition = value
	queue_redraw()


## 0..1: em que momento do wipe esta célula deixa de ser planta azul.
func _wipe_value(c: Vector2i) -> float:
	var total := float(level.grid_size.x) + float(level.grid_size.y) * 0.3
	return (float(c.x) + float(c.y) * 0.3) / total


func _build_route_links() -> void:
	_route_links.clear()
	for e in graph.edge_cells.size():
		var chain: Array[Vector2i] = [graph.node_cells[graph.edge_from[e]]]
		for c: Vector2i in graph.edge_cells[e]:
			chain.append(c)
		chain.append(graph.node_cells[graph.edge_to[e]])
		for i in chain.size() - 1:
			var a: Vector2i = chain[i]
			var b: Vector2i = chain[i + 1]
			if not _route_links.has(a):
				_route_links[a] = []
			if not _route_links.has(b):
				_route_links[b] = []
			_route_links[a].append(b - a)
			_route_links[b].append(a - b)


func _rect(c: Vector2i) -> Rect2:
	return Rect2(Vector2(c * TILE), Vector2(TILE, TILE))


func _center(c: Vector2i) -> Vector2:
	return cell_to_world(Vector2(c))


func _draw_blueprint() -> void:
	_draw_blueprint_cells(-1.0)
	_draw_blocked_preview()
	_draw_nodes(BP_LINE, BP_BG)
	_draw_spawn_count()


## Desenha a planta azul tile a tile. Com threshold >= 0 (transição), só as
## células cujo wipe ainda não passou.
func _draw_blueprint_cells(threshold: float) -> void:
	for y in level.grid_size.y:
		for x in level.grid_size.x:
			var c := Vector2i(x, y)
			if threshold >= 0.0 and _wipe_value(c) < threshold:
				continue
			var r := _rect(c)
			draw_rect(r, BP_BG)
			draw_line(r.position, r.position + Vector2(TILE, 0), BP_GRID, 1.0)
			draw_line(r.position, r.position + Vector2(0, TILE), BP_GRID, 1.0)
			match graph.tile_type(c):
				RouteGraph.TileType.RAVINE:
					_draw_hatch(r, BP_DANGER)
				RouteGraph.TileType.MARKED:
					draw_rect(r.grow(-10), BP_LINE, false, 2.0)
			# rotas iluminadas: meio-segmento do centro até cada vizinho
			if _route_links.has(c):
				var center := r.get_center()
				for d: Vector2i in _route_links[c]:
					draw_line(center, center + Vector2(d) * (TILE * 0.5), BP_ROUTE, 6.0)
				draw_circle(center, 3.0, BP_ROUTE)


# S19: aresta que a bigorna vai bloquear aparece tracejada no Blueprint.
func _draw_blocked_preview() -> void:
	if plan == null:
		return
	for p in plan.placements:
		if p.trap.id != &"anvil":
			continue
		var edge_id: int = graph.edge_at(p.cell).x
		if edge_id < 0:
			continue
		for c: Vector2i in graph.edge_cells[edge_id]:
			var center := _center(c)
			for d: Vector2i in _route_links.get(c, []):
				var end := center + Vector2(d) * (TILE * 0.5)
				draw_line(center, end, BP_BG, 8.0)
				draw_dashed_line(center, end, BP_ROUTE, 4.0, 7.0)


func _draw_action() -> void:
	for y in level.grid_size.y:
		for x in level.grid_size.x:
			var c := Vector2i(x, y)
			var r := _rect(c)
			match graph.tile_type(c):
				RouteGraph.TileType.ROUTE, RouteGraph.TileType.NODE:
					draw_rect(r, TRAIL)
				RouteGraph.TileType.RAVINE:
					draw_rect(r, RAVINE_EDGE)
					draw_rect(r.grow(-8), RAVINE_DEEP)
				RouteGraph.TileType.MARKED:
					draw_rect(r, GRASS)
					draw_circle(r.get_center(), 22.0, MARKED)
					draw_arc(r.get_center(), 22.0, 0.0, TAU, 24, INK, 3.0)
				_:
					draw_rect(r, GRASS if (x + y) % 2 == 0 else GRASS_ALT)
	for c in _decor:
		var center := _center(c)
		if _decor[c] == 1:
			draw_circle(center, 26.0, TREE)
			draw_arc(center, 26.0, 0.0, TAU, 24, INK, 3.0)
		else:
			draw_circle(center, 16.0, BUSH)
			draw_arc(center, 16.0, 0.0, TAU, 20, INK, 2.0)
	# rotas bloqueadas por bigorna (leve escurecimento)
	if sim != null:
		for e in graph.edge_cells.size():
			if sim.is_edge_blocked(e):
				for c: Vector2i in graph.edge_cells[e]:
					draw_rect(_rect(c), Color(0, 0, 0, 0.12))
	_draw_nodes(INK, PAPER)
	_draw_escape_paths()


# S19: caminho de cada porco que fugiu, em vermelho (visível até voltar ao Blueprint).
func _draw_escape_paths() -> void:
	for path: Array in _escape_paths:
		for e: int in path:
			var pts := PackedVector2Array([_center(graph.node_cells[graph.edge_from[e]])])
			for c: Vector2i in graph.edge_cells[e]:
				pts.append(_center(c))
			pts.append(_center(graph.node_cells[graph.edge_to[e]]))
			draw_polyline(pts, Color(BP_DANGER, 0.75), 5.0)
	if not _escape_paths.is_empty():
		for i in graph.node_cells.size():
			if graph.node_kinds[i] == RouteNode.Kind.EXIT:
				draw_arc(_center(graph.node_cells[i]), 28.0, 0.0, TAU, 24, BP_DANGER, 4.0)


func _on_sim_pig_escaped(_pig_id: int, path_edges: PackedInt32Array) -> void:
	_escape_paths.append(Array(path_edges))


func _draw_nodes(line: Color, fill: Color, threshold: float = -1.0) -> void:
	var font := ThemeDB.fallback_font
	for i in graph.node_cells.size():
		if threshold >= 0.0 and _wipe_value(graph.node_cells[i]) < threshold:
			continue
		var center := _center(graph.node_cells[i])
		match graph.node_kinds[i]:
			RouteNode.Kind.SPAWN:
				draw_rect(Rect2(center - Vector2(18, 14), Vector2(36, 28)), fill)
				draw_rect(Rect2(center - Vector2(18, 14), Vector2(36, 28)), line, false, 3.0)
				draw_colored_polygon(PackedVector2Array([center + Vector2(-22, -14), center + Vector2(0, -30), center + Vector2(22, -14)]), line)
			RouteNode.Kind.EXIT:
				draw_line(center + Vector2(-8, 22), center + Vector2(-8, -24), line, 4.0)
				draw_colored_polygon(PackedVector2Array([center + Vector2(-8, -24), center + Vector2(20, -14), center + Vector2(-8, -4)]), BP_DANGER)
				draw_string(font, center + Vector2(-24, 30), "SAÍDA", HORIZONTAL_ALIGNMENT_CENTER, 48, 12, line)
			RouteNode.Kind.FORK:
				if blueprint_mode or threshold >= 0.0:
					draw_colored_polygon(PackedVector2Array([center + Vector2(0, -10), center + Vector2(10, 0), center + Vector2(0, 10), center + Vector2(-10, 0)]), line)


func _draw_spawn_count() -> void:
	var font := ThemeDB.fallback_font
	var center := _center(graph.node_cells[graph.spawn_node])
	draw_string(font, center + Vector2(-32, 40), "%d porcos" % level.spawns.size(), HORIZONTAL_ALIGNMENT_CENTER, 64, 13, BP_LINE)


func _draw_hatch(r: Rect2, color: Color) -> void:
	draw_rect(r, Color(color, 0.25))
	var step := 12.0
	var x := -r.size.y
	while x < r.size.x:
		var a := r.position + Vector2(maxf(x, 0.0), maxf(-x, 0.0))
		var b := r.position + Vector2(minf(x + r.size.y, r.size.x), minf(r.size.x - x, r.size.y))
		draw_line(a, b, color, 2.0)
		x += step


func _draw_valid_tiles() -> void:
	if _cursor_trap == null or not _cursor_visible or plan == null:
		return
	var cells: Array[Vector2i] = []
	if (_cursor_trap.valid_tiles & TrapData.TileFlag.ROUTE) != 0:
		cells.append_array(graph.route_cells)
	if (_cursor_trap.valid_tiles & TrapData.TileFlag.MARKED) != 0:
		cells.append_array(graph.marked_cells)
	for c in cells:
		if plan.at(c) == null:
			draw_rect(_rect(c).grow(-3), Color(BP_VALID, 0.35))


func _draw_cursor() -> void:
	if not _cursor_visible or level == null:
		return
	if _cursor_cell.x < 0 or _cursor_cell.y < 0 \
			or _cursor_cell.x >= level.grid_size.x or _cursor_cell.y >= level.grid_size.y:
		return
	var r := _rect(_cursor_cell)
	if _cursor_trap == null:
		# sem seleção: só realça a armadilha removível sob o cursor
		if plan != null and plan.at(_cursor_cell) != null:
			draw_rect(r.grow(-4), BP_DANGER, false, 3.0)
		return
	var color := Color(0.3, 1.0, 0.5, 0.55) if _cursor_valid else Color(BP_DANGER, 0.55)
	draw_rect(r.grow(-8), color)
	draw_rect(r.grow(-8), BP_LINE, false, 2.0)
	var font := ThemeDB.fallback_font
	draw_string(font, r.position + Vector2(0, r.size.y * 0.66), TRAP_LETTERS.get(_cursor_trap.id, "?"),
			HORIZONTAL_ALIGNMENT_CENTER, r.size.x, 22, BP_LINE)
	if _cursor_trap.has_direction:
		_draw_arrow(r.get_center(), _cursor_dir, BP_ROUTE)
		# prévia da corrente de ar (até FAN_RANGE tiles)
		var c := _cursor_cell
		for i in SimConst.FAN_RANGE:
			c += _cursor_dir
			var ty := graph.tile_type(c)
			if ty == RouteGraph.TileType.BLOCKED or ty == RouteGraph.TileType.MARKED:
				break
			draw_rect(_rect(c).grow(-20), Color(BP_LINE, 0.35))


func _draw_arrow(center: Vector2, dir: Vector2i, color: Color) -> void:
	var d := Vector2(dir)
	var tip := center + d * 28.0
	draw_line(center - d * 10.0, tip, color, 4.0)
	var side := Vector2(-d.y, d.x) * 8.0
	draw_colored_polygon(PackedVector2Array([tip + d * 6.0, tip - d * 6.0 + side, tip - d * 6.0 - side]), color)


func _draw_fallen_anvil(c: Vector2i) -> void:
	var center := _center(c)
	var body := PackedVector2Array([
		center + Vector2(-22, 16), center + Vector2(22, 16), center + Vector2(14, -2),
		center + Vector2(26, -6), center + Vector2(26, -14), center + Vector2(-20, -14), center + Vector2(-14, -2)])
	draw_colored_polygon(body, TRAP_COLORS[&"anvil"])
	body.append(body[0])
	draw_polyline(body, INK, 3.0)
	draw_line(center + Vector2(-16, -10), center + Vector2(18, -10), Color("8a909c"), 2.0)
	# poeira
	draw_circle(center + Vector2(-26, 18), 5.0, Color(PAPER, 0.7))
	draw_circle(center + Vector2(28, 18), 4.0, Color(PAPER, 0.7))


func _draw_traps() -> void:
	if plan == null:
		return
	for i in plan.placements.size():
		var p: Placement = plan.placements[i]
		var state: TrapState = sim.traps[i] if sim != null and i < sim.traps.size() else null
		if blueprint_mode:
			_draw_trap_blueprint(p)
		else:
			_draw_trap_action(p, state)
	if not blueprint_mode:
		_draw_ravine_fill()
		_draw_fx()


# Blueprint: traço branco + regras visíveis (isca controla a bifurcação, S12).
func _draw_trap_blueprint(p: Placement) -> void:
	var font := ThemeDB.fallback_font
	var r := _rect(p.cell).grow(-12)
	var bp_name := "%s_bp" % ("fan_base" if p.trap.id == &"fan" else String(p.trap.id))
	if _tex(bp_name) != null:
		_draw_tex(bp_name, _center(p.cell))
	else:
		draw_rect(r, BP_LINE, false, 3.0)
		draw_string(font, r.position + Vector2(0, r.size.y * 0.7), TRAP_LETTERS.get(p.trap.id, "?"),
				HORIZONTAL_ALIGNMENT_CENTER, r.size.x, 22, BP_LINE)
	match p.trap.id:
		&"fan":
			_draw_arrow(_center(p.cell), p.dir, BP_ROUTE)
			var c := p.cell
			for i in SimConst.FAN_RANGE:
				c += p.dir
				var ty := graph.tile_type(c)
				if ty == RouteGraph.TileType.BLOCKED or ty == RouteGraph.TileType.MARKED:
					break
				draw_rect(_rect(c).grow(-22), Color(BP_LINE, 0.4))
		&"bait":
			var node_id := graph.bait_controlled_node(p.cell)
			if node_id < 0:
				# sem bifurcação para controlar: só segura a fila
				draw_string(font, r.position + Vector2(r.size.x - 8, 8), "!", HORIZONTAL_ALIGNMENT_LEFT, -1, 22, BP_DANGER)
			else:
				var edge_id: int = graph.edge_at(p.cell).x
				var from := _center(graph.node_cells[node_id])
				var to := _center(graph.cell_of_edge(edge_id, 0))
				_draw_arrow(from, Vector2i((to - from).sign()), BP_DANGER)


# Ação: placeholders coloridos por armadilha + estados da simulação.
func _draw_trap_action(p: Placement, state: TrapState) -> void:
	var font := ThemeDB.fallback_font
	var center := _center(p.cell)
	var r := _rect(p.cell).grow(-12)
	var color: Color = TRAP_COLORS.get(p.trap.id, Color.MAGENTA)
	var now := Time.get_ticks_msec() / 1000.0
	if _draw_trap_textured(p, state, center, now):
		return
	match p.trap.id:
		&"anvil":
			if state != null and state.fallen:
				_draw_fallen_anvil(p.cell)
			elif state != null and state.triggered:
				# sombra crescendo + bigorna descendo (S08)
				var k := 1.0 - float(state.timer) / float(SimConst.ANVIL_SHADOW)
				draw_circle(center, 8.0 + 18.0 * k, Color(0, 0, 0, 0.35))
				var drop := Rect2(center + Vector2(-16, -16 - 120.0 * (1.0 - k)), Vector2(32, 24))
				draw_rect(drop, color)
				draw_rect(drop, INK, false, 2.0)
			else:
				# escondida "no céu": só uma marquinha de corda no chão
				draw_circle(center, 5.0, Color(0, 0, 0, 0.2))
		&"tnt":
			if state != null and state.consumed:
				return  # explodiu (o FX desenha a nuvem)
			draw_rect(r, color)
			draw_rect(r, INK, false, 3.0)
			draw_rect(Rect2(r.position + Vector2(0, r.size.y * 0.38), Vector2(r.size.x, r.size.y * 0.26)), PAPER)
			draw_string(font, r.position + Vector2(0, r.size.y * 0.6), "TNT", HORIZONTAL_ALIGNMENT_CENTER, r.size.x, 13, INK)
			var fuse_tip := r.position + Vector2(r.size.x * 0.75, -8)
			draw_line(r.position + Vector2(r.size.x * 0.6, 0), fuse_tip, INK, 2.0)
			if state != null and state.triggered:
				var flicker := 5.0 + 3.0 * sin(now * 40.0)
				draw_circle(fuse_tip, flicker, Color("ffd84d"))
				draw_circle(fuse_tip, flicker * 0.5, Color.WHITE)
		&"bait":
			var eaten := state != null and state.consumed
			var bowl := PackedVector2Array()
			for a in 9:
				var ang := PI * float(a) / 8.0
				bowl.append(center + Vector2(cos(ang) * 22.0, sin(ang) * 14.0 + 2.0))
			draw_colored_polygon(bowl, color)
			bowl.append(bowl[0])
			draw_polyline(bowl, INK, 2.5)
			if not eaten:
				draw_line(center + Vector2(-18, 2), center + Vector2(18, 2), Color("a3c244"), 5.0)
				# linhas de cheiro
				for k in 3:
					var x := -10.0 + 10.0 * k
					var y0 := -6.0 - fmod(now * 18.0 + k * 7.0, 18.0)
					draw_line(center + Vector2(x + 3.0 * sin(now * 4.0 + k), y0), center + Vector2(x - 3.0 * sin(now * 4.0 + k), y0 - 7.0), Color(PAPER, 0.85), 2.0)
		&"fan":
			draw_rect(r, Color("5c6470"))
			draw_rect(r, INK, false, 3.0)
			draw_circle(center, 15.0, color)
			draw_arc(center, 15.0, 0.0, TAU, 20, INK, 2.0)
			var spin := now * 12.0
			for k in 3:
				var ang := spin + TAU * k / 3.0
				draw_line(center, center + Vector2(cos(ang), sin(ang)) * 13.0, INK, 3.0)
			_draw_arrow(center, p.dir, INK)
			if state != null:
				_draw_fan_current(state, now)
		&"tar":
			draw_set_transform(center + Vector2(0, 4), 0.0, Vector2(1.4, 0.8))
			draw_circle(Vector2.ZERO, 20.0, color)
			draw_set_transform(Vector2.ZERO)
			var bubble := fmod(now * 0.8, 1.0)
			draw_arc(center + Vector2(-8, 2), 3.0 + 3.0 * bubble, 0.0, TAU, 12, Color("4a4458"), 2.0)
			draw_arc(center + Vector2(9, 6), 2.0 + 2.0 * fmod(bubble + 0.5, 1.0), 0.0, TAU, 12, Color("4a4458"), 2.0)


# --- arte SVG (S21) ----------------------------------------------------------

const ART_TRAPS: Array[String] = [
	"anvil", "anvil_fallen", "tnt", "bait", "bait_empty", "fan_base", "fan_blades", "tar",
	"anvil_bp", "tnt_bp", "bait_bp", "fan_base_bp", "tar_bp",
]
const ART_SCALE: float = 0.5  # SVGs desenhados a 128 px; tile de 64 px


func _load_art() -> void:
	for art_name in ART_TRAPS:
		var path := "res://art/traps/%s.svg" % art_name
		if ResourceLoader.exists(path):
			_art[art_name] = load(path)


func _tex(art_name: String) -> Texture2D:
	return _art.get(art_name)


func _draw_tex(art_name: String, center: Vector2, rotation_rad: float = 0.0, scale_mult: float = 1.0) -> void:
	var tex := _tex(art_name)
	if tex == null:
		return
	draw_set_transform(center, rotation_rad, Vector2.ONE * ART_SCALE * scale_mult)
	draw_texture(tex, -tex.get_size() / 2.0)
	draw_set_transform(Vector2.ZERO)


## Desenha a armadilha com a arte SVG. Retorna false se faltar textura (usa o placeholder).
func _draw_trap_textured(p: Placement, state: TrapState, center: Vector2, now: float) -> bool:
	match p.trap.id:
		&"anvil":
			if _tex("anvil") == null or _tex("anvil_fallen") == null:
				return false
			if state != null and state.fallen:
				_draw_tex("anvil_fallen", center)
			elif state != null and state.triggered:
				var k := 1.0 - float(state.timer) / float(SimConst.ANVIL_SHADOW)
				draw_circle(center + Vector2(0, 10), 8.0 + 18.0 * k, Color(0, 0, 0, 0.35))
				_draw_tex("anvil", center + Vector2(0, -130.0 * (1.0 - k)))
			else:
				draw_circle(center, 5.0, Color(0, 0, 0, 0.2))
		&"tnt":
			if _tex("tnt") == null:
				return false
			if state != null and state.consumed:
				return true
			_draw_tex("tnt", center)
			if state != null and state.triggered:
				var tip := center + Vector2(21, -25)
				var flicker := 5.0 + 3.0 * sin(now * 40.0)
				draw_circle(tip, flicker, Color("ffd84d"))
				draw_circle(tip, flicker * 0.5, Color.WHITE)
		&"bait":
			if _tex("bait") == null or _tex("bait_empty") == null:
				return false
			_draw_tex("bait_empty" if state != null and state.consumed else "bait", center)
		&"fan":
			if _tex("fan_base") == null or _tex("fan_blades") == null:
				return false
			_draw_tex("fan_base", center)
			_draw_tex("fan_blades", center, now * 12.0, 0.9)
			_draw_arrow(center, p.dir, INK)
			if state != null:
				_draw_fan_current(state, now)
		&"tar":
			if _tex("tar") == null:
				return false
			_draw_tex("tar", center)
			var bubble := fmod(now * 0.8, 1.0)
			draw_arc(center + Vector2(-10, -2), 3.0 + 3.0 * bubble, 0.0, TAU, 12, Color("4a4458"), 2.0)
		_:
			return false
	return true


# Ventilador: linhas de vento correndo na corrente (S13).
func _draw_fan_current(state: TrapState, now: float) -> void:
	var d := Vector2(state.dir)
	var side := Vector2(-d.y, d.x)
	for c: Vector2i in state.current:
		var base := _center(c)
		for k in 2:
			var phase := fmod(now * 2.5 + k * 0.5, 1.0)
			var start := base - d * 24.0 + d * 48.0 * phase + side * (k * 14.0 - 7.0)
			draw_line(start, start + d * 12.0, Color(1, 1, 1, 0.75), 2.0)


# Barranco lotado: mostra quantos porcos caíram (D014).
func _draw_ravine_fill() -> void:
	if sim == null:
		return
	var font := ThemeDB.fallback_font
	for c in level.ravine:
		var n := sim.ravine_fill(c)
		if n > 0:
			var r := _rect(c)
			var txt := "%d/%d" % [n, SimConst.RAVINE_CAP]
			draw_string(font, r.position + Vector2(0, r.size.y * 0.62), txt, HORIZONTAL_ALIGNMENT_CENTER, r.size.x, 16,
					BP_DANGER if n >= SimConst.RAVINE_CAP else PAPER)


# --- FX de eventos (S11–S13), só visual -------------------------------------

func _add_fx(kind: StringName, pos: Vector2, duration: float) -> void:
	_fx.append({"kind": kind, "pos": pos, "t0": Time.get_ticks_msec() / 1000.0, "dur": duration})


func _on_sim_trap_resolved(trap_id: int) -> void:
	var t: TrapState = sim.traps[trap_id]
	match t.kind:
		&"tnt":
			_add_fx(&"boom", _center(t.cell), 0.6)
		&"anvil":
			_add_fx(&"dust", _center(t.cell), 0.4)


func _on_sim_pig_captured(pig_id: int, cause: StringName) -> void:
	var pos := cell_to_world(sim.get_pig_render_pos(pig_id))
	_add_fx(&"fall" if cause == &"ravine" else &"pop", pos, 0.45)


func _update_fx() -> void:
	var now := Time.get_ticks_msec() / 1000.0
	_fx = _fx.filter(func(f: Dictionary) -> bool: return now - float(f.t0) < float(f.dur))


func _draw_fx() -> void:
	var now := Time.get_ticks_msec() / 1000.0
	for f: Dictionary in _fx:
		var k: float = clampf((now - float(f.t0)) / float(f.dur), 0.0, 1.0)
		var pos: Vector2 = f.pos
		match f.kind:
			&"boom":
				if k < 0.15:
					draw_circle(pos, 40.0 + 120.0 * k, Color(1, 1, 0.8, 0.9))
				for j in 3:
					var off := Vector2(cos(j * 2.1), sin(j * 2.1)) * 26.0 * k
					draw_circle(pos + off, 34.0 * (1.0 - k * 0.4), Color(0.85, 0.85, 0.85, 0.85 * (1.0 - k)))
			&"dust":
				for j in 4:
					var off := Vector2(cos(j * 1.6 + 0.4), 0.4) * 30.0 * k
					draw_circle(pos + off + Vector2(0, 18), 7.0 * (1.0 - k), Color(PAPER, 0.8 * (1.0 - k)))
			&"pop":
				for j in 5:
					var ang := TAU * j / 5.0
					draw_circle(pos + Vector2(cos(ang), sin(ang)) * 24.0 * k, 4.0 * (1.0 - k), Color("ffd84d"))
			&"fall":
				draw_arc(pos, 20.0 * (1.0 - k), 0.0, TAU, 16, Color(PAPER, 1.0 - k), 3.0)
