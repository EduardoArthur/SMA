class_name ActionBar
extends PanelContainer
## Controles durante a Ação: pausar, velocidade, abortar (atalhos P, F, Backspace).

signal pause_pressed()
signal speed_pressed()

var _pause: Button
var _speed: Button


func _ready() -> void:
	UiStyle.style_panel(self)
	var box := VBoxContainer.new()
	box.add_theme_constant_override("separation", 8)
	add_child(box)
	box.add_child(UiStyle.label("EMBOSCADA!", 22))
	box.add_child(UiStyle.label("Agora é só assistir...", 12))
	_pause = _button("Pausar (P)")
	_pause.pressed.connect(pause_pressed.emit)
	box.add_child(_pause)
	_speed = _button("Velocidade 1x (F)")
	_speed.pressed.connect(speed_pressed.emit)
	box.add_child(_speed)
	var abort := _button("Abortar (Backspace)")
	abort.pressed.connect(GameEvents.abort_requested.emit)
	box.add_child(abort)


func refresh(paused: bool, speed: int) -> void:
	_pause.text = "Continuar (P)" if paused else "Pausar (P)"
	_speed.text = "Velocidade %dx (F)" % speed


func _button(text: String) -> Button:
	var b := Button.new()
	b.text = text
	b.custom_minimum_size = Vector2(0, 44)
	UiStyle.style_button(b)
	return b
