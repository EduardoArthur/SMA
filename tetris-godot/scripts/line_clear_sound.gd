extends AudioStreamPlayer

const SAMPLE_RATE: int = 22050
const NOTE_DURATION: float = 0.09
const NOTES: Array[float] = [523.25, 659.25, 783.99]


func _ready() -> void:
	var frames_per_note: int = roundi(SAMPLE_RATE * NOTE_DURATION)
	var pcm: PackedByteArray = PackedByteArray()
	pcm.resize(frames_per_note * NOTES.size() * 2)
	for note_index: int in range(NOTES.size()):
		for frame: int in range(frames_per_note):
			var time: float = float(frame) / SAMPLE_RATE
			var envelope: float = minf(time / 0.005, 1.0) * pow(1.0 - time / NOTE_DURATION, 1.5)
			var phase: float = TAU * NOTES[note_index] * time
			var sample: float = (sin(phase) + 0.25 * sin(2.0 * phase)) * envelope * 0.65
			pcm.encode_s16((note_index * frames_per_note + frame) * 2, roundi(sample * 32767.0))
	var effect: AudioStreamWAV = AudioStreamWAV.new()
	effect.format = AudioStreamWAV.FORMAT_16_BITS
	effect.mix_rate = SAMPLE_RATE
	effect.data = pcm
	stream = effect
