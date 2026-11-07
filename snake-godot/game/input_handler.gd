extends Node
## Turns keyboard and mouse input into Game calls (Technical Design §6).
## Touch swipes are added in M6. A touch tap already arrives here as a mouse
## click, because Godot emulates the mouse from touch by default.

@onready var game: Game = owner as Game


func _unhandled_input(event: InputEvent) -> void:
	if event.is_action_pressed("move_up"):
		game.request_turn(Vector2i.UP)
	elif event.is_action_pressed("move_down"):
		game.request_turn(Vector2i.DOWN)
	elif event.is_action_pressed("move_left"):
		game.request_turn(Vector2i.LEFT)
	elif event.is_action_pressed("move_right"):
		game.request_turn(Vector2i.RIGHT)
	elif event.is_action_pressed("pause"):
		game.toggle_pause()
	elif event is InputEventKey and event.is_pressed() and not event.is_echo():
		game.on_tap(true) # "Press any key" on the Title and Game Over screens.
	elif _is_left_click(event):
		game.on_tap()
	else:
		return
	get_viewport().set_input_as_handled()


func _is_left_click(event: InputEvent) -> bool:
	var mb := event as InputEventMouseButton
	return mb != null and mb.pressed and mb.button_index == MOUSE_BUTTON_LEFT
