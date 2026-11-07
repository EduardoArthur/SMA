class_name ResultPanel
extends PanelContainer
## Tela de resultado (S15): estrelas carimbadas, gasto/par, fugas e botões.

var _title: Label
var _stars: Array[Label] = []
var _stats: Label
var _hint: Label
var _next: Button


func _ready() -> void:
	UiStyle.style_panel(self)
	custom_minimum_size = Vector2(460, 0)
	var box := VBoxContainer.new()
	box.alignment = BoxContainer.ALIGNMENT_CENTER
	box.add_theme_constant_override("separation", 10)
	add_child(box)
	_title = UiStyle.label("", 30)
	_title.horizontal_alignment = HORIZONTAL_ALIGNMENT_CENTER
	box.add_child(_title)
	var row := HBoxContainer.new()
	row.alignment = BoxContainer.ALIGNMENT_CENTER
	row.add_theme_constant_override("separation", 18)
	box.add_child(row)
	for i in 3:
		var s := UiStyle.label("★", 64, UiStyle.STAR_OFF)
		s.pivot_offset = Vector2(24, 40)
		row.add_child(s)
		_stars.append(s)
	_stats = UiStyle.label("", 16)
	_stats.horizontal_alignment = HORIZONTAL_ALIGNMENT_CENTER
	box.add_child(_stats)
	_hint = UiStyle.label("", 13)
	_hint.horizontal_alignment = HORIZONTAL_ALIGNMENT_CENTER
	box.add_child(_hint)
	var buttons := HBoxContainer.new()
	buttons.alignment = BoxContainer.ALIGNMENT_CENTER
	buttons.add_theme_constant_override("separation", 8)
	box.add_child(buttons)
	buttons.add_child(_button("Repetir", GameEvents.retry_requested.emit))
	buttons.add_child(_button("Ajustar", GameEvents.adjust_requested.emit))
	_next = _button("Próxima", GameEvents.next_requested.emit)
	buttons.add_child(_next)
	buttons.add_child(_button("Menu", GameState.go_to_menu))
	hide()


func show_result(result: SimResult, level: LevelData, has_next: bool) -> void:
	_title.text = "FUGIRAM!" if result.failed else "JANTAR SERVIDO!"
	_title.add_theme_color_override("font_color", UiStyle.DANGER if result.failed else UiStyle.INK)
	_stats.text = "Capturados %d de %d  ·  Fugas %d / %d permitidas\nGasto %d moedas  ·  par %d" % [
		result.captured, result.captured + result.escaped, result.escaped, level.escape_limit, result.cost, level.par]
	match result.stars:
		0: _hint.text = "Muitos porcos escaparam. Ajuste o plano!"
		1: _hint.text = "Para ★★: não deixe nenhum porco fugir."
		2: _hint.text = "Para ★★★: zero fugas gastando até %d moedas." % level.par
		_: _hint.text = "Banquete perfeito!"
	_next.disabled = result.failed or not has_next
	show()
	for i in 3:
		var s := _stars[i]
		s.add_theme_color_override("font_color", UiStyle.STAR_OFF)
		s.scale = Vector2.ONE
	if result.stars == 0:
		return
	var tw := create_tween()
	for i in result.stars:
		var s := _stars[i]
		tw.tween_interval(0.15)
		tw.tween_callback(s.add_theme_color_override.bind("font_color", UiStyle.STAR_ON))
		tw.tween_callback(GameEvents.star_stamped.emit.bind(i))
		tw.tween_property(s, "scale", Vector2(1.5, 1.5), 0.0)
		tw.tween_property(s, "scale", Vector2.ONE, 0.2)


func _button(text: String, action: Callable) -> Button:
	var b := Button.new()
	b.text = text
	b.custom_minimum_size = Vector2(96, 44)
	UiStyle.style_button(b)
	b.pressed.connect(action)
	return b
