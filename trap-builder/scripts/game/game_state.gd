extends Node
## Estado global da sessão (autoload "GameState"): fases, fase atual e
## melhor estrela por fase (só em memória no protótipo).

signal screen_requested(scene: PackedScene)

const LEVEL_PATHS: Array[String] = [
	"res://data/levels/level_01.tres",
	"res://data/levels/level_02.tres",
	"res://data/levels/level_03.tres",
	"res://data/levels/level_04.tres",
	"res://data/levels/level_05.tres",
]
const LEVEL_SCREEN: PackedScene = preload("res://scenes/screens/level_screen.tscn")
const MENU_SCREEN: PackedScene = preload("res://scenes/screens/menu_screen.tscn")

var levels: Array[LevelData] = []
var current_index: int = 0
var best_stars: Dictionary[int, int] = {}


func _ready() -> void:
	InputSetup.ensure_actions()
	for path in LEVEL_PATHS:
		if ResourceLoader.exists(path):
			levels.append(load(path) as LevelData)
		else:
			push_warning("Fase não encontrada: %s (rode export_levels.js)" % path)


func current_level() -> LevelData:
	if levels.is_empty():
		return null
	return levels[current_index]


func start_level(index: int) -> void:
	current_index = clampi(index, 0, levels.size() - 1)
	screen_requested.emit(LEVEL_SCREEN)


func go_to_menu() -> void:
	screen_requested.emit(MENU_SCREEN)


func has_next_level() -> bool:
	return current_index < levels.size() - 1


func record_result(level_id: int, stars: int) -> void:
	best_stars[level_id] = maxi(best_stars.get(level_id, 0), stars)
