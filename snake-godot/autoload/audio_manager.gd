extends Node
## Plays every sound and the music in response to Events (Audio Bible §5-9).
## Audio files that don't exist yet are simply skipped, so the game runs fine
## with silent placeholders while the real sounds are being made.
## Buses (Master -> Music, SFX) are created here if they don't exist yet.

const SFX_DIR := "res://assets/audio/sfx/"
const MUSIC_DIR := "res://assets/audio/music/"
const SFX_NAMES := ["chomp", "gasp", "bonk", "slide_down", "milestone", "speed_up", "ui_click", "pop"]
const JINGLE_NAMES := ["jingle_game_over", "jingle_new_best", "jingle_perfect"]
const POOL_SIZE := 4
## When all players are busy, a sound may replace a less important one (Audio Bible §6.2).
const PRIORITY := {"ui_click": 0, "pop": 1, "speed_up": 2, "chomp": 3, "gasp": 4, "milestone": 5, "bonk": 7, "slide_down": 7}
const MUSIC_BASE_DB := -10.0
const SFX_BASE_DB := -4.0
const LOWPASS_HZ := 800.0
const MAX_MUSIC_PITCH := 1.08 ## Above ~1.1 the music sounds like chipmunks.

var config: GameConfig = preload("res://config/game_config.tres")

var _sounds := {} ## name -> Array of AudioStream variants (_a/_b/_c)
var _jingles := {} ## name -> AudioStream
var _music_stream: AudioStream
var _pool: Array[AudioStreamPlayer] = []
var _pool_names: Array[String] = []
var _music: AudioStreamPlayer
var _jingle: AudioStreamPlayer
var _music_bus := 0
var _sfx_bus := 0
var _pitch_tween: Tween
var _perfect := false
var _new_best := false


func _ready() -> void:
	_music_bus = _ensure_bus(&"Music")
	_sfx_bus = _ensure_bus(&"SFX")
	if AudioServer.get_bus_effect_count(_music_bus) == 0:
		var lowpass := AudioEffectLowPassFilter.new()
		lowpass.cutoff_hz = LOWPASS_HZ
		AudioServer.add_bus_effect(_music_bus, lowpass)
	_set_lowpass(false)

	for sfx_name in SFX_NAMES:
		var variants: Array[AudioStream] = []
		for suffix in ["", "_a", "_b", "_c"]:
			var stream := _load_if_exists(SFX_DIR + "sfx_%s%s.wav" % [sfx_name, suffix])
			if stream != null:
				variants.append(stream)
		_sounds[sfx_name] = variants
	for jingle_name in JINGLE_NAMES:
		_jingles[jingle_name] = _load_if_exists(MUSIC_DIR + jingle_name + ".ogg")
	_music_stream = _load_if_exists(MUSIC_DIR + "music_garden_loop.ogg")

	for i in POOL_SIZE:
		var player := AudioStreamPlayer.new()
		player.bus = &"SFX"
		add_child(player)
		_pool.append(player)
		_pool_names.append("")
	_music = AudioStreamPlayer.new()
	_music.bus = &"Music"
	add_child(_music)
	_jingle = AudioStreamPlayer.new()
	_jingle.bus = &"Music"
	add_child(_jingle)

	apply_volumes()

	Events.run_started.connect(_on_run_started)
	Events.fruit_eaten.connect(_on_fruit_eaten)
	Events.near_miss.connect(func(_cell: Vector2i) -> void: play_sfx("gasp", -2.0, 0.05))
	Events.milestone_reached.connect(func(_length: int) -> void: play_sfx("milestone", -2.0))
	Events.speed_changed.connect(_on_speed_changed)
	Events.died.connect(_on_died)
	Events.board_cleared.connect(_on_board_cleared)
	Events.new_best.connect(func(_score: int) -> void: _new_best = true)
	Events.state_changed.connect(_on_state_changed)


## Applies the Settings volumes (0..1) on top of each bus's base level.
func apply_volumes() -> void:
	_apply_bus(_music_bus, MUSIC_BASE_DB, SaveData.music_volume)
	_apply_bus(_sfx_bus, SFX_BASE_DB, SaveData.sfx_volume)


