# Snake - Design Handoff

> **From:** Design Team (Diana, Theo, Aurora, Anton, Dylan)
> **To:** Engine Team, Godot 4 (Érico, Guilherme, Gabi, Gina)
> **Date:** 2026-11-06
> **Status:** ✅ Ready for the Engine phase
> **Track:** Standard (Ideation → Design → Engine). There is no Planning phase: the build order below replaces sprints and stories.

---

## 1. The game in 30 seconds

**Snake: Same old rules, a very dramatic little snake.** Classic single-player Snake on a grid. The rules are **locked and classic**. All the personality comes from a cute cartoon snake drawn in code, with expressive faces, bouncy effects and comic sounds.

- **Engine:** Godot 4.7, GDScript, GL Compatibility renderer
- **Platforms:** PC (keyboard) + Web (keyboard and touch swipe)
- **Team:** solo developer, new to Godot · **Timebox:** 1-4 weeks

---

## 2. Document Set

| Document | Version | Owner | Use it for |
|----------|---------|-------|-----------|
| [GDD](gdd/gdd.md) | 1.0 | Diana | **What** to build: rules, screens, features, tuning values |
| [Technical Design](technical-design.md) | 1.0 | Theo | **How** to build it in Godot: architecture, code, tests, web export |
| [Art Bible](art-bible.md) | 1.0 | Aurora | **How it looks**: palette, sizes, faces, UI theme, effects (all drawn in code) |
| [Audio Bible](audio-bible.md) | 1.0 | Anton | **How it sounds**: effect list (jsfxr), music (BeepBox), mixing, event-to-sound map |
| [Ideation Handoff](../01-ideation/ideation-handoff.md) | — | Ivy | Background and reasoning |

**Where to look when docs disagree:** GDD for rules → Technical design for code → Art and Audio bibles for presentation. *(After this review, no conflicts are known.)*

Not produced, by decision: market analysis (learning prototype), separate level design (GDD §7), narrative design (no story).

---

## 3. Consistency Review (Dylan)

### 3.1 Checked ✅
- Rules (walls, tail exception, buffer, speed formula, near-miss) are **identical** in the GDD (§3, §6.2) and the technical design (§4.4, §5.3).
- The GDD §9 tuning table matches `GameConfig` field by field (plus `death_sequence`, see below).
- The GDD §6.1 events match the `Events` bus, which matches the audio bible §9 map and the art bible expression triggers.
- Draw order and the readability rule match between the art bible §4/§9 and the technical design §7.4.
- Platform scope (PC + Web + touch) is the same everywhere. CPUParticles2D is used everywhere (web-safe).

### 3.2 Conflicts found & fixed
| # | Issue | Resolution | Changed in |
|---|-------|-----------|-----------|
| 1 | The GDD listed an optional "turn tick" sound, which the audio bible cut | **Cut** (it gets tiring at 15 moves/sec) | GDD §6.5 |
| 2 | Banana (yellow) was too close to the golden snake | **Grapes** (purple) instead | GDD §3.3, Technical Design §4.1 *(earlier)* |
| 3 | A PERFECT run that is also a new best: which jingle plays? | **One jingle only**, priority perfect > new best > game over | Audio Bible §4.2 |
| 4 | The DYING state (about 1 s) was shorter than the death audio (about 1.15 s) | New `death_sequence = 1.2 s` | Technical Design §3, §5.1 |
| 5 | "Press any key" on Title / Game Over wasn't covered by the Input Map actions | Any non-echo key press counts as a tap in those states | Technical Design §6.2 |

> ℹ️ `01-ideation/mechanics-exploration.md` still says "banana". It's an archived ideation document, superseded by the GDD.

---

## 4. Locked Decisions (do not change without a design update)

