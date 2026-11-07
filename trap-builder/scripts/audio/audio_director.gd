extends Node
## Autoload "AudioDirector" (S20, audio-bible.md). Escuta GameEvents e toca SFX/música.
## A simulação nunca chama áudio. Variação de pitch usa um RNG próprio (nunca o da sim).
## Arquivos: res://audio/sfx/<id>.wav e res://audio/music/<id>.wav (tools/generate_audio.js).

const SFX_DIR := "res://audio/sfx/"
const MUSIC_DIR := "res://audio/music/"
const POOL_SIZE: int = 12
const CROSSFADE: float = 0.7
const BUSES: Dictionary[StringName, float] = {&"Music": -10.0, &"SFX": -4.0, &"UI": -6.0}
## SFX repetitivos ganham ±5% de pitch.
const VARY: Array[StringName] = [&"sfx_pig_oink", &"sfx_capture_pop", &"sfx_push", &"sfx_place", &"sfx_bait_eat"]

var _rng := RandomNumberGenerator.new()
var _streams: Dictionary[StringName, AudioStream] = {}
var _pool: Array[AudioStreamPlayer] = []
var _music_a: AudioStreamPlayer
var _music_b: AudioStreamPlayer
var _music_current: StringName = &""
var _stinger: AudioStreamPlayer
var _fan: AudioStreamPlayer


func _ready() -> void:
	_rng.randomize()
	_ensure_buses()
	_load_dir(SFX_DIR)
	_load_dir(MUSIC_DIR)
	for id in [&"music_blueprint", &"music_action", &"sfx_fan_loop"]:
		_make_looping(_streams.get(id))
	for i in POOL_SIZE:
		_pool.append(_player(&"SFX"))
	_music_a = _player(&"Music")
	_music_b = _player(&"Music")
	_stinger = _player(&"Music")
	_fan = _player(&"SFX")
	_fan.stream = _streams.get(&"sfx_fan_loop")
	_connect_events()


# --- API ---------------------------------------------------------------------

func play(id: StringName, bus: StringName = &"SFX", pitch: float = 1.0) -> void:
	var stream: AudioStream = _streams.get(id)
	if stream == null:
		return
	var p := _free_player()
	p.bus = bus
	p.stream = stream
	var variation: float = 1.0 + _rng.randf_range(-0.05, 0.05) if id in VARY else 1.0
	p.pitch_scale = pitch * variation
	p.play()


func play_music(id: StringName) -> void:
	if id == _music_current:
		return
	_music_current = id
	var old := _music_a if _music_a.playing else _music_b
	var new := _music_b if old == _music_a else _music_a
	var stream: AudioStream = _streams.get(id)
	if stream != null:
		new.stream = stream
		new.volume_db = -40.0
		new.play()
		create_tween().tween_property(new, "volume_db", 0.0, CROSSFADE)
	if old.playing:
		var tw := create_tween()
		tw.tween_property(old, "volume_db", -40.0, CROSSFADE)
		tw.tween_callback(old.stop)


func stop_music() -> void:
	_music_current = &""
	for m in [_music_a, _music_b]:
		if m.playing:
			var tw := create_tween()
			tw.tween_property(m, "volume_db", -40.0, 0.3)
			tw.tween_callback(m.stop)


## Abaixa a música por um instante nos impactos grandes.
func duck(amount_db: float = -6.0, duration: float = 0.5) -> void:
	var bus := AudioServer.get_bus_index(&"Music")
	if bus < 0:
		return
	var base: float = BUSES[&"Music"]
	AudioServer.set_bus_volume_db(bus, base + amount_db)
	var tw := create_tween()
	tw.tween_method(func(v: float) -> void: AudioServer.set_bus_volume_db(bus, v), base + amount_db, base, duration)


# --- eventos -----------------------------------------------------------------

