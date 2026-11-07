class_name InputSetup
## Registra as ações do Input Map em código (godot-architecture.md §9).
## Evita editar a seção [input] do project.godot à mão. Chamado uma vez pelo GameState.

const TRAP_SLOTS: int = 5


static func ensure_actions() -> void:
	_mouse(&"place", MOUSE_BUTTON_LEFT)
	_mouse(&"rotate", MOUSE_BUTTON_RIGHT)
	_key(&"rotate", KEY_R)
	_key(&"remove", KEY_DELETE)
	_key(&"play", KEY_SPACE)
	_key(&"play", KEY_ENTER)
	_key(&"toggle_speed", KEY_F)
	_key(&"pause", KEY_P)
	_key(&"pause", KEY_ESCAPE)
	_key(&"abort", KEY_BACKSPACE)
	for i in TRAP_SLOTS:
		_key(select_action(i), KEY_1 + i)
	# atalhos de desenvolvimento
	_key(&"debug_ref_plan", KEY_TAB)
	_key(&"clear_plan", KEY_0)
	_key(&"next_level", KEY_N)
	_key(&"prev_level", KEY_B)


static func select_action(slot: int) -> StringName:
	return StringName("select_trap_%d" % (slot + 1))


static func _key(action: StringName, keycode: Key) -> void:
	if not InputMap.has_action(action):
		InputMap.add_action(action)
	var ev := InputEventKey.new()
	ev.keycode = keycode
	InputMap.action_add_event(action, ev)


static func _mouse(action: StringName, button: MouseButton) -> void:
	if not InputMap.has_action(action):
		InputMap.add_action(action)
	var ev := InputEventMouseButton.new()
	ev.button_index = button
	InputMap.action_add_event(action, ev)