| Area | Decision |
|------|----------|
| Grid | 20×15 landscape / 15×20 portrait, chosen at run start, fixed for the run |
| Walls | Kill · no wrap-around |
| Self-collision | Kills · moving into the cell the tail is leaving is safe unless growing |
| Turning | 180° reversal ignored · 2-turn buffer · reversals checked against the *last planned* direction |
| Fruit | 1 at a time on a random free cell · apple / cherry / grapes, cosmetic only · panic is visual only |
| Score | +10 per fruit · best score saved locally |
| Speed | `min(7 + 0.5 × floor(fruits / 3), 15)` moves/sec · same cap for touch *(retest)* |
| Near-miss | Turn applied, and the cell the snake would have entered was deadly · 1.5 s cooldown |
| Movement | The logic moves cell by cell · the visuals slide smoothly (interpolation) |
| Architecture | `SnakeModel` (pure logic) → `Game` (states, tick, buffer, emits) → `Events` bus → views and audio (listen only) |
| Art | 100% drawn in code · `Palette` script · sizes relative to the cell size · Fredoka font |
| Audio | jsfxr effects + BeepBox music/jingles · Music/SFX buses · low-pass on Title/Pause · music pitch up to ×1.08 with speed |

---

## 5. Build Plan (replaces the Planning phase)

| Milestone | Contents (GDD features) | Done when | Est. |
|-----------|------------------------|-----------|------|
| **M1 – Rules** | `GameConfig`, `SnakeModel`, headless tests T1-T11 | `ALL PASSED` | 1-2 days |
| **M2 – Playable squares** | F1-F7: tick loop, keyboard + buffer, collisions, fruit, speed, score + best, states and basic screens | A full run with colored squares, the best score survives a restart | 2-3 days |
| **M3 – Web check** ⚠️ | F8 Events bus · F14 web export (Technical Design §9.4) | Runs in a phone browser · best score kept after a reload | 1 day |
| **M4 – The snake comes alive** | F9 smooth slide · F10 expressions · art bible board/snake/fruit drawing | Grayscale test passes · no jitter at 15 moves/sec | 3-4 days |
| **M5 – Juice & sound** | F11 effects + `AudioManager` (placeholders are fine) · F15 settings | GDD §6.5 table done · shake toggle works | 2-3 days |
| **M6 – Touch & layouts** | F12 swipe · F13 two grids + rotation · F16 fruit panic · F17 PERFECT | Playable on a phone in both orientations | 2 days |
| **M7 – Polish** | Final effects and music, UI theme, icon, playtest tuning (touch speed cap!) | 5-minute listening and playing test with nothing annoying | rest of the timebox |

**MVP cut line:** M1-M5 is a complete, shareable PC + web game. M6-M7 make it great on phones.

---

## 6. Open Items for the Engine Team

| # | Item | When |
|---|------|------|
| 1 | Is the 15 moves/sec cap playable with swipes? (Tune `speed_cap`, maybe a separate touch cap) | M6 playtest |
| 2 | Does input feel laggy? If so, add the "early-tick grace" (Technical Design §12) | M2 playtest |
| 3 | Does the smooth slide hold up at high speed? Fallback is snap movement | M4 |
| 4 | Final title: does "Snake" stay? | Any time (low impact) |
| 5 | You download the Fredoka font; you make the audio in jsfxr/BeepBox | In parallel with M2-M5 |

---

## 7. Meet the Engine Team (Godot 4)

| Agent | Role | Ask them for |
|-------|------|-------------|
| **Érico** 🎯 | Engine Coordinator (Godot) | Readiness review, per-milestone tech specs, LLM prompts for GDScript |
| **Guilherme** | Godot Architect | Project structure, autoloads, scene tree (Technical Design §1-2) |
| **Gabi** | GDScript Lead | `SnakeModel`, `Game`, tests, coding style |
| **Gina** | Godot Node Specialist | Scenes, `_draw()` views, CanvasLayer UI, particles, Camera2D shake |

**Recommended start:** Érico → `*review` (readiness check), then **M1** with Gabi.

---

## ✅ Design Handoff Checklist
- [x] GDD complete with locked rules and acceptance criteria
- [x] Technical design for Godot 4.7 with architecture, algorithms, tests and a web plan
- [x] Art bible (code-drawn, palette, faces, UI, effects)
- [x] Audio bible (effects, music, mixing, event map)
- [x] Cross-document consistency review, 5 issues fixed
- [x] Build plan with milestones and an MVP cut line
- [x] Open items assigned

**Design Phase Complete! 🎉**

---
*Generated by Dylan, Design Coordinator, GD-KS Design Team*
