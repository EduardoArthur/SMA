extends CanvasLayer
## HUD and screens (GDD §2) in the Art Bible §8 style: score and best, a pause
## button, the gold bobbing title, cream cards for Paused / Game Over with a
## pulsing "NEW BEST!", and the Settings overlay (F15). Built in code; colors
## come from Palette and the font from Fonts (Fredoka, if installed).

@onready var game: Game = owner as Game

var _score: Label
var _best: Label
var _dim: ColorRect
var _pause_button: Button
var _card: PanelContainer
var _card_style: StyleBoxFlat
var _headline: Label
var _new_best: Label
var _details: Label
var _settings_button: Button
var _settings: Control
var _time := 0.0


func _ready() -> void:
	Fonts.setup()
	_card_style = _panel_style()

	_dim = ColorRect.new()
	_dim.color = Palette.DIM
	_dim.mouse_filter = Control.MOUSE_FILTER_IGNORE # Let clicks reach InputHandler.
	_dim.set_anchors_and_offsets_preset(Control.PRESET_FULL_RECT)
	add_child(_dim)

	# HUD: score (left), best (right), pause button (center, while playing).
	_score = _make_label(36)
	_best = _make_label(36)
	for label: Label in [_score, _best]:
		_paint(label, Color.WHITE, 8)
		Fonts.make_bold(label)
		label.set_anchors_and_offsets_preset(Control.PRESET_TOP_WIDE)
		label.offset_left = 24
		label.offset_right = -24
		label.offset_top = 14
		label.offset_bottom = 70
		add_child(label)
	_best.horizontal_alignment = HORIZONTAL_ALIGNMENT_RIGHT

	_pause_button = _make_button("II")
	_pause_button.custom_minimum_size = Vector2(64, 52)
	_pause_button.set_anchors_and_offsets_preset(Control.PRESET_CENTER_TOP)
	_pause_button.offset_left = -32
	_pause_button.offset_right = 32
	_pause_button.offset_top = 12
	_pause_button.offset_bottom = 64
	_pause_button.pressed.connect(game.toggle_pause)
	add_child(_pause_button)

	# Centered message, optionally on a cream card.
	var center := CenterContainer.new()
	center.mouse_filter = Control.MOUSE_FILTER_IGNORE
	center.set_anchors_and_offsets_preset(Control.PRESET_FULL_RECT)
	add_child(center)
	_card = PanelContainer.new()
	_card.mouse_filter = Control.MOUSE_FILTER_IGNORE # Taps on the card still resume/retry.
	center.add_child(_card)
	var box := VBoxContainer.new()
	box.alignment = BoxContainer.ALIGNMENT_CENTER
	box.mouse_filter = Control.MOUSE_FILTER_IGNORE
	box.add_theme_constant_override("separation", 14)
	_card.add_child(box)

	_headline = _make_label(64)
	Fonts.make_bold(_headline)
	_new_best = _make_label(40)
	_new_best.text = "NEW BEST!"
	Fonts.make_bold(_new_best)
	_paint(_new_best, Palette.GOLD, 8)
	_details = _make_label(28)
	for label: Label in [_headline, _new_best, _details]:
		label.horizontal_alignment = HORIZONTAL_ALIGNMENT_CENTER
		label.resized.connect(func() -> void: label.pivot_offset = label.size * 0.5)
		box.add_child(label)
	_settings_button = _make_button("Settings (O)")
	_settings_button.size_flags_horizontal = Control.SIZE_SHRINK_CENTER
	_settings_button.pressed.connect(_open_settings)
	box.add_child(_settings_button)

	_settings = _build_settings()
	_settings.visible = false
	add_child(_settings)

	Events.state_changed.connect(_on_state_changed)
	Events.score_changed.connect(_on_score_changed)


func _process(delta: float) -> void:
	_time += delta
	# Title bob and "NEW BEST!" pulse (Art Bible §8).
	var on_title := game.state == Game.State.TITLE
	_headline.rotation = sin(_time * 2.0) * 0.035 if on_title else 0.0
	_headline.scale = Vector2.ONE * (1.0 + 0.03 * sin(_time * 2.6)) if on_title else Vector2.ONE
	_new_best.scale = Vector2.ONE * (1.0 + 0.04 * (sin(_time * 6.0) + 1.0))


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
			_show("SNAKE", "Press any key or tap to play\nArrows / WASD or swipe to move  ·  Esc / P to pause", true, false)
			_headline.add_theme_font_size_override("font_size", 112)
			_paint(_headline, Palette.GOLD, 14)
		Game.State.READY:
			_show("Ready?", "Press a direction or swipe to go!", false, false)
		Game.State.PAUSED:
			_show("Paused", "Press Esc / P or tap to resume", true, true)
		Game.State.GAME_OVER:
			var details := "Score: %d   ·   Best: %d\n\nPress any key or tap to retry" % [game.model.score, SaveData.best]
			_show("PERFECT!" if game.last_run_perfect else "Game Over", details, true, true)
			if game.last_run_perfect:
				_paint(_headline, Palette.UI_ACCENT, 0)
			_new_best.visible = game.last_run_new_best
		_:
			_show("", "", false, false) # PLAYING and DYING: keep the board clear.


