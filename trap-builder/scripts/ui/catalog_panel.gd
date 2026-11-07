class_name CatalogPanel
extends PanelContainer
## Catálogo Catapimba S.A. (S10): lista as armadilhas da fase com preço,
## desabilita o que não cabe no orçamento e tem o botão JOGAR.

signal trap_chosen(slot: int)

var _traps: Array[TrapData] = []
var _buttons: Array[Button] = []
var _items: VBoxContainer
var _play: Button


func _ready() -> void:
	UiStyle.style_panel(self)
	var box := VBoxContainer.new()
	box.add_theme_constant_override("separation", 8)
	add_child(box)
	box.add_child(UiStyle.label("CATÁLOGO", 22))
	box.add_child(UiStyle.label("Catapimba S.A. · Primavera", 12))
	box.add_child(HSeparator.new())
	_items = VBoxContainer.new()
	_items.add_theme_constant_override("separation", 6)
	box.add_child(_items)
	var spacer := Control.new()
	spacer.size_flags_vertical = Control.SIZE_EXPAND_FILL
	box.add_child(spacer)
	box.add_child(UiStyle.label("Dica: clique numa armadilha\nposicionada para removê-la.", 11))
	_play = Button.new()
	_play.text = "JOGAR ▶"
	_play.custom_minimum_size = Vector2(0, 56)
	_play.add_theme_font_size_override("font_size", 22)
	UiStyle.style_button(_play, UiStyle.DANGER.lightened(0.35))
	_play.pressed.connect(GameEvents.play_requested.emit)
	box.add_child(_play)


func setup(traps: Array[TrapData]) -> void:
	_traps = traps
	for b in _buttons:
		b.queue_free()
	_buttons.clear()
	for i in traps.size():
		var t: TrapData = traps[i]
		var b := Button.new()
		b.toggle_mode = true
		b.alignment = HORIZONTAL_ALIGNMENT_LEFT
		b.text = "%d · %s\n      %d moedas" % [i + 1, t.display_name, t.cost]
		b.custom_minimum_size = Vector2(0, 52)
		UiStyle.style_button(b)
		b.pressed.connect(trap_chosen.emit.bind(i))
		_items.add_child(b)
		_buttons.append(b)


func refresh(spent: int, budget: int, selected: TrapData) -> void:
	for i in _buttons.size():
		var t: TrapData = _traps[i]
		_buttons[i].set_pressed_no_signal(t == selected)
		_buttons[i].disabled = spent + t.cost > budget and t != selected
