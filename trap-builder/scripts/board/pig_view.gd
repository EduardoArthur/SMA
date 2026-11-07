class_name PigView
extends Node2D
## Visual de um porco. Placeholder vetorial desenhado em _draw() (arte real na S21).
## Posição definida pelo BoardView a partir de Simulation.get_pig_render_pos().

const BODY := Color("f4a6b8")
const SNOUT := Color("e77f9b")
const INK := Color("2b1b17")

var pig_id: int = -1
var _state: int = -1
var _bob_time: float = 0.0
var _last_pos: Vector2
var _facing: float = 1.0


func set_state(state: int) -> void:
	if state == _state:
		return
	_state = state
	match state:
		PigState.State.CAPTURED:
			var tw := create_tween()
			tw.tween_property(self, "scale", Vector2(1.4, 0.5), 0.12)
			tw.tween_property(self, "modulate:a", 0.0, 0.25)
			tw.tween_callback(queue_free)
		PigState.State.ESCAPED:
			var tw := create_tween()
			tw.tween_property(self, "position:y", position.y - 20.0, 0.15)
			tw.parallel().tween_property(self, "modulate:a", 0.0, 0.3)
			tw.tween_callback(queue_free)
		PigState.State.PUSH:
			var tw := create_tween()
			tw.tween_property(self, "rotation", 0.3 * _facing, 0.08)
			tw.tween_property(self, "rotation", 0.0, 0.15)
	queue_redraw()


func _process(delta: float) -> void:
	var moved := position - _last_pos
	if absf(moved.x) > 0.01:
		_facing = signf(moved.x)
	_last_pos = position
	if _state == PigState.State.WALK or _state == PigState.State.BACK:
		_bob_time += delta * 10.0
	queue_redraw()


func _draw() -> void:
	var bob := absf(sin(_bob_time)) * -3.0
	var o := Vector2(0, bob)
	# corpo (elipse)
	draw_set_transform(o, 0.0, Vector2(1.25, 1.0))
	draw_circle(Vector2.ZERO, 16.0, BODY)
	draw_arc(Vector2.ZERO, 16.0, 0.0, TAU, 28, INK, 2.5)
	draw_set_transform(Vector2.ZERO)
	# orelhas
	var ear_x := -6.0 * _facing
	draw_colored_polygon(PackedVector2Array([o + Vector2(ear_x - 6, -12), o + Vector2(ear_x, -24), o + Vector2(ear_x + 6, -12)]), SNOUT)
	# focinho e olho
	var snout := o + Vector2(18.0 * _facing, 0)
	draw_circle(snout, 6.0, SNOUT)
	draw_arc(snout, 6.0, 0.0, TAU, 16, INK, 2.0)
	draw_circle(o + Vector2(9.0 * _facing, -6), 2.5, INK)
	# indicadores de estado (placeholder até a S17/S21)
	var font := ThemeDB.fallback_font
	var mark := ""
	match _state:
		PigState.State.LOOK: mark = "!"
		PigState.State.BACK: mark = "?"
		PigState.State.EAT: mark = "♥"
		PigState.State.CLIMB: mark = "^"
		PigState.State.STUN: mark = "*"
	if mark != "":
		draw_string(font, o + Vector2(-10, -26), mark, HORIZONTAL_ALIGNMENT_CENTER, 20, 18, INK)
