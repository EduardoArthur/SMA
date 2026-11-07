class_name HudPanel
extends PanelContainer
## HUD do topo (S10/S15): fase, moedas, fugas permitidas e contadores da Ação.

var _title: Label
var _coins: Label
var _escapes: Label
var _counts: Label
var _status: Label


func _ready() -> void:
	UiStyle.style_panel(self)
	var row := HBoxContainer.new()
	row.add_theme_constant_override("separation", 28)
	add_child(row)
	_title = UiStyle.label("", 18)
	_coins = UiStyle.label("", 16)
	_escapes = UiStyle.label("", 16)
	_counts = UiStyle.label("", 16)
	_status = UiStyle.label("", 16, UiStyle.DANGER)
	for l: Label in [_title, _coins, _escapes, _counts, _status]:
		row.add_child(l)


func set_level(level: LevelData) -> void:
	_title.text = "Fase %d — %s" % [level.level_id, level.title]
	_escapes.text = "Fugas permitidas: %d" % level.escape_limit


func set_budget(spent: int, budget: int, par: int) -> void:
	_coins.text = "Moedas: %d / %d   (★★★ até %d)" % [spent, budget, par]
	_coins.add_theme_color_override("font_color", UiStyle.INK if spent <= par else UiStyle.DANGER.darkened(0.2))


func set_counts(visible_counts: bool, captured: int = 0, escaped: int = 0, total: int = 0) -> void:
	_counts.visible = visible_counts
	_counts.text = "Jantar: %d / %d   Fugiram: %d" % [captured, total, escaped]


func set_status(text: String) -> void:
	_status.text = text
