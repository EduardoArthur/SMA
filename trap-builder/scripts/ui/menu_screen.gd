extends Control
## Menu: título + seleção de fase com a melhor estrela de cada uma.

func _ready() -> void:
	set_anchors_preset(Control.PRESET_FULL_RECT)
	var bg := ColorRect.new()
	bg.color = UiStyle.PAPER
	bg.set_anchors_preset(Control.PRESET_FULL_RECT)
	add_child(bg)

	var center := CenterContainer.new()
	center.set_anchors_preset(Control.PRESET_FULL_RECT)
	add_child(center)
	var box := VBoxContainer.new()
	box.add_theme_constant_override("separation", 14)
	center.add_child(box)

	var title := UiStyle.label("TRAP BUILDER", 56)
	title.horizontal_alignment = HORIZONTAL_ALIGNMENT_CENTER
	box.add_child(title)
	var sub := UiStyle.label("O Lobo Mau cansou de soprar casas.\nEngenhocas por Catapimba S.A.", 16)
	sub.horizontal_alignment = HORIZONTAL_ALIGNMENT_CENTER
	box.add_child(sub)
	box.add_child(HSeparator.new())

	for i in GameState.levels.size():
		var level: LevelData = GameState.levels[i]
		var best: int = GameState.best_stars.get(level.level_id, 0)
		var b := Button.new()
		b.text = "%d · %s     %s" % [level.level_id, level.title, "★".repeat(best) + "☆".repeat(3 - best)]
		b.custom_minimum_size = Vector2(420, 48)
		b.add_theme_font_size_override("font_size", 18)
		UiStyle.style_button(b)
		b.pressed.connect(GameState.start_level.bind(i))
		box.add_child(b)
