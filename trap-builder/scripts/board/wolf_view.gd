class_name WolfView
extends Node2D
## O Lobo Mau assistindo e reagindo (S17, D007). Escuta GameEvents; só visual.
## Texturas: res://art/wolf/wolf_<expressão>.svg (S21). Sem textura, desenha um placeholder.

const EXPRESSIONS: Array[StringName] = [&"watch", &"lick", &"cheer", &"angry", &"feast", &"sad"]
const REACTION_TIME: float = 1.0
const MULTI_WINDOW_MS: int = 1000

var _textures: Dictionary[StringName, Texture2D] = {}
var _bush: Texture2D
var _expression: StringName = &"watch"
var _locked: bool = false          # expressão final do resultado
var _recent_captures: Array = []  # sem tipo: recebe o retorno de filter()
var _timer: SceneTreeTimer

@onready var _sprite: Sprite2D = Sprite2D.new()
@onready var _bush_sprite: Sprite2D = Sprite2D.new()
@onready var _caption: Label = Label.new()


func _ready() -> void:
	for e in EXPRESSIONS:
		var path := "res://art/wolf/wolf_%s.svg" % e
		if ResourceLoader.exists(path):
			_textures[e] = load(path)
	if ResourceLoader.exists("res://art/wolf/bush.svg"):
		_bush = load("res://art/wolf/bush.svg")
	add_child(_sprite)
	_bush_sprite.texture = _bush
	_bush_sprite.position = Vector2(0, 46)
	add_child(_bush_sprite)
	_caption.position = Vector2(-110, 100)
	_caption.size = Vector2(220, 24)
	_caption.horizontal_alignment = HORIZONTAL_ALIGNMENT_CENTER
	_caption.add_theme_color_override("font_color", UiStyle.INK)
	_caption.add_theme_font_size_override("font_size", 15)
	add_child(_caption)
	GameEvents.phase_changed.connect(_on_phase_changed)
	GameEvents.pig_captured.connect(_on_pig_captured)
	GameEvents.pig_escaped.connect(_on_pig_escaped)
	GameEvents.wave_finished.connect(_on_wave_finished)
	visible = false
	_show(&"watch", false)


func _on_phase_changed(state: int) -> void:
	# 0 BLUEPRINT · 1 TRANSICAO · 2 ACAO · 3 RESULTADO (LevelController.Phase)
	visible = state >= 1
	if state <= 1:
		_locked = false
		_recent_captures.clear()
		_show(&"watch", state == 1)


func _on_pig_captured(_pig_id: int, _cause: StringName) -> void:
	var now := Time.get_ticks_msec()
	_recent_captures.append(now)
	_recent_captures = _recent_captures.filter(func(t: int) -> bool: return now - t <= MULTI_WINDOW_MS)
	_react(&"cheer" if _recent_captures.size() >= 2 else &"lick")


func _on_pig_escaped(_pig_id: int, _path: PackedInt32Array) -> void:
	_react(&"angry")


func _on_wave_finished(result: SimResult) -> void:
	_locked = true
	if result.failed:
		_show(&"sad", true)
	elif result.stars == 3:
		_show(&"feast", true)
	else:
		_show(&"cheer", true)


func _react(expression: StringName) -> void:
	if _locked:
		return
	# fuga tem prioridade sobre captura dentro da mesma janela
	if _expression == &"angry" and expression != &"angry" and _timer != null and _timer.time_left > 0.0:
		return
	_show(expression, true)
	_timer = get_tree().create_timer(REACTION_TIME)
	_timer.timeout.connect(_back_to_watch.bind(_timer))


func _back_to_watch(timer: SceneTreeTimer) -> void:
	if timer == _timer and not _locked:
		_show(&"watch", false)


func _show(expression: StringName, pop: bool) -> void:
	if pop and expression != &"watch":
		GameEvents.wolf_reacted.emit(expression)
	_expression = expression
	_sprite.texture = _textures.get(expression)
	_caption.text = {
		&"watch": "", &"lick": "Hmm, toucinho!", &"cheer": "Que banquete!",
		&"angry": "Grrr! Fugiu!", &"feast": "Jantar perfeito!", &"sad": "De novo não...",
	}.get(expression, "")
	queue_redraw()
	if pop:
		var tw := create_tween()
		_sprite.scale = Vector2(1.15, 0.85)
		_sprite.position = Vector2(0, -8)
		tw.tween_property(_sprite, "scale", Vector2.ONE, 0.18)
		tw.parallel().tween_property(_sprite, "position", Vector2.ZERO, 0.18)


func _draw() -> void:
	if _sprite.texture != null:
		return
	# placeholder sem textura: cabeça cinza com orelhas e olhos amarelos
	var gray := Color("7a7f8c")
	draw_circle(Vector2(0, 0), 34.0, gray)
	draw_colored_polygon(PackedVector2Array([Vector2(-30, -16), Vector2(-20, -52), Vector2(-6, -26)]), gray)
	draw_colored_polygon(PackedVector2Array([Vector2(30, -16), Vector2(20, -52), Vector2(6, -26)]), gray)
	draw_circle(Vector2(-12, -6), 6.0, Color("ffe45c"))
	draw_circle(Vector2(12, -6), 6.0, Color("ffe45c"))
	draw_circle(Vector2(0, 30), 26.0, Color("3f8a4a"))
