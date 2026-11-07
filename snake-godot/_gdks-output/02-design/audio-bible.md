# Snake - Audio Bible

> **Author:** Anton (Audio Director), GD-KS Design Team
> **Date:** 2026-11-06 · **Version:** 1.0
> **Production:** sound effects made in **jsfxr** (free, in the browser; the generated sounds are yours) · music and jingles made in **BeepBox** (free, in the browser)
> **Implements:** GDD §6.5 feedback table, Technical Design §9.3 (`AudioManager`, buses)

---

## 1. Sonic Identity

**"A cartoon snake in a toy garden."** Bouncy, chiptune-flavoured, exaggerated, never harsh.

| # | Principle | In practice |
|---|-----------|-------------|
| S1 | **Sound sells the chomp** | Eating is the most frequent and most satisfying sound. Spend the most time on it. |
| S2 | **Cartoon timing** | Short attack, quick decay. Every effect is ≤ 0.4 s except the death and celebration sounds. |
| S3 | **Silence is a tool** | Music cuts out at death. The 0.3 s freeze is *silent*, then the bonk lands. |
| S4 | **Never annoying** | The chomp plays hundreds of times per session: soft high frequencies, pitch variation, no shrill tones. |

---

## 2. Tools & Workflow

| Tool | Use | Link |
|------|-----|------|
| **jsfxr** | All sound effects | sfxr.me |
| **BeepBox** | Music loop + 3 jingles (same instruments, so they all match) | beepbox.co |
| **Audacity** (optional) | Trim silence, normalize, fade | audacityteam.org |

**Export workflow**
1. jsfxr → tweak → **Export WAV** (44.1 kHz, 16-bit).
2. BeepBox → File → **Export → WAV**. Then convert to **OGG** in Audacity (File → Export → OGG, quality 5) for music only.
3. Normalize every file to **-1 dBFS peak** (Audacity → Effect → Normalize) so levels are set in Godot, not in the files.
4. Trim leading silence. **Sound must start at 0 ms**, because any delay feels like input lag.

> 💾 Save the **jsfxr parameter string** (the "Copy" or permalink button) and the **BeepBox song URL** in `assets/audio/SOURCES.md`, so you can tweak any sound later.

---

## 3. Sound Effects Catalog

| ID / file | Event (GDD §6.1) | Description | jsfxr starting point | Length | Volume | Pitch variation |
|-----------|------------------|-------------|----------------------|--------|--------|-----------------|
| `sfx_chomp.wav` | `fruit_eaten` | Crunchy little bite: a short noise burst with a low "thump" | **Hit/Hurt** preset → Wave **Noise**, sustain very short, start freq low (~25%), slight down-slide. Export 2-3 variants (`_a`, `_b`, `_c`). | 0.12 s | 0 dB | **±8%** random, plus a random variant |
| `sfx_gasp.wav` | `near_miss` | Quick rising "hhuh!" breath | **Jump** preset → Wave **Noise**, quick up-slide, very short sustain, low-pass about 60% | 0.2 s | -2 dB | ±5% |
| `sfx_bonk.wav` | `died` (part 1) | Cartoon head bump: hollow, low | **Hit/Hurt** preset → Wave **Square**, low start freq, fast down-slide, short | 0.15 s | 0 dB | none |
| `sfx_slide_down.wav` | `died` (part 2, +0.15 s after the bonk) | Slide whistle going down, "wheeeoooo" | Blank → Wave **Sine**, start freq high, **slide negative** (slow), sustain ~0.6 s, add a little **vibrato** | 0.7 s | -3 dB | none |
| `sfx_milestone.wav` | `milestone_reached` | Bright upward 3-note "ta-da-ding" | **Powerup** preset, shorten, or a 1-bar BeepBox jingle | 0.5 s | -2 dB | none |
| `sfx_speed_up.wav` *(optional)* | `speed_changed` | Tiny "whoosh" | Blank → Noise, high-pass about 40%, up-slide, very short | 0.15 s | -8 dB | ±5% |
| `sfx_ui_click.wav` | Buttons, start / restart | Soft "blip" | **Blip/Select** preset, lower the frequency a little | 0.06 s | -6 dB | ±3% |
| `sfx_pop.wav` | Fruit spawns | Tiny bubble "plip" | **Blip/Select** → Wave **Sine**, short up-slide | 0.08 s | **-12 dB** (very subtle) | ±10% |

