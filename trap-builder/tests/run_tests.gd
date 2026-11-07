extends SceneTree
## Testes headless da simulação (technical-design.md §8, godot-architecture.md §8).
## Rode a partir da pasta do projeto:
##   godot --headless --import                 (primeira vez: monta o cache de classes)
##   godot --headless -s res://tests/run_tests.gd
## Código de saída = número de verificações que falharam (0 = tudo passou).
##
## tests/expected.json é GERADO por _gdks-output/02-design/levels/tools/export_levels.js
## a partir do simulador de referência (sim.js). Se este teste falha, o port
## GDScript divergiu da referência.

const EXPECTED_PATH := "res://tests/expected.json"

var _failed: int = 0
var _checks: int = 0


func _init() -> void:
	var expected: Dictionary = _load_expected()
	if expected.is_empty():
		quit(1)
		return
	_test_fork_hash(expected.get("fork_hash", []))
	_test_graph_level_1()
	_test_parity(expected.get("levels", []))
	_test_step_chunking()
	_test_plan_rules()

	print("")
	if _failed == 0:
		print("ALL PASSED (%d checks)" % _checks)
	else:
		print("FAILED: %d of %d checks" % [_failed, _checks])
	quit(_failed)


# --- helpers -----------------------------------------------------------------

func _check(condition: bool, label: String) -> void:
	_checks += 1
	if condition:
		print("  ok   %s" % label)
	else:
		_failed += 1
		print("  FAIL %s" % label)


func _load_expected() -> Dictionary:
	var text := FileAccess.get_file_as_string(EXPECTED_PATH)
	if text.is_empty():
		print("FAIL: não achei %s (rode export_levels.js)" % EXPECTED_PATH)
		return {}
	var data: Variant = JSON.parse_string(text)
	if typeof(data) != TYPE_DICTIONARY:
		print("FAIL: %s inválido" % EXPECTED_PATH)
		return {}
	return data


func _trap(id: String) -> TrapData:
	return load("res://data/traps/%s.tres" % id) as TrapData


func _v2i(arr: Array) -> Vector2i:
	return Vector2i(int(arr[0]), int(arr[1]))


func _run(level: LevelData, plan: Plan) -> SimResult:
	var graph := RouteGraph.build(level)
	return Simulation.new(level, graph, plan).run_to_end()


# --- testes ------------------------------------------------------------------

## T4: o hash é idêntico ao do sim.js.
func _test_fork_hash(samples: Array) -> void:
	print("fork_hash (%d amostras)" % samples.size())
	var mismatches: int = 0
	for s: Dictionary in samples:
		var got := ForkHash.fork_hash(int(s.seed), int(s.pig), int(s.node))
		if got != int(s.value):
			mismatches += 1
			print("    seed=%d pig=%d node=%d esperado=%d obtido=%d" % [s.seed, s.pig, s.node, int(s.value), got])
	_check(mismatches == 0 and samples.size() > 0, "fork_hash igual ao JS")


## Estrutura do grafo da Fase 1 (atalho 4 tiles, volta 10 tiles).
func _test_graph_level_1() -> void:
	print("grafo da fase 1")
	var level := load("res://data/levels/level_01.tres") as LevelData
	_check(level != null, "level_01.tres carrega")
	if level == null:
		return
	var g := RouteGraph.build(level)
	_check(g.spawn_node == 0, "spawn é o nó 0")
	_check(g.node_outs[1] == PackedInt32Array([1, 2]), "bifurcação tem saídas [1, 2] ordenadas")
	_check(g.edge_length(1) == 4, "atalho tem 4 tiles")
	_check(g.edge_length(2) == 10, "volta longa tem 10 tiles")
	_check(g.tile_type(Vector2i(5, 4)) == RouteGraph.TileType.ROUTE, "(5,4) é ROTA")
	_check(g.tile_type(Vector2i(3, 4)) == RouteGraph.TileType.NODE, "(3,4) é NÓ")
	_check(g.tile_type(Vector2i(10, 8)) == RouteGraph.TileType.BLOCKED, "(10,8) é BLOQUEADO")


## T1: paridade total com o simulador de referência (capturas, fugas, tick final, estrelas).
func _test_parity(levels: Array) -> void:
	for lv: Dictionary in levels:
		var level := load(String(lv.path)) as LevelData
		print("paridade fase %d" % int(lv.level_id))
		_check(level != null, "carrega %s" % lv.path)
		if level == null:
			continue
		for ref: Dictionary in lv.refs:
			var plan := Plan.new()
			for p: Dictionary in ref.plan:
				plan.add(_trap(String(p.type)), _v2i(p.cell), _v2i(p.dir))
			var r := _run(level, plan)
			var e: Dictionary = ref.expect
			var same: bool = r.captured == int(e.captured) and r.escaped == int(e.escaped) \
					and r.ticks == int(e.ticks) and r.stars == int(e.stars) \
					and r.cost == int(e.cost) and r.deadlock == bool(e.deadlock)
			if not same:
				print("    esperado capt=%d fug=%d ticks=%d ★%d custo=%d | obtido capt=%d fug=%d ticks=%d ★%d custo=%d" % [
					int(e.captured), int(e.escaped), int(e.ticks), int(e.stars), int(e.cost),
					r.captured, r.escaped, r.ticks, r.stars, r.cost])
			_check(same, "fase %d · %s" % [int(lv.level_id), ref.name])


## T2: o resultado não depende de quantos step() rodam por frame.
func _test_step_chunking() -> void:
	print("independência de velocidade")
	var level := load("res://data/levels/level_01.tres") as LevelData
	var plan := Plan.new()
	plan.add(_trap("anvil"), Vector2i(5, 4))
	plan.add(_trap("anvil"), Vector2i(3, 2))
	var a := _run(level, plan)
	var graph := RouteGraph.build(level)
	var sim := Simulation.new(level, graph, plan)
	while not sim.is_finished:
		sim.step()
		sim.step()
	var b := sim.result
	_check(a.captured == b.captured and a.escaped == b.escaped and a.ticks == b.ticks, "1x e 2x dão o mesmo resultado")


## Regras do Plan (posicionamento e custo).
func _test_plan_rules() -> void:
	print("regras do plano")
	var level := load("res://data/levels/level_04.tres") as LevelData
	var g := RouteGraph.build(level)
	var plan := Plan.new()
	var anvil := _trap("anvil")
	var fan := _trap("fan")
	_check(plan.can_place(g, anvil, Vector2i(5, 2)), "bigorna pode em ROTA")
	_check(not plan.can_place(g, anvil, Vector2i(3, 5)), "bigorna não pode em NÓ")
	_check(not plan.can_place(g, anvil, Vector2i(6, 3)), "bigorna não pode em MARCADO")
	_check(plan.can_place(g, fan, Vector2i(6, 3)), "ventilador pode em MARCADO")
	_check(not plan.can_place(g, fan, Vector2i(5, 2)), "ventilador não pode em ROTA")
	plan.add(fan, Vector2i(6, 3), Vector2i.UP)
	_check(not plan.can_place(g, fan, Vector2i(6, 3)), "um tile, uma armadilha")
	_check(plan.cost() == 125, "custo do ventilador = 125")
	_check(not plan.can_place(g, fan, Vector2i(9, 3), 200), "orçamento bloqueia o 2º ventilador")
	plan.remove_at(Vector2i(6, 3))
	_check(plan.cost() == 0, "remover devolve 100% (D013)")
