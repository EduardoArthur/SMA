class_name Juice
extends Node2D
## Board effects (GDD §6.5, Art Bible §9): chomp bursts, "+10" pop-ups,
## confetti and speed lines. Sits between the lawn and the fruit/snake in the
## tree, so every effect is drawn UNDER the snake (readability rule).

@onready var game: Game = owner as Game
@onready var board: BoardView = get_parent() as BoardView

static var _dot: Texture2D
var _speed_t := 99.0 ## Time since the last speed-up (speed lines show for 0.25 s).


func _ready() -> void:
	Events.fruit_eaten.connect(_on_fruit_eaten)
	Events.milestone_reached.connect(_on_milestone)
	Events.speed_changed.connect(func(_mps: float) -> void: _speed_t = 0.0)
	Events.board_cleared.connect(_on_board_cleared)
	Events.state_changed.connect(_on_state_changed)


func _process(delta: float) -> void:
	_speed_t += delta
	if _speed_t < 0.3:
		queue_redraw()


# --- Event handlers -------------------------------------------------------------

func _on_fruit_eaten(cell: Vector2i, fruit_type: int) -> void:
	var c := board.cell
	var pos := board.cell_to_pos(Vector2(cell))
	var color: Color = Palette.FRUIT[fruit_type]
	_burst(pos, color, 10, c * 4.0, c * 0.16)
	_burst(pos, Color.WHITE, 3, c * 3.0, c * 0.16)
	_popup(pos, "+%d" % game.config.points_per_fruit)


func _on_state_changed(_state: int) -> void:
	if game.state == Game.State.READY:
		_clear() # A new run starts with a clean board.


func _on_milestone(_length: int) -> void:
	_confetti(board.cell_to_pos(Vector2(game.model.body[0])), 30)


func _on_board_cleared() -> void:
	_confetti(Vector2(board.grid_size) * board.cell * 0.5, 60)


# --- Effects ----------------------------------------------------------------------

## A one-shot burst of round particles that shrink to nothing.
func _burst(pos: Vector2, color: Color, amount: int, speed: float, diameter: float) -> void:
	var p := CPUParticles2D.new()
	p.position = pos
	p.texture = _dot_texture()
	p.one_shot = true
	p.explosiveness = 1.0
	p.amount = amount
	p.lifetime = 0.35
	p.spread = 180.0
	p.initial_velocity_min = speed * 0.5
	p.initial_velocity_max = speed
	p.gravity = Vector2.ZERO
	p.damping_min = speed * 2.0
	p.damping_max = speed * 2.0
	p.scale_amount_min = diameter / 32.0
	p.scale_amount_max = diameter / 32.0
	p.scale_amount_curve = _shrink_curve()
	p.color = color
	_spawn(p)


## Falling, spinning paper rectangles in the palette colors.
func _confetti(pos: Vector2, amount: int) -> void:
	var c := board.cell
	var colors := Gradient.new()
	colors.interpolation_mode = Gradient.GRADIENT_INTERPOLATE_CONSTANT
	colors.offsets = PackedFloat32Array([0.0, 0.2, 0.4, 0.6, 0.8])
	colors.colors = PackedColorArray([Palette.GOLD, Palette.UI_ACCENT, Palette.FRUIT[2], Palette.SWEAT, Palette.LEAF])
	var p := CPUParticles2D.new()
	p.position = pos
	p.one_shot = true
	p.explosiveness = 0.9
	p.amount = amount
	p.lifetime = 1.2
	p.direction = Vector2.UP
	p.spread = 70.0
	p.initial_velocity_min = c * 6.0
	p.initial_velocity_max = c * 11.0
	p.gravity = Vector2(0.0, c * 18.0)
	p.angle_min = -180.0
	p.angle_max = 180.0
	p.angular_velocity_min = -360.0
	p.angular_velocity_max = 360.0
	p.scale_amount_min = c * 0.1
	p.scale_amount_max = c * 0.16
	p.color_initial_ramp = colors
	_spawn(p)


func _popup(pos: Vector2, text: String) -> void:
	var c := board.cell
	var label := Label.new()
	label.text = text
	label.horizontal_alignment = HORIZONTAL_ALIGNMENT_CENTER
	label.vertical_alignment = VERTICAL_ALIGNMENT_CENTER
	label.add_theme_font_size_override("font_size", int(c * 0.6))
	label.add_theme_color_override("font_color", Color.WHITE)
	label.add_theme_color_override("font_outline_color", Palette.OUTLINE)
	label.add_theme_constant_override("outline_size", maxi(4, int(c * 0.15)))
	label.size = Vector2(c * 3.0, c)
	label.position = pos - label.size * 0.5
	label.mouse_filter = Control.MOUSE_FILTER_IGNORE
	Fonts.make_bold(label)
	add_child(label)
	var tw := label.create_tween().set_parallel()
	tw.tween_property(label, "position:y", label.position.y - c, 0.6).set_ease(Tween.EASE_OUT).set_trans(Tween.TRANS_CUBIC)
	tw.tween_property(label, "modulate:a", 0.0, 0.6).set_ease(Tween.EASE_IN)
	tw.chain().tween_callback(label.queue_free)


func _spawn(p: CPUParticles2D) -> void:
	p.finished.connect(p.queue_free)
	add_child(p)
	p.emitting = true


func _clear() -> void:
	for child in get_children():
		child.queue_free()


## Speed lines: short white streaks on both sides of the head, trailing behind.
func _draw() -> void:
	if _speed_t >= 0.25 or game.model == null or game.model.body.is_empty():
		return
	var model := game.model
	var c := board.cell
	var from := Vector2(model.prev_body[0]) if not model.prev_body.is_empty() else Vector2(model.body[0])
	var head := board.cell_to_pos(from.lerp(Vector2(model.body[0]), game.tick_alpha))
	var back := -Vector2(model.direction)
	var side := Vector2(back.y, -back.x)
	var col := Color(1, 1, 1, 0.6 * (1.0 - _speed_t / 0.25))
	for k: float in [-0.85, -0.6, 0.6, 0.85]:
		var start := head + back * c * 0.4 + side * c * k
		draw_line(start, start + back * c * (0.7 - absf(k) * 0.3), col, maxf(2.0, c * 0.07), true)


# --- Shared resources ----------------------------------------------------------------

## A soft white circle, generated once, used by every round particle.
static func _dot_texture() -> Texture2D:
	if _dot == null:
		var size := 32
		var img := Image.create_empty(size, size, false, Image.FORMAT_RGBA8)
		var center := Vector2(size, size) * 0.5
		for y in size:
			for x in size:
				var d := Vector2(x + 0.5, y + 0.5).distance_to(center)
				img.set_pixel(x, y, Color(1, 1, 1, clampf(size * 0.5 - d, 0.0, 1.0)))
		_dot = ImageTexture.create_from_image(img)
	return _dot


static func _shrink_curve() -> Curve:
	var curve := Curve.new()
	curve.add_point(Vector2(0.0, 1.0))
	curve.add_point(Vector2(1.0, 0.0))
	return curve