**The turn sound is cut** (GDD lists it as optional). At 15 moves/sec with many turns it gets tiring fast (S4).

---

## 4. Music & Jingles (BeepBox)

### 4.1 `music_garden_loop.ogg`: main gameplay loop
| Property | Value |
|----------|-------|
| Mood | Bouncy, cheerful, slightly mischievous (a "sneaking for snacks" feeling) |
| Tempo | **132 BPM** |
| Key / Scale | **C major** (BeepBox scale "Major"), so it's hard to sound wrong |
| Length | **16 bars** (about 29 s), loops seamlessly |
| Instruments (4 channels) | Lead: **square/chip lead** (melody) · Bass: **triangle/"chip bass"** (bouncy octave jumps) · Chords: soft **pulse/"chip wave"** off-beat stabs · Drums: **noise** kick, snare and hi-hat |
| Structure | Bars 1-8: A theme · bars 9-16: A' (same melody, an octave up or with harmony) |
| Loop tip | Make sure the last bar leads back into bar 1. Export the full 16 bars **without a fade-out**. |
| Level | Sits **behind** the effects (see §6). Keep the lead melody in the middle register so it doesn't fight with the chomp. |

### 4.2 Jingles (same BeepBox instruments, no drums or a light hit)
| File | Event | Description | Length |
|------|-------|-------------|--------|
| `jingle_game_over.ogg` | Game Over screen appears | Sad-funny "wah-wah-wah-waaah" falling 4 notes (comedy, not sadness) | about 2 s |
| `jingle_new_best.ogg` | `new_best` | Triumphant short melody going up, ends on a high held note | about 2 s |
| `jingle_perfect.ogg` | `board_cleared` | The new-best jingle **plus** a drum roll and a big final chord | about 3 s |

Only **one** jingle ever plays per run end, by priority: `jingle_perfect` (board cleared) > `jingle_new_best` > `jingle_game_over`.

### 4.3 Title screen
Uses the same `music_garden_loop` with a **low-pass filter** (see §5), so it sounds like it's "playing in the next room". It opens up when the run starts. This means one track does two jobs.

---

## 5. Dynamic Audio (cheap and effective)

| Situation | Behaviour | How (Godot) |
|-----------|-----------|-------------|
| **Title / Paused** | Music is muffled | `AudioEffectLowPassFilter` on the **Music** bus, cutoff about 800 Hz, enabled with `AudioServer.set_bus_effect_enabled()` |
| **Playing** | Music is clear | Filter off |
| **Speed ramp** | Music gets subtly faster *and* higher: things get frantic | `music_player.pitch_scale = remap(mps, 7.0, 15.0, 1.0, 1.08)`, tweened over 0.5 s at each `speed_changed` |
| **Death** | Music **stops instantly** at the freeze (S3) → silence 0.3 s → bonk → slide whistle → game over jingle | `music_player.stop()` on `died` |
| **Restart** | Music starts again from the top, `pitch_scale = 1.0` | On `run_started` |
| **Window / tab not focused** | Everything pauses (the game pauses already). Audio follows the game pause. | `AudioManager` sets `process_mode = PROCESS_MODE_PAUSABLE` for gameplay SFX. Music players use `PROCESS_MODE_ALWAYS` and are handled by the low-pass filter. |

> ⚠️ Pitch-scaling music above about 1.1 sounds like chipmunks. Keep the cap at **1.08**.

---

## 6. Mixing

### 6.1 Buses (create them in the Godot **Audio** tab)
```
Master (0 dB)
├── Music  (-10 dB default)   [LowPassFilter, disabled by default]
└── SFX    (-4 dB default)
```
- Settings sliders (Music / SFX) set the bus volume: `AudioServer.set_bus_volume_db(idx, linear_to_db(value))`. When the value is 0, **mute** the bus (`set_bus_mute`) instead of using -inf dB.
- Optional: add an `AudioEffectLimiter` on **Master** (ceiling -1 dB) so overlapping chomps and jingles never clip.

