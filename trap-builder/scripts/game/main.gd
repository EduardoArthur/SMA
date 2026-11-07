extends Node
## Raiz do jogo: troca a tela ativa dentro de ScreenHost. Começa no menu.

@onready var _host: Node = $ScreenHost


func _ready() -> void:
	GameState.screen_requested.connect(_show_screen)
	GameState.go_to_menu()


func _show_screen(scene: PackedScene) -> void:
	for child in _host.get_children():
		child.queue_free()
	_host.add_child(scene.instantiate())
