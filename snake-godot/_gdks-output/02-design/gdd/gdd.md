# Snake - Game Design Document

> **Author:** Diana (Game Design Director), GD-KS Design Team
> **Date:** 2026-11-06
> **Version:** 1.0 (Draft)
> **Engine:** Godot 4.x · **Platforms:** PC (Windows) + Web, keyboard and touch
> **Sources:** `01-ideation/ideation-handoff.md`, `concept-brief.md`, `mechanics-exploration.md`

---

## 1. Overview

**Logline:** *Same old rules, a very dramatic little snake.*

A single-player arcade Snake game. The rules are classic and are **locked** (section 3). The game stands out through a cute cartoon snake that reacts to everything, plus heavy game feel (section 6).

### 1.1 Design Pillars
Every feature must serve at least one pillar. If it doesn't, cut it.

| # | Pillar | Means | Does NOT mean |
|---|--------|-------|---------------|
| P1 | **Classic & fair** | Familiar rules, precise controls, deaths that are always the player's fault | New rules, random hazards |
| P2 | **Dramatic little snake** | The snake and fruit have faces and react to events | Story, dialogue |
| P3 | **Juicy but readable** | Every action has punchy visual and audio feedback | Effects that hide the grid or the snake's path |
| P4 | **One more run** | A fast start, instant restart, a high score to beat | Long menus, unlock grinds |

---

## 2. Game Flow & Screens

```
 [Boot] → [Title] ──play──► [Playing] ──Esc/P/focus lost──► [Paused] ──resume──┐
                              ▲    │                                           │
                              │    └──────────── died / board full ─────► [Game Over]
                              │                                               │
                              └────────────── restart (any key / tap) ────────┘
                                               (title: Esc / button)
```

