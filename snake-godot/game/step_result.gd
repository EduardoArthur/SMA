class_name StepResult
extends RefCounted
## What happened during one SnakeModel.step() (Technical Design §4.2).
## Game turns these flags into Events signals.

var turned := false
var ate := false
var died := false
var death_cause := "" ## "wall" or "self"
var near_miss := false
var near_miss_cell := Vector2i.ZERO
var board_cleared := false
var milestone := 0 ## 0 = none, otherwise the snake length that was reached.
var speed_changed := false
