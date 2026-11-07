extends Node
## Autoload "AudioDirector". Escuta GameEvents e toca SFX/música.
## Stub até a story S20 (ver 02-design/audio/audio-bible.md).
## Regra: usa um RNG próprio para variação de pitch; nunca o da simulação.

var _rng := RandomNumberGenerator.new()


func _ready() -> void:
	_rng.randomize()
