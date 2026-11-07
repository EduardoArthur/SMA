extends Camera2D
## Screen shake (GDD §6.5, Technical Design §7.4). The camera is fixed at the
## top-left, so the view is unchanged; shaking only moves `offset`.
## Skipped entirely when screen shake is turned off in Settings.

var _strength := 0.0
var _duration := 1.0
var _left := 0.0


func _ready() -> void:
	anchor_mode = Camera2D.ANCHOR_MODE_FIXED_TOP_LEFT
	Events.fruit_eaten.connect(func(_cell: Vector2i, _type: int) -> void: shake(2.0, 0.1))
	# The big death shake lands with the "bonk", after the 0.3 s freeze.
	Events.died.connect(func(_cause: String) -> void: get_tree().create_timer(0.3).timeout.connect(shake.bind(8.0, 0.4)))
	Events.board_cleared.connect(func() -> void: shake(5.0, 0.3))


func shake(strength: float, duration: float) -> void:
	if not SaveData.screen_shake:
		return
	var current := _strength * maxf(_left / _duration, 0.0)
	if strength >= current: # Never let a small shake cut a big one short.
		_strength = strength
		_duration = duration
		_left = duration


func _process(delta: float) -> void:
	if _left > 0.0:
		_left -= delta
		var k := maxf(_left / _duration, 0.0)
		offset = Vector2(randf_range(-1.0, 1.0), randf_range(-1.0, 1.0)) * _strength * k
	else:
		offset = Vector2.ZERO
