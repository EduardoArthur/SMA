class_name ForkHash
## Hash determinístico das bifurcações (technical-design.md §2.5, D003).
## NUNCA troque por randi()/hash()/RandomNumberGenerator: precisa ser estável
## entre execuções, plataformas e versões, e igual ao sim.js (teste test_fork_hash).
## Os produtos cabem em int de 64 bits (máximo < 2^59).

const MASK32: int = 0xFFFFFFFF


static func fork_hash(route_seed: int, pig_id: int, node_id: int) -> int:
	var h: int = (route_seed * 73856093) ^ (pig_id * 19349663) ^ (node_id * 83492791)
	h = h & MASK32
	h = ((h ^ (h >> 16)) * 0x45d9f3b) & MASK32
	h = ((h ^ (h >> 16)) * 0x45d9f3b) & MASK32
	h = h ^ (h >> 16)
	return h


## `options` deve estar ordenado por id de aresta (ordem estável).
static func choose_edge(route_seed: int, pig_id: int, node_id: int, options: PackedInt32Array) -> int:
	return options[fork_hash(route_seed, pig_id, node_id) % options.size()]