### 6.2 Priority & polyphony
| Rule | Value |
|------|-------|
| SFX player pool | **4** `AudioStreamPlayer` nodes (Technical Design §9.3) |
| Same sound playing more than twice at once | Skip the new one (avoids a chomp pile-up at high speed) |
| Priority when the pool is full | Death > jingle > milestone > gasp > chomp > speed > pop > UI |

### 6.3 Relative loudness (perceived, after bus levels)
Death sounds and jingles are the loudest. Then the chomp and gasp. The music sits under the effects. The pop and speed whoosh are barely there.

---

## 7. Godot Import Settings

| File type | Format | Import settings |
|-----------|--------|-----------------|
| SFX | **WAV** (16-bit, mono is fine) | Loop: **Disabled**. Compress mode: **QOA** (default in Godot 4.3+, good for web size). |
| Music & jingles | **OGG Vorbis** | Music loop: **Loop = On**. Jingles: Loop = Off. |

**Web notes**
- Browsers block audio until the first click, tap or key press. The title screen's "press any key / tap" is that first input, so **start the music there** (Technical Design §9.3).
- Keep the total audio under about **2 MB** (16 bars of OGG at quality 5 is about 0.5 MB, the effects are tiny).

---

## 8. File Layout & Naming

```
res://assets/audio/
├── SOURCES.md                # jsfxr strings + BeepBox URLs (to re-edit later)
├── sfx/
│   ├── sfx_chomp_a.wav  sfx_chomp_b.wav  sfx_chomp_c.wav
│   ├── sfx_gasp.wav  sfx_bonk.wav  sfx_slide_down.wav
│   ├── sfx_milestone.wav  sfx_speed_up.wav
│   └── sfx_ui_click.wav  sfx_pop.wav
└── music/
    ├── music_garden_loop.ogg
    ├── jingle_game_over.ogg
    ├── jingle_new_best.ogg
    └── jingle_perfect.ogg
```
Naming: `lower_snake_case`, prefix `sfx_` / `music_` / `jingle_`, variants `_a/_b/_c`.

---

## 9. Event → Sound Map (for `AudioManager`)

| Signal | Sound(s) | Notes |
|--------|----------|-------|
| `run_started` | start `music_garden_loop` (pitch 1.0), low-pass off | |
| `fruit_eaten` | `sfx_chomp_[a/b/c]` ±8% | After a short delay (≈ 0.05 s), `sfx_pop` for the new fruit |
| `near_miss` | `sfx_gasp` | Respects the near-miss cooldown already in `Game` |
| `milestone_reached` | `sfx_milestone` | |
| `speed_changed` | `sfx_speed_up` (optional) + music pitch tween | |
| `died` | stop music → wait 0.3 s → `sfx_bonk` → +0.15 s `sfx_slide_down` | |
| *Game Over shown* | `jingle_game_over` **or** `jingle_new_best` | Never both |
| `board_cleared` | stop music → `jingle_perfect` | |
| `state_changed` → TITLE / PAUSED | low-pass on | |
| UI buttons | `sfx_ui_click` | |

---

## 10. Production Checklist

- [ ] 8 sound effects (+2 chomp variants) made in jsfxr, trimmed, normalized to -1 dBFS
- [ ] Music loop (16 bars, 132 BPM) made in BeepBox and exported as OGG with Loop on
- [ ] 3 jingles in the same BeepBox instruments
- [ ] `SOURCES.md` saved with every jsfxr string and BeepBox URL
- [ ] Buses Master / Music / SFX created, with the low-pass filter on Music
- [ ] Tested on web: music starts after the first input, no clipping when chomping fast at 15 moves/sec
- [ ] Listening test: play 5 minutes. Is anything annoying? Lower it or vary it more.

**Tip:** the Engine phase can start with **silent placeholders**. The `AudioManager` can skip any file that doesn't exist yet, so audio production can happen in parallel with coding.

---
*Audio Bible by Anton, GD-KS Design Team. Next: Design handoff (Dylan) → Engine Team.*
