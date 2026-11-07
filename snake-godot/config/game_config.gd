class_name GameConfig
extends Resource
## Single source of truth for every tuning value (GDD §9, Technical Design §3).
## Edit the values in res://config/game_config.tres, not here.

@export_group("Grid")
@export var grid_landscape := Vector2i(20, 15)
@export var grid_portrait := Vector2i(15, 20)
@export var start_length := 3
@export var start_delay := 1.0 ## Seconds before the run starts by itself (or the first input).

@export_group("Speed")
@export var speed_start := 7.0 ## Moves per second at the start of a run.
@export var speed_step := 0.5 ## Added every `fruits_per_step` fruits.
@export var fruits_per_step := 3
@export var speed_cap := 15.0

@export_group("Rules")
@export var input_buffer_size := 2
@export var points_per_fruit := 10
@export var milestones: Array[int] = [10, 25, 50, 100] ## Snake lengths that trigger a celebration.

@export_group("Feel")
@export var near_miss_cooldown := 1.5
@export var fruit_panic_range := 3 ## Manhattan distance in cells.
@export var swipe_threshold := 30.0 ## Canvas units.
@export var gameover_input_lock := 0.5
@export var death_freeze := 0.3
@export var death_sequence := 1.2 ## Total DYING time before the Game Over screen.


## Speed ramp: min(start + step * floor(fruits / fruits_per_step), cap).
func moves_per_sec(fruits_eaten: int) -> float:
	var steps := floori(fruits_eaten / float(fruits_per_step))
	return minf(speed_start + speed_step * steps, speed_cap)
