extends CanvasLayer
## Basic HUD and screens (GDD §2): score, best, one centered message for
## Title / Ready / Paused / Game Over, and the Settings overlay (F15).
## Built in code on purpose; M7 replaces it with themed scenes (Art Bible §8).

@onready var game: Game = owner as Game

var _score: Label
var _best: Label
var _dim: ColorRect
var _message: VBoxContainer
var _headline: Label
var _details: Label
var _settings_button: Button
var _pause_button: Button
var _settings: Control


func _ready() -> void:
	_dim = ColorRect.new()
	_dim.color = Palette.DIM
	_dim.mouse_filter = Control.MOUSE_FILTER_IGNORE # Let clicks reach InputHandler.
	_dim.set_anchors_and_offsets_preset(Control.PRESET_FULL_RECT)
	add_child(_dim)

	_score = _make_label(36)
	_best = _make_label(36)
	for label: Label in [_score, _best]:
		label.set_anchors_and_offsets_preset(Control.PRESET_TOP_WIDE)
		label.offset_left = 24
		label.offset_right = -24
		label.offset_top = 14
		label.offset_bottom = 70
		add_child(label)
	_best.horizontal_alignment = HORIZONTAL_ALIGNMENT_RIGHT

	# Pause button at the top center while playing (needed on touch screens).
	_pause_button = _make_button("II")
	_pause_button.custom_minimum_size = Vector2(64, 52)
	_pause_button.set_anchors_and_offsets_preset(Control.PRESET_CENTER_TOP)
	_pause_button.offset_left = -32
	_pause_button.offset_right = 32
	_pause_button.offset_top = 12
	_pause_button.offset_bottom = 64
	_pause_button.pressed.connect(game.toggle_pause)
	add_child(_pause_button)

	_message = VBoxContainer.new()
	_message.alignment = BoxContainer.ALIGNMENT_CENTER
	_message.mouse_filter = Control.MOUSE_FILTER_IGNORE
	_message.set_anchors_and_offsets_preset(Control.PRESET_FULL_RECT)
	_message.add_theme_constant_override("separation", 12)
	add_child(_message)
	_headline = _make_label(72)
	_details = _make_label(28)
	for label: Label in [_headline, _details]:
		label.horizontal_alignment = HORIZONTAL_ALIGNMENT_CENTER
		_message.add_child(label)
	_settings_button = _make_button("Settings (O)")
	_settings_button.size_flags_horizontal = Control.SIZE_SHRINK_CENTER
	_settings_button.pressed.connect(_open_settings)
	_message.add_child(_settings_button)

	_settings = _build_settings()
	_settings.visible = false
	add_child(_settings)

	Events.state_changed.connect(_on_state_changed)
	Events.score_changed.connect(_on_score_changed)


func _unhandled_input(event: InputEvent) -> void:
	if _settings.visible:
		if event.is_action_pressed("pause") or event.is_action_pressed("settings"):
			_close_settings()
		if event is InputEventKey or event is InputEventMouseButton:
			get_viewport().set_input_as_handled() # Nothing reaches the game behind the panel.
	elif event.is_action_pressed("settings") and _settings_allowed():
		_open_settings()
		get_viewport().set_input_as_handled()


# --- HUD and messages -------------------------------------------------------------

func _on_score_changed(score: int) -> void:
	_score.text = "Score: %d" % score


func _on_state_changed(_state: int) -> void:
	_best.text = "Best: %d" % SaveData.best
	match game.state:
		Game.State.TITLE:
			_show("SNAKE", "Press any key or tap to play\nArrows / WASD or swipe to move  ·  Esc / P to pause")
		Game.State.READY:
			_show("Ready?", "Press a direction or swipe to go!", false)
		Game.State.PAUSED:
			_show("Paused", "Press Esc / P or tap to resume")
		Game.State.GAME_OVER:
			var details := "Score: %d" % game.model.score
			if game.last_run_new_best:
				details += "\nNEW BEST!"
			details += "\n\nPress any key or tap to retry"
			_show("PERFECT!" if game.last_run_perfect else "Game Over", details)
		_:
			_show("", "", false) # PLAYING and DYING: keep the board clear.


func _show(headline: String, details: String, dim := true) -> void:
	_headline.text = headline
	_details.text = details
	_headline.visible = headline != ""
	_details.visible = details != ""
	_dim.visible = dim
	_settings_button.visible = _settings_allowed()
	_pause_button.visible = game.state == Game.State.PLAYING


func _settings_allowed() -> bool:
	return game.state == Game.State.TITLE or game.state == Game.State.PAUSED


# --- Settings (GDD §2, F15) ----------------------------------------------------------

func _open_settings() -> void:
	AudioManager.play_sfx("ui_click", -6.0, 0.03)
	_settings.visible = true
	_message.visible = false
	_dim.visible = true
	game.input_blocked = true


