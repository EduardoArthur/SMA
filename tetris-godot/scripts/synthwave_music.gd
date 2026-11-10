extends AudioStreamPlayer

const SAMPLE_RATE: int = 22050
const STEP_SECONDS: float = 60.0 / 148.0 / 2.0
const BASS_ROOTS: Array[int] = [40, 45, 45, 40, 50, 45, 40, 45, 45, 48, 50, 40, 45, 48, 45, 40]
# Korobeiniki. Each row is a bar of eighth notes; 0 sustains the preceding note.
const MELODY: Array[PackedInt32Array] = [
	[76, 0, 71, 72, 74, 0, 72, 71],
	[69, 0, 69, 72, 76, 0, 74, 72],
	[71, 0, 0, 72, 74, 0, 76, 0],
	[72, 0, 69, 0, 69, 0, 0, 0],
	[74, 0, 0, 77, 81, 0, 79, 77],
	[76, 0, 0, 72, 76, 0, 74, 72],
	[71, 0, 71, 72, 74, 0, 76, 0],
	[72, 0, 69, 0, 69, 0, 0, 0],
	[76, 0, 0, 0, 72, 0, 0, 0],
	[74, 0, 0, 0, 71, 0, 0, 0],
	[72, 0, 0, 0, 69, 0, 0, 0],
	[68, 0, 0, 0, 71, 0, 0, 0],
	[76, 0, 0, 0, 72, 0, 0, 0],
	[74, 0, 0, 0, 71, 0, 0, 0],
	[72, 0, 76, 0, 81, 0, 0, 0],
	[80, 0, 0, 0, 0, 0, 0, 0]
]


func _ready() -> void:
	var samples_per_step: int = roundi(SAMPLE_RATE * STEP_SECONDS)
	var frame_count: int = samples_per_step * MELODY.size() * 8
	var pcm: PackedByteArray = PackedByteArray()
	pcm.resize(frame_count * 2)
	var lead_hz: float = 0.0
	var lead_start: int = 0
	for step: int in range(MELODY.size() * 8):
		var chord: int = step >> 3
		var beat: int = step % 8
		var root: int = BASS_ROOTS[chord]
		var bass_hz: float = _frequency(root + 12 * (beat % 2))
		var note: int = MELODY[chord][beat]
		if note != 0:
			lead_hz = _frequency(note)
			lead_start = step * samples_per_step
		var next_step: int = (step + 1) % (MELODY.size() * 8)
		var release: bool = MELODY[next_step >> 3][next_step % 8] != 0
		for frame: int in range(samples_per_step):
			var time: float = float(frame) / SAMPLE_RATE
			var fade: float = minf(time * 180.0, 1.0) * minf((STEP_SECONDS - time) * 100.0, 1.0)
			var bass_phase: float = TAU * bass_hz * time
			var sample: float = (sin(bass_phase) + 0.30 * sin(3.0 * bass_phase)) * 0.19 * exp(-5.0 * time)
			if beat % 2 == 0:
				sample += sin(TAU * (47.0 * time + 2.8 * (1.0 - exp(-30.0 * time)))) * 0.29 * exp(-19.0 * time)
			else:
				sample += sin(TAU * 5917.0 * time) * sin(TAU * 3923.0 * time) * 0.06 * exp(-55.0 * time)
			var lead_time: float = float(step * samples_per_step + frame - lead_start) / SAMPLE_RATE
			var lead_phase: float = TAU * lead_hz * lead_time
			var lead_envelope: float = minf(lead_time * 180.0, 1.0)
			if release:
				lead_envelope *= minf((STEP_SECONDS - time) * 100.0, 1.0)
			var lead: float = (sin(lead_phase) + 0.32 * sin(2.0 * lead_phase) + 0.13 * sin(3.0 * lead_phase)) * 0.24
			pcm.encode_s16((step * samples_per_step + frame) * 2, roundi((sample * fade + lead * lead_envelope) * 32767.0))
	var track: AudioStreamWAV = AudioStreamWAV.new()
	track.format = AudioStreamWAV.FORMAT_16_BITS
	track.mix_rate = SAMPLE_RATE
	track.data = pcm
	track.loop_mode = AudioStreamWAV.LOOP_FORWARD
	track.loop_end = frame_count
	stream = track
	play()


func _frequency(note: int) -> float:
	return 440.0 * pow(2.0, (note - 69) / 12.0)
