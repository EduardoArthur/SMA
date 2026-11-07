extends Node
## Turns keyboard, mouse and touch input into Game calls (Technical Design §6).
## - Keys: Input Map actions (arrows/WASD, Esc/P, any key on Title/Game Over).
## - Swipes: one turn per swipe, re-anchored after each turn so a quick
##   L-shaped double swipe gives two turns (GDD §4).
## - Taps: handled through the mouse click Godot emulates from touch, so a
##   touch is never counted twice.

@onready var game: Game = owner as Game

var _touching := false
var _anchor := Vector2.ZERO


func _unhandled_input(event: InputEvent) -> void:
	if game.input_blocked or event.is_action_pressed("settings"):
		return # Settings is open, or is about to be opened by Screens.

	var touch := event as InputEventScreenTouch
	if touch != null:
		if touch.index == 0: # Only the first finger steers.
			_touching = touch.pressed
			_anchor = touch.position
		return
	var drag := event as InputEventScreenDrag
	if drag != null:
		if _touching and drag.index == 0:
			_on_drag(drag.position)
		return

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
		game.on_tap() # Taps during PLAYING do nothing (Game ignores them).
	else:
		return
	get_viewport().set_input_as_handled()


## A swipe is the dominant axis of the drag once it passes the threshold.
## Positions are in canvas units (stretch mode canvas_items), so the same
## threshold works on every screen size.
func _on_drag(pos: Vector2) -> void:
	var d := pos - _anchor
	if d.length() < game.config.swipe_threshold:
		return
	var dir := Vector2i.RIGHT if d.x > 0.0 else Vector2i.LEFT
	if absf(d.y) > absf(d.x):
		dir = Vector2i.DOWN if d.y > 0.0 else Vector2i.UP
	game.request_turn(dir)
	_anchor = pos # Re-anchor: keep dragging in a new direction to turn again.


func _is_left_click(event: InputEvent) -> bool:
	var mb := event as InputEventMouseButton
	return mb != null and mb.pressed and mb.button_index == MOUSE_BUTTON_LEFT