func _connect_events() -> void:
	GameEvents.phase_changed.connect(_on_phase_changed)
	GameEvents.trap_selected.connect(func(_t: TrapData) -> void: play(&"sfx_rotate", &"UI"))
	GameEvents.trap_placed.connect(func(_t: TrapData, _c: Vector2i) -> void: play(&"sfx_place", &"UI"))
	GameEvents.trap_removed.connect(func(_t: TrapData, _c: Vector2i) -> void: play(&"sfx_remove", &"UI"))
	GameEvents.trap_rotated.connect(func(_c: Vector2i) -> void: play(&"sfx_rotate", &"UI"))
	GameEvents.invalid_action.connect(func() -> void: play(&"sfx_invalid", &"UI"))
	GameEvents.trap_triggered.connect(_on_trap_triggered)
	GameEvents.trap_resolved.connect(_on_trap_resolved)
	GameEvents.pig_spawned.connect(func(_id: int) -> void: play(&"sfx_pig_oink"))
	GameEvents.pig_pushed.connect(func(_id: int) -> void: play(&"sfx_push"))
	GameEvents.pig_turned_back.connect(func(_id: int) -> void: play(&"sfx_pig_turn"))
	GameEvents.pig_captured.connect(_on_pig_captured)
	GameEvents.pig_escaped.connect(func(_id: int, _p: PackedInt32Array) -> void: play(&"sfx_escape"))
	GameEvents.fan_count_changed.connect(_on_fan_count_changed)
	GameEvents.wolf_reacted.connect(func(e: StringName) -> void: play(StringName("wolf_%s" % e)))
	GameEvents.wave_finished.connect(_on_wave_finished)
	GameEvents.star_stamped.connect(func(i: int) -> void: play(&"sfx_star", &"UI", [1.0, 1.122, 1.26][clampi(i, 0, 2)]))
	GameEvents.paused_changed.connect(func(paused: bool) -> void: _fan.stream_paused = paused)


func _on_phase_changed(state: int) -> void:
	# 0 BLUEPRINT · 1 TRANSICAO · 2 ACAO · 3 RESULTADO (LevelController.Phase)
	match state:
		0:
			play_music(&"music_blueprint")
		1:
			play(&"sfx_play", &"UI")
			play(&"sfx_transition", &"UI")
			play_music(&"music_action")


func _on_trap_triggered(_id: int, kind: StringName, _cell: Vector2i) -> void:
	match kind:
		&"anvil": play(&"sfx_anvil_whistle")
		&"tnt": play(&"sfx_tnt_fuse")
		&"bait": play(&"sfx_bait_eat")


func _on_trap_resolved(_id: int, kind: StringName, _cell: Vector2i) -> void:
	match kind:
		&"anvil":
			play(&"sfx_anvil_clang")
			duck()
		&"tnt":
			play(&"sfx_tnt_boom")
			duck()


func _on_pig_captured(_id: int, cause: StringName) -> void:
	play(&"sfx_capture_pop")
	if cause == &"ravine":
		play(&"sfx_ravine_fall")


func _on_fan_count_changed(count: int) -> void:
	if count > 0 and _fan.stream != null:
		_fan.volume_db = -14.0 + minf(6.0, 3.0 * (count - 1))
		if not _fan.playing:
			_fan.play()
	else:
		_fan.stop()


func _on_wave_finished(result: SimResult) -> void:
	_fan.stop()
	stop_music()
	_stinger.stream = _streams.get(&"stinger_lose" if result.failed else &"stinger_win")
	if _stinger.stream != null:
		_stinger.play()


# --- infraestrutura ----------------------------------------------------------

func _ensure_buses() -> void:
	for bus_name in BUSES:
		var idx := AudioServer.get_bus_index(bus_name)
		if idx < 0:
			AudioServer.add_bus()
			idx = AudioServer.bus_count - 1
			AudioServer.set_bus_name(idx, bus_name)
			AudioServer.set_bus_send(idx, &"Master")
		AudioServer.set_bus_volume_db(idx, BUSES[bus_name])


func _load_dir(dir_path: String) -> void:
	for file in ResourceLoader.list_directory(dir_path):
		if file.ends_with(".wav") or file.ends_with(".ogg"):
			var id := StringName(file.get_basename())
			_streams[id] = load(dir_path + file)


func _make_looping(stream: AudioStream) -> void:
	var wav := stream as AudioStreamWAV
	if wav != null:
		wav.loop_mode = AudioStreamWAV.LOOP_FORWARD
		wav.loop_begin = 0
		wav.loop_end = int(wav.get_length() * wav.mix_rate)


func _player(bus: StringName) -> AudioStreamPlayer:
	var p := AudioStreamPlayer.new()
	p.bus = bus
	add_child(p)
	return p


func _free_player() -> AudioStreamPlayer:
	for p in _pool:
		if not p.playing:
			return p
	return _pool[0]