| Screen | Contents | Input |
|--------|----------|-------|
| **Title** | Logo "Snake", the snake idling and blinking, "Press any key / Tap to play", best score, ⚙ settings button | Any key / tap → Playing |
| **HUD (Playing)** | Score (left/top), Best (right/top), ⏸ pause button (always on touch, optional on PC) | — |
| **Paused** | Dimmed board, "Paused", Resume / Settings / Quit to Title | Esc/P/tap Resume |
| **Game Over** | Fainted snake, final score, best score, "NEW BEST!" crown if beaten, "Press any key / Tap to retry" | Any key / tap → Playing (after a **0.5 s input lock** so a panicked keypress doesn't skip it) |
| **Win (board full)** | Game Over screen variant: "PERFECT!", confetti, crowned snake | Same as Game Over |
| **Settings** (overlay) | Music volume, SFX volume, Screen shake On/Off | Back |

**Acceptance criteria**
- From launch to playing in **≤ 2 inputs**. From death to playing again in **1 input** (after the 0.5 s lock).
- The game pauses itself when the window or tab loses focus.
- Settings and best score persist between sessions (desktop and web).

---

## 3. Core Rules (LOCKED)

### 3.1 Arena & Grid: two layouts
| Layout | Grid (cols × rows) | Used when |
|--------|--------------------|-----------|
| **Landscape** | **20 × 15** | Viewport width ≥ height at **run start** |
| **Portrait** | **15 × 20** | Viewport height > width at **run start** |

- Both grids have **300 cells**, so scoring and difficulty are identical. The best score is shared.
- The layout is **chosen when a run starts** and **never changes mid-run**.
- If the orientation or window size changes mid-run, the game **pauses automatically** and the board is rescaled to fit (letterboxed). The grid stays the same until the next run.
- The board is scaled to fit the screen, leaving room for the HUD. Cells are always square.

### 3.2 Snake
- Starts with **3 segments** at the grid center, heading **toward the long side**: right in landscape, up in portrait.
- Moves **exactly 1 cell per tick**.
- Turning: 4 directions. A **180° reversal is ignored**, checked against the direction the snake is *actually* moving, not the last buffered one.
- **Input buffer of 2 turns.** One buffered turn is used per tick. Extra inputs are dropped. A turn equal to the current direction is ignored (it doesn't take a buffer slot).
- The run begins on the **first direction input** (or after 1 s, whichever comes first), so the player isn't caught off guard.

### 3.3 Fruit
- Exactly **1 fruit** on the board at a time, placed on a uniformly random **free cell** (never on the snake).
- Type (🍎 apple / 🍒 cherry / 🍇 grapes; banana dropped because it was too close in color to the snake, see art bible §2.3) is picked at random and is **cosmetic only**.
- Eating means the head enters the fruit cell: **+10 points**, and the snake grows by **1 segment** (the tail stays still on the next tick).
- **Win:** if there's no free cell for the next fruit, the run ends as **PERFECT!**

### 3.4 Collision & Death
- The head leaving the grid (a wall) means **death**. There is no wrap-around.
- The head entering a cell occupied by the body means **death**. **Exception:** the cell the tail is leaving this tick is safe (unless the snake grows this tick).
- Collisions are checked **once per tick**, after the head moves.

### 3.5 Speed Ramp
`moves_per_sec = min(7 + 0.5 × floor(fruits_eaten / 3), 15)`

| Fruits eaten | 0 | 3 | 6 | 9 | 12 | 15 | … | 48+ |
|---|---|---|---|---|---|---|---|---|
| Moves/sec | 7.0 | 7.5 | 8.0 | 8.5 | 9.0 | 9.5 | … | 15.0 (cap) |

The same cap applies to keyboard and touch. **To be retested on a real phone** (see section 9).

### 3.6 Scoring
- +10 per fruit. No multipliers in the MVP.
- **Best score** is saved locally (`user://`; browser storage on web).

---

## 4. Controls

| Action | Keyboard | Touch |
|--------|----------|-------|
| Turn | Arrows / WASD | **Swipe** anywhere on screen |
| Pause / Resume | Esc / P | ⏸ button |
| Start / Restart | Any key | Tap |
| Back (menus) | Esc | Back button |

**Swipe rules**
- A swipe is detected once the finger moves past **≈ 30 px × screen scale** from where the touch started. The direction is the **dominant axis** of that movement.
- **One turn per swipe.** To turn again, lift the finger or move another 30 px in a different direction from the last turn point (this allows quick L-shaped double turns).
- Taps during play do nothing (no accidental pauses).

**Acceptance criteria:** pressing ↑ then → within one tick produces two turns on two consecutive ticks. No input is lost and no unfair reversal death happens. This is the same for a fast double swipe.

---

## 5. Movement Presentation: smooth slide

- **The logic stays discrete:** the snake's position is an array of grid cells, updated once per tick (section 3).
- **The visuals interpolate:** each segment's on-screen position is tweened from its previous cell to its current cell over the tick duration. The head leads and the body follows the same path. Corners are drawn rounded.
- On the tick where the snake grows, the new tail segment appears in place (no slide).
- **Death freeze:** the visuals stop wherever the interpolation is, and the head is drawn bumping into the obstacle.
- Rule: **the visuals never decide anything.** Collisions, eating and turning are based only on the grid state.

**Acceptance criteria:** at 15 moves/sec the movement still reads as continuous with no jitter. Turning feels instant: the head's facing updates at the start of the tick.

---

## 6. Reactions & Juice (the hook)

The logic emits **events**. The presentation layer listens and reacts. The juice never changes the rules (P1, P3).

### 6.1 Game Events
| Event | Fired when |
|-------|-----------|
| `run_started` | First tick of a run |
| `turned(dir)` | A buffered turn is applied |
| `fruit_eaten(cell, type)` | The head enters the fruit cell |
| `near_miss(cell)` | See 6.2 |
| `milestone_reached(length)` | Snake length reaches **10, 25, 50, 100** |
| `speed_changed(mps)` | The speed ramp steps up |
| `died(cause)` | `wall` or `self` |
| `board_cleared` | No free cell left (PERFECT) |
| `new_best(score)` | When the run ends with score > best |

### 6.2 Near-miss definition: "last-second turn"
On a tick where a **turn was applied**, check the cell the head **would have entered** if it had kept its **previous** direction. If that cell is a **wall** or a **body segment** (using the same tail exception as 3.4), fire `near_miss`.
- **Cooldown: 1.5 s** between near-miss reactions, so zig-zagging along a wall doesn't spam it.
- The near-miss is never fired on the same tick as `died` or `fruit_eaten` (eating wins).

### 6.3 Snake Expressions
| Expression | Trigger | Duration |
|------------|---------|----------|
| 😐 **Neutral** (blinks every 2-4 s) | Default | — |
| 😋 **Happy chomp** | `fruit_eaten` | 0.4 s |
| 😱 **Gasp** + sweat drop | `near_miss` | 0.6 s |
| 🤩 **Proud** (sparkle eyes) | `milestone_reached` | 1.0 s |
| 😵 **Faint** (X-eyes, tongue out) | `died` | Until Game Over |
| 😎 **Cool** + crown | `board_cleared`, and `new_best` on the Game Over screen | Until restart |
| 😴 **Sleepy** | Title screen idle, and the Paused screen | — |

Priority if two overlap: Faint > Cool > Gasp > Proud > Happy > Neutral.

### 6.4 Fruit Reactions (cosmetic)
- **Idle:** gentle bounce.
- **Panic:** when the head is **≤ 3 cells away** (Manhattan distance), the fruit makes a scared face and shakes. **It never changes cell.**
- **Eaten:** a burst of juice particles in the fruit's color.

### 6.5 Feedback Table (MVP)
| Event | Visual | Audio | Shake |
|-------|--------|-------|-------|
| `turned` | Head turns with a small overshoot | — (cut: tiring at high speed, see audio bible §3) | — |
| `fruit_eaten` | Head squash 0.15 s, particle burst, "+10" popup floats up | **Chomp** (crunchy) | Small (2 px, 0.1 s) |
| `near_miss` | Gasp face, sweat drop, a small push toward the danger | **Gasp** | — |
| `milestone_reached` | Rainbow wave runs along the body, confetti | **Milestone** sting | — |
| `speed_changed` | Short speed-line streaks behind the head | — | — |
| `died` | **0.3 s freeze**, faint face, body segments bounce one after another from head to tail | **Bonk** + slide whistle | Large (8 px, 0.4 s) |
| `board_cleared` | Confetti storm, crown | **Fanfare** | Medium |
| `new_best` | Crown on the Game Over snake, sparkle on the score | **Jingle** | — |

**Readability rule (P3):** effects are drawn **under** the snake and fruit, or last **≤ 0.6 s**. Nothing covers the head's next 3 cells. With **screen shake Off**, all shake is skipped and everything else stays.

---

## 7. Arena (Level)

There is only one arena: a garden lawn with a checkerboard pattern in two soft shades of green for readable cells, and a thick rounded hedge border marking the deadly wall. No obstacles in the MVP.
*(This replaces a separate level design doc for this project.)*

---

## 8. Feature List, Priorities & Dependencies

| ID | Feature | Pillar | Priority | Depends on |
|----|---------|--------|----------|------------|
| F1 | Grid model and tick loop | P1 | **Must** | — |
| F2 | Keyboard input + 2-turn buffer | P1 | **Must** | F1 |
| F3 | Fruit spawn, eat and grow | P1 | **Must** | F1 |
| F4 | Collisions (walls, self, tail exception) | P1 | **Must** | F1 |
| F5 | Score + best score save | P4 | **Must** | F3 |
| F6 | Game flow: Title / Playing / Paused / Game Over | P4 | **Must** | F1-F5 |
| F7 | Speed ramp | P1 | **Must** | F3 |
| F8 | Event signals (section 6.1) | P2, P3 | **Must** | F1-F7 |
| F9 | Smooth slide visuals | P2 | **Should** | F1, F8 |
| F10 | Snake expressions | P2 | **Should** | F8 |
| F11 | Feedback table effects + sound | P3 | **Should** | F8 |
| F12 | Touch swipe input | P1 | **Should** | F2 |
| F13 | Two grid layouts + resize/orientation handling | P1 | **Should** | F1, F6 |
| F14 | Web export (audio unlock, save, focus pause) | P4 | **Should** | F5, F6 |
| F15 | Settings (volumes, shake toggle) | P3 | **Should** | F11 |
| F16 | Fruit panic and idle animations | P2 | Could | F10 |
| F17 | PERFECT win screen | P4 | Could | F3, F6 |
| F18 | Combo pitch on chomp | P3 | Won't (MVP) | F11 |

**Recommended build order:** F1 → F2 → F4 → F3 → F7 → F5 → F6 → F8 → *(first playable build)* → F14 *(web check in week 1)* → F9 → F10 → F11 → F12 → F13 → F15 → F16 → F17.

---

## 9. Tuning Values (single source of truth)

Every value below must live in **one config place** in code (for example a `GameConfig` resource or constants script) so it can be tuned without searching.

| Name | Value | Notes |
|------|-------|-------|
| `GRID_LANDSCAPE` | 20 × 15 | |
| `GRID_PORTRAIT` | 15 × 20 | |
| `START_LENGTH` | 3 | |
| `START_DELAY` | 1.0 s | Or the first input |
| `SPEED_START` | 7.0 mps | |
| `SPEED_STEP` | +0.5 mps | |
| `FRUITS_PER_STEP` | 3 | |
| `SPEED_CAP` | 15.0 mps | Same for touch, **retest on a phone** |
| `INPUT_BUFFER` | 2 | |
| `POINTS_PER_FRUIT` | 10 | |
| `MILESTONES` | 10, 25, 50, 100 | Snake length |
| `NEAR_MISS_COOLDOWN` | 1.5 s | |
| `FRUIT_PANIC_RANGE` | 3 cells | Manhattan distance |
| `SWIPE_THRESHOLD` | 30 px × scale | |
| `GAMEOVER_INPUT_LOCK` | 0.5 s | |
| `DEATH_FREEZE` | 0.3 s | |

---

## 10. Out of Scope (MVP)
Power-ups · obstacles / multiple arenas · enemies / tail-trap · rhythm mode · difficulty select · skins / unlocks · leaderboards · gamepad *(easy to add later through the Input Map)*.

---

## 11. Open Questions & Risks

| # | Item | Owner | Status |
|---|------|-------|--------|
| 1 | Is a 15 mps cap playable with swipes? | Playtest on a phone | Open, tune after F12 |
| 2 | Final title (stays "Snake"?) | You | Open, low impact |
| 3 | Smooth slide at high speed might look blurry or laggy | Engine / playtest | Watch, fallback is snap movement |
| 4 | Web audio only starts after the first user input | Engine | Known, start music on the first input |

---

## 12. Next Steps (Design Team)
- **Theo (Technical Game Designer):** technical design for the grid model, tick, input buffer, events and config resource in Godot.
- **Aurora (Art Director):** short art bible with the snake and fruit style, expression sheet, palette and effects style.
- **Anton (Audio Director):** short audio bible with the list of 7 sound effects and 1 music loop.
- Level design is covered by section 7 and narrative design is skipped (handoff decision).

---
*GDD by Diana, GD-KS Design Team · a living document, update it as playtests reveal things.*
