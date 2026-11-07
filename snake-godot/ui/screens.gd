extends CanvasLayer
## Basic HUD and screens for M2 (GDD §2): score, best, and one centered message
## for Title / Ready / Paused / Game Over. Built in code on purpose; M7 replaces
## it with themed scenes (Art Bible §8).

@onready var game: Game = owner as Game

var _score: Label
var _best: Label
var _dim: ColorRect
var _headline: Label
var _details: Label


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

	var box := VBoxContainer.new()
	box.alignment = BoxContainer.ALIGNMENT_CENTER
	box.mouse_filter = Control.MOUSE_FILTER_IGNORE
	box.set_anchors_and_offsets_preset(Control.PRESET_FULL_RECT)
	add_child(box)
	_headline = _make_label(72)
	_details = _make_label(28)
	for label: Label in [_headline, _details]:
		label.horizontal_alignment = HORIZONTAL_ALIGNMENT_CENTER
		box.add_child(label)

	Events.state_changed.connect(_on_state_changed)
	Events.score_changed.connect(_on_score_changed)


func _make_label(font_size: int) -> Label:
	var label := Label.new()
	label.add_theme_font_size_override("font_size", font_size)
	label.add_theme_color_override("font_color", Color.WHITE)
	label.add_theme_color_override("font_outline_color", Palette.OUTLINE)
	label.add_theme_constant_override("outline_size", maxi(4, int(font_size * 0.2)))
	return label


func _on_score_changed(score: int) -> void:
	_score.text = "Score: %d" % score


func _on_state_changed(_state: int) -> void:
	_best.text = "Best: %d" % SaveData.best
	match game.state:
		Game.State.TITLE:
			_show("SNAKE", "Press any key or click to play\nArrows / WASD to move  ·  Esc / P to pause")
		Game.State.READY:
			_show("Ready?", "Press a direction to go!", false)
		Game.State.PAUSED:
			_show("Paused", "Press Esc / P or click to resume")
		Game.State.GAME_OVER:
			var details := "Score: %d" % game.model.score
			if game.last_run_new_best:
				details += "\nNEW BEST!"
			details += "\n\nPress any key or click to retry"
			_show("PERFECT!" if game.last_run_perfect else "Game Over", details)
		_:
			_show("", "", false) # PLAYING and DYING: keep the board clear.


func _show(headline: String, details: String, dim := true) -> void:
	_headline.text = headline
	_details.text = details
	_headline.visible = headline != ""
	_details.visible = details != ""
	_dim.visible = dim