## Shows the centered message. `on_card` puts it on a cream panel with brown
## text; otherwise it is white outlined text straight over the board.
func _show(headline: String, details: String, dim: bool, on_card: bool) -> void:
	_headline.text = headline
	_details.text = details
	_headline.visible = headline != ""
	_details.visible = details != ""
	_new_best.visible = false
	_dim.visible = dim
	_card.add_theme_stylebox_override("panel", _card_style if on_card else StyleBoxEmpty.new())
	_headline.add_theme_font_size_override("font_size", 64)
	var text_color := Palette.UI_TEXT if on_card else Color.WHITE
	var outline := 0 if on_card else 10
	_paint(_headline, text_color, outline)
	_paint(_details, text_color, 0 if on_card else 6)
	_settings_button.visible = _settings_allowed()
	_pause_button.visible = game.state == Game.State.PLAYING


func _settings_allowed() -> bool:
	return game.state == Game.State.TITLE or game.state == Game.State.PAUSED


# --- Settings (GDD §2, F15) ----------------------------------------------------------

func _open_settings() -> void:
	AudioManager.play_sfx("ui_click", -6.0, 0.03)
	_settings.visible = true
	_card.visible = false
	_dim.visible = true
	game.input_blocked = true


func _close_settings() -> void:
	AudioManager.play_sfx("ui_click", -6.0, 0.03)
	SaveData.save_data()
	_settings.visible = false
	_card.visible = true
	game.input_blocked = false
	_on_state_changed(game.state)


func _build_settings() -> Control:
	var center := CenterContainer.new()
	center.set_anchors_and_offsets_preset(Control.PRESET_FULL_RECT)
	center.mouse_filter = Control.MOUSE_FILTER_STOP # Clicks outside the panel don't reach the game.

	var panel := PanelContainer.new()
	panel.add_theme_stylebox_override("panel", _panel_style())
	center.add_child(panel)

	var box := VBoxContainer.new()
	box.add_theme_constant_override("separation", 18)
	panel.add_child(box)

	var title := _make_panel_label("Settings", 44)
	Fonts.make_bold(title)
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


# --- Style helpers ---------------------------------------------------------------------

## Cream panel with a brown border, rounded corners and a soft shadow (Art Bible §8).
func _panel_style() -> StyleBoxFlat:
	var style := StyleBoxFlat.new()
	style.bg_color = Palette.UI_PANEL
	style.border_color = Palette.OUTLINE
	style.set_border_width_all(4)
	style.set_corner_radius_all(24)
	style.set_content_margin_all(36)
	style.shadow_color = Color(Palette.OUTLINE, 0.25)
	style.shadow_offset = Vector2(0, 6)
	style.shadow_size = 2
	return style


func _paint(label: Label, color: Color, outline: int) -> void:
	label.add_theme_color_override("font_color", color)
	label.add_theme_color_override("font_outline_color", Palette.OUTLINE)
	label.add_theme_constant_override("outline_size", outline)


func _make_label(font_size: int) -> Label:
	var label := Label.new()
	label.mouse_filter = Control.MOUSE_FILTER_IGNORE
	label.add_theme_font_size_override("font_size", font_size)
	return label


func _make_panel_label(text: String, font_size: int) -> Label:
	var label := _make_label(font_size)
	label.text = text
	_paint(label, Palette.UI_TEXT, 0)
	return label


func _make_button(text: String) -> Button:
	var button := Button.new()
	button.text = text
	button.focus_mode = Control.FOCUS_NONE # Space/Enter stay "any key" for the game.
	button.add_theme_font_size_override("font_size", 26)
	button.add_theme_color_override("font_color", Color.WHITE)
	button.add_theme_color_override("font_hover_color", Color.WHITE)
	button.add_theme_color_override("font_pressed_color", Color.WHITE)
	button.add_theme_color_override("font_outline_color", Palette.OUTLINE)
	button.add_theme_constant_override("outline_size", 6)
	Fonts.make_bold(button)
	button.custom_minimum_size = Vector2(220, 52)
	var normal := StyleBoxFlat.new()
	normal.bg_color = Palette.UI_ACCENT
	normal.border_color = Palette.OUTLINE
	normal.set_border_width_all(3)
	normal.set_corner_radius_all(18)
	normal.set_content_margin_all(10)
	var hover := normal.duplicate() as StyleBoxFlat
	hover.bg_color = Palette.UI_ACCENT.lightened(0.08)
	var pressed := normal.duplicate() as StyleBoxFlat
	pressed.bg_color = Palette.UI_ACCENT.darkened(0.08)
	button.add_theme_stylebox_override("normal", normal)
	button.add_theme_stylebox_override("hover", hover)
	button.add_theme_stylebox_override("pressed", pressed)
	button.add_theme_stylebox_override("focus", StyleBoxEmpty.new())
	return button
