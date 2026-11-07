class_name SimConst
## Constantes da simulação (technical-design.md §2.1 e §4).
## Devem bater com _gdks-output/02-design/levels/tools/sim.js.

const TICK_RATE: int = 60
const TILE_UNITS: int = 1200
const SPAWN_INTERVAL: int = 60
const MAX_TICKS: int = 20000

# Armadilhas (ticks)
const ANVIL_SHADOW: int = 24
const ANVIL_CLIMB: int = 120
const HELMET_STUN: int = 30
const TNT_FUSE: int = 18
const TNT_RADIUS: int = 1
const BAIT_EAT: int = 180
const FAN_PUSH: int = 15
const FAN_RANGE: int = 3
## Porcos por tile de barranco antes de lotar (D014). 0 = sem limite.
const RAVINE_CAP: int = 2

const DIRS: Array[Vector2i] = [Vector2i.RIGHT, Vector2i.DOWN, Vector2i.LEFT, Vector2i.UP]
