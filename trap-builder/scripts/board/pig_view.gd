class_name PigView
extends Node2D
## Visual de um porco. Placeholder vetorial desenhado em _draw() (arte real na S21).
## Posição definida pelo BoardView a partir de Simulation.get_pig_render_pos().

const BODY := Color("f4a6b8")
const SNOUT := Color("e77f9b")
const INK := Color("2b1b17")
# Arte SVG (S21). Sem os arquivos, cai no desenho placeholder.
static var TEX_IDLE: Texture2D = load("res://art/pigs/pig.svg") if ResourceLoader.exists("res://art/pigs/pig.svg") else null
static var TEX_WALK: Texture2D = load("res://art/pigs/pig_walk.svg") if ResourceLoader.exists("res://art/pigs/pig_walk.svg") else null

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
	if TEX_IDLE != null and TEX_WALK != null:
		# arte SVG (S21): 128 px desenhado a 64 px, espelhado pela direção
		var walking := _state == PigState.State.WALK or _state == PigState.State.BACK
		var tex: Texture2D = TEX_WALK if walking and int(_bob_time / PI) % 2 == 1 else TEX_IDLE
		draw_set_transform(o, 0.0, Vector2(0.5 * _facing, 0.5))
		draw_texture(tex, -tex.get_size() / 2.0 + Vector2(0, -10))
		draw_set_transform(Vector2.ZERO)
		_draw_state_mark(o)
		return
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
	_draw_state_mark(o)


func _draw_state_mark(o: Vector2) -> void:
	var font := ThemeDB.fallback_font
	var mark := ""
	match _state:
		PigState.State.LOOK: mark = "!"
		PigState.State.BACK: mark = "?"
		PigState.State.EAT: mark = "♥"
		PigState.State.CLIMB: mark = "^"
		PigState.State.STUN: mark = "*"
	if mark != "":
		draw_string(font, o + Vector2(-10, -40), mark, HORIZONTAL_ALIGNMENT_CENTER, 20, 20, INK)
