extends Node
## Best score and settings, saved to user://save.cfg (Technical Design §9.1).
## On web, user:// is stored in the browser (IndexedDB) automatically.

const PATH := "user://save.cfg"

var best := 0
var music_volume := 0.8
var sfx_volume := 0.8
var screen_shake := true


func _ready() -> void:
	load_data()


func load_data() -> void:
	var cf := ConfigFile.new()
	if cf.load(PATH) != OK:
		return # First launch: keep the defaults.
	best = cf.get_value("score", "best", 0)
	music_volume = cf.get_value("settings", "music", 0.8)
	sfx_volume = cf.get_value("settings", "sfx", 0.8)
	screen_shake = cf.get_value("settings", "shake", true)


func save_data() -> void:
	var cf := ConfigFile.new()
	cf.set_value("score", "best", best)
	cf.set_value("settings", "music", music_volume)
	cf.set_value("settings", "sfx", sfx_volume)
	cf.set_value("settings", "shake", screen_shake)
	var err := cf.save(PATH)
	if err != OK:
		push_warning("Could not save %s (error %d)" % [PATH, err])
