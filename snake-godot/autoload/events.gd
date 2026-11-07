extends Node
## Global signal bus (Technical Design §8).
## Only Game emits these. Views, UI and audio connect to them in _ready().

signal run_started
signal turned(dir: Vector2i)
signal fruit_eaten(cell: Vector2i, fruit_type: int)
signal near_miss(cell: Vector2i)
signal milestone_reached(length: int)
signal speed_changed(moves_per_sec: float)
signal died(cause: String)
signal board_cleared
signal new_best(score: int)
signal score_changed(score: int)
signal state_changed(state: int)