func _close_settings() -> void:
	AudioManager.play_sfx("ui_click", -6.0, 0.03)
	SaveData.save_data()
	_settings.visible = false
	_message.visible = true
	game.input_blocked = false
	_on_state_changed(game.state)


func _build_settings() -> Control:
	var center := CenterContainer.new()
	center.set_anchors_and_offsets_preset(Control.PRESET_FULL_RECT)
	center.mouse_filter = Control.MOUSE_FILTER_STOP # Clicks outside the panel don't reach the game.

	var panel := PanelContainer.new()
	var style := StyleBoxFlat.new()
	style.bg_color = Palette.UI_PANEL
	style.border_color = Palette.OUTLINE
	style.set_border_width_all(4)
	style.set_corner_radius_all(24)
	style.set_content_margin_all(32)
	style.shadow_color = Color(Palette.OUTLINE, 0.25)
	style.shadow_offset = Vector2(0, 6)
	style.shadow_size = 2
	panel.add_theme_stylebox_override("panel", style)
	center.add_child(panel)

	var box := VBoxContainer.new()
	box.add_theme_constant_override("separation", 18)
	panel.add_child(box)

	var title := _make_panel_label("Settings", 40)
	title.horizontal_alignment = HORIZONTAL_ALIGNMENT_CENTER
	box.add_child(title)

	box.add_child(_slider_row("Music", SaveData.music_volume, _on_music_volume_changed))
	box.add_child(_slider_row("Sound effects", SaveData.sfx_volume, _on_sfx_volume_changed))

	var shake := CheckButton.new()
	shake.text = "Screen shake"
	shake.button_pressed = SaveData.screen_shake
	shake.focus_mode = Control.FOCUS_NONE
	shake.add_theme_font_size_override("font_size", 26)
	for color_name in ["font_color", "font_hover_color", "font_pressed_color", "font_hover_pressed_color"]:
		shake.add_theme_color_override(color_name, Palette.UI_TEXT)
	shake.toggled.connect(func(on: bool) -> void: SaveData.screen_shake = on)
	box.add_child(shake)

	var back := _make_button("Back")
	back.size_flags_horizontal = Control.SIZE_SHRINK_CENTER
	back.pressed.connect(_close_settings)
	box.add_child(back)
	return center


func _on_music_volume_changed(value: float) -> void:
	SaveData.music_volume = value
	AudioManager.apply_volumes()


func _on_sfx_volume_changed(value: float) -> void:
	SaveData.sfx_volume = value
	AudioManager.apply_volumes()
	AudioManager.play_sfx("ui_click", -6.0, 0.03) # Preview the new level.


func _slider_row(text: String, value: float, on_change: Callable) -> Control:
	var row := HBoxContainer.new()
	row.add_theme_constant_override("separation", 16)
	var label := _make_panel_label(text, 26)
	label.custom_minimum_size.x = 190
	row.add_child(label)
	var slider := HSlider.new()
	slider.min_value = 0.0
	slider.max_value = 1.0
	slider.step = 0.05
	slider.value = value
	slider.custom_minimum_size = Vector2(280, 32)
	slider.size_flags_vertical = Control.SIZE_SHRINK_CENTER
	slider.value_changed.connect(on_change)
	row.add_child(slider)
	return row


# --- Widget helpers --------------------------------------------------------------------

func _make_label(font_size: int) -> Label:
	var label := Label.new()
	label.add_theme_font_size_override("font_size", font_size)
	label.add_theme_color_override("font_color", Color.WHITE)
	label.add_theme_color_override("font_outline_color", Palette.OUTLINE)
	label.add_theme_constant_override("outline_size", maxi(4, int(font_size * 0.2)))
	return label


func _make_panel_label(text: String, font_size: int) -> Label:
	var label := Label.new()
	label.text = text
	label.add_theme_font_size_override("font_size", font_size)
	label.add_theme_color_override("font_color", Palette.UI_TEXT)
	return label


func _make_button(text: String) -> Button:
	var button := Button.new()
	button.text = text
	button.focus_mode = Control.FOCUS_NONE # Space/Enter stay "any key" for the game.
	button.add_theme_font_size_override("font_size", 26)
	button.custom_minimum_size = Vector2(220, 52)
	var normal := StyleBoxFlat.new()
	normal.bg_color = Palette.UI_ACCENT
	normal.border_color = Palette.OUTLINE
	normal.set_border_width_all(3)
	normal.set_corner_radius_all(18)
	var hover := normal.duplicate() as StyleBoxFlat
	hover.bg_color = Palette.UI_ACCENT.lightened(0.08)
	var pressed := normal.duplicate() as StyleBoxFlat
	pressed.bg_color = Palette.UI_ACCENT.darkened(0.08)
	button.add_theme_stylebox_override("normal", normal)
	button.add_theme_stylebox_override("hover", hover)
	button.add_theme_stylebox_override("pressed", pressed)
	return button
