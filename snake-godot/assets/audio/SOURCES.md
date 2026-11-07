# Audio sources

Keep every jsfxr parameter string and BeepBox song URL here, so any sound can be re-edited later (Audio Bible §2).
The game skips any file that doesn't exist yet, so add sounds one at a time.

## Current files: generated placeholders

Every file below currently comes from **`tools/generate_audio.ps1`**, a small synthesizer that builds them from
basic waveforms (no external assets, so no license issues). To tweak a sound, edit its function in that script and
run it again from the project folder:

    powershell -ExecutionPolicy Bypass -File tools/generate_audio.ps1

To replace a sound with a jsfxr/BeepBox version, save it with the same name. Music and jingles can be
`.ogg` or `.wav`; an `.ogg` wins if both exist, so delete the `.wav` when you replace it. Music loops in code.

## Sound effects → `sfx/` (WAV, 44.1 kHz, 16-bit, normalized to -1 dBFS, no leading silence)

| File | jsfxr starting point | jsfxr string |
|------|---------------------|--------------|
| `sfx_chomp_a.wav`, `_b`, `_c` | Hit/Hurt → Noise, very short, low freq | |
| `sfx_gasp.wav` | Jump → Noise, quick up-slide | |
| `sfx_bonk.wav` | Hit/Hurt → Square, low, fast down-slide | |
| `sfx_slide_down.wav` | Sine, slow negative slide, vibrato, ~0.7 s | |
| `sfx_milestone.wav` | Powerup, shortened | |
| `sfx_speed_up.wav` | Noise, high-pass, short up-slide | |
| `sfx_ui_click.wav` | Blip/Select, a bit lower | |
| `sfx_pop.wav` | Blip/Select → Sine, short up-slide | |

A single file without a suffix (for example `sfx_chomp.wav`) also works.

## Music → `music/` (OGG Vorbis)

| File | Notes | BeepBox URL |
|------|-------|-------------|
| `music_garden_loop.ogg` | 16 bars, 132 BPM, C major. In the Import tab set **Loop = On**. | |
| `jingle_game_over.ogg` | Comic falling "wah-wah", about 2 s | |
| `jingle_new_best.ogg` | Rising fanfare, about 2 s | |
| `jingle_perfect.ogg` | New-best jingle + drum roll + big chord, about 3 s | |
