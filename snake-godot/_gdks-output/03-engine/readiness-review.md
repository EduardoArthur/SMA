# Snake - Engine Readiness Review

> **Reviewer:** Érico (Engine Coordinator, Godot)
> **Date:** 2026-11-06
> **Input:** `02-design/design-handoff.md` and the documents it links
> **Verdict:** ✅ **Ready to start M1.** A few project setup items are done inside M1/M2 (listed below).

---

## 1. Documentation readiness

| Check | Status | Notes |
|-------|--------|-------|
| Every milestone maps to GDD features | ✅ | Design handoff §5 |
| Every system names its script path | ✅ | Technical Design §2 |
| Signal contract written before code | ✅ | Technical Design §8 (`Events`) |
| Tuning values in one place | ✅ | `GameConfig` (Technical Design §3) |
| Testable rules with defined cases | ✅ | T1-T11 (Technical Design §10) |
| Art/audio production doesn't block code | ✅ | Code-drawn art · silent audio placeholders allowed |
| Planning phase skipped | ✅ OK | Engine module lists `planning` as a dependency. The milestone plan replaces it for this track. |

## 2. Project readiness (`project.godot`)

| Item | Current | Needed | When |
|------|---------|--------|------|
| Godot version | 4.7 | 4.7 | ✅ |
| Renderer | GL Compatibility (desktop + mobile) | GL Compatibility (web-safe) | ✅ |
| Stretch | `canvas_items` + `expand` | Same | ✅ |
| Base window size | Default (1152×648) | **1280×720** | M2 |
| Main scene | Not set | `res://game/game.tscn` | M2 |
| Autoloads | None | `Events`, `SaveData`, `AudioManager` | M2 (Events can wait until M3) |
| Input Map | Empty | `move_*`, `pause`, `confirm` | M2 |
| MSAA 2D | Off | 2× | M4 |
| Emulate touch from mouse | Off | On | M6 |
| `.gitignore` | `.godot/`, `/android/` | + `build/` (web export output) | M3 |
| 3D physics = Jolt | Set by the template | Unused (2D game), harmless | — |

## 3. Tooling

| Item | Status |
|------|--------|
| Godot command line available to run headless tests | ⏳ Checking (see the session) |
| Web export templates installed | Unknown, needed at M3 |

## 4. M1 scope (handed to Gabi)

Files:
- `config/game_config.gd` + `config/game_config.tres`
- `game/step_result.gd`
- `game/snake_model.gd`
- `tests/test_snake_model.gd`

Done when: `godot --headless -s res://tests/test_snake_model.gd` prints `ALL PASSED` for T1-T11.

---
*Érico, GD-KS Engine Team (Godot)*
