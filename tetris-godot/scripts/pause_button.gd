extends Button

@onready var _pause_shade: ColorRect = $"../PauseShade"


func _ready() -> void:
	pressed.connect(_toggle_pause)


func _input(event: InputEvent) -> void:
	if not visible or not event is InputEventKey:
		return
	var key: InputEventKey = event
	if key.keycode == KEY_ESCAPE and key.pressed and not key.echo:
		get_viewport().set_input_as_handled()
		_toggle_pause()


func _toggle_pause() -> void:
	var paused: bool = not get_tree().paused
	get_tree().paused = paused
	_pause_shade.visible = paused
	text = "Continuar  [Esc]" if paused else "Pausar  [Esc]"
