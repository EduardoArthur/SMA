class_name SimResult
extends RefCounted
## Resultado de uma onda (technical-design.md §5.3).

var captured: int = 0
var escaped: int = 0
var ticks: int = 0
var cost: int = 0
var deadlock: bool = false
var failed: bool = false
var stars: int = 0
var escaped_ids: PackedInt32Array = PackedInt32Array()


static func make(p_captured: int, p_escaped: int, p_ticks: int, p_cost: int, p_deadlock: bool, level: LevelData) -> SimResult:
	var r := SimResult.new()
	r.captured = p_captured
	r.escaped = p_escaped
	r.ticks = p_ticks
	r.cost = p_cost
	r.deadlock = p_deadlock
	r.failed = p_escaped > level.escape_limit or p_deadlock
	if r.failed:
		r.stars = 0
	elif p_escaped == 0:
		r.stars = 3 if p_cost <= level.par else 2
	else:
		r.stars = 1
	return r