## Plays one sound effect. `pitch_var` is the random pitch range (0.08 = ±8%).
func play_sfx(sfx_name: String, volume_db := 0.0, pitch_var := 0.0) -> void:
	var variants: Array = _sounds.get(sfx_name, [])
	if variants.is_empty():
		return # File not made yet: silent placeholder.
	var same := 0
	for i in POOL_SIZE:
		if _pool[i].playing and _pool_names[i] == sfx_name:
			same += 1
	if same >= 2:
		return # Avoid a pile-up of the same sound at high speed.
	var idx := -1
	for i in POOL_SIZE:
		if not _pool[i].playing:
			idx = i
			break
	if idx == -1: # All busy: replace the least important sound, if it matters less.
		var lowest: int = PRIORITY.get(sfx_name, 0)
		for i in POOL_SIZE:
			var p: int = PRIORITY.get(_pool_names[i], 0)
			if p < lowest:
				lowest = p
				idx = i
		if idx == -1:
			return
	var player := _pool[idx]
	player.stream = variants.pick_random()
	player.volume_db = volume_db
	player.pitch_scale = 1.0 + randf_range(-pitch_var, pitch_var)
	player.play()
	_pool_names[idx] = sfx_name


# --- Event handlers -------------------------------------------------------------

func _on_run_started() -> void:
	_perfect = false
	_new_best = false
	_jingle.stop()
	_play_music()


func _on_fruit_eaten(_cell: Vector2i, _type: int) -> void:
	play_sfx("chomp", 0.0, 0.08)
	get_tree().create_timer(0.05).timeout.connect(play_sfx.bind("pop", -12.0, 0.1))


func _on_speed_changed(moves_per_sec: float) -> void:
	play_sfx("speed_up", -8.0, 0.05)
	var target := clampf(remap(moves_per_sec, config.speed_start, config.speed_cap, 1.0, MAX_MUSIC_PITCH), 1.0, MAX_MUSIC_PITCH)
	if _pitch_tween != null:
		_pitch_tween.kill()
	_pitch_tween = create_tween()
	_pitch_tween.tween_property(_music, "pitch_scale", target, 0.5)


func _on_died(_cause: String) -> void:
	_stop_music() # Silence is a tool: the 0.3 s freeze is silent, then the bonk.
	get_tree().create_timer(0.3).timeout.connect(play_sfx.bind("bonk"))
	get_tree().create_timer(0.45).timeout.connect(play_sfx.bind("slide_down", -3.0))


func _on_board_cleared() -> void:
	_perfect = true
	_stop_music()


func _on_state_changed(state: int) -> void:
	match state:
		Game.State.TITLE:
			_set_lowpass(true) # "Playing in the next room".
			if not _music.playing:
				_play_music()
		Game.State.PAUSED:
			_set_lowpass(true)
		Game.State.READY:
			play_sfx("ui_click", -6.0, 0.03)
			_set_lowpass(false)
		Game.State.PLAYING:
			_set_lowpass(false)
		Game.State.GAME_OVER:
			# Only one jingle per run end: perfect > new best > game over.
			var jingle := "jingle_perfect" if _perfect else ("jingle_new_best" if _new_best else "jingle_game_over")
			var stream: AudioStream = _jingles.get(jingle)
			if stream != null:
				_jingle.stream = stream
				_jingle.play()


# --- Helpers ----------------------------------------------------------------------

func _play_music() -> void:
	if _music_stream == null:
		return
	if _pitch_tween != null:
		_pitch_tween.kill()
	_music.stream = _music_stream
	_music.pitch_scale = 1.0
	_music.play()


func _stop_music() -> void:
	if _pitch_tween != null:
		_pitch_tween.kill()
	_music.stop()


func _set_lowpass(on: bool) -> void:
	AudioServer.set_bus_effect_enabled(_music_bus, 0, on)


func _apply_bus(idx: int, base_db: float, volume: float) -> void:
	AudioServer.set_bus_mute(idx, volume <= 0.001)
	AudioServer.set_bus_volume_db(idx, base_db + linear_to_db(maxf(volume, 0.001)))


func _ensure_bus(bus_name: StringName) -> int:
	var idx := AudioServer.get_bus_index(bus_name)
	if idx == -1:
		AudioServer.add_bus()
		idx = AudioServer.bus_count - 1
		AudioServer.set_bus_name(idx, bus_name)
		AudioServer.set_bus_send(idx, &"Master")
		AudioServer.set_bus_volume_db(idx, 0.0)
	return idx


func _load_if_exists(path: String) -> AudioStream:
	if ResourceLoader.exists(path):
		return load(path) as AudioStream
	return null
