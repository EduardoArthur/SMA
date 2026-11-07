# Mechanics Exploration Document

## Game: Snake
## Date: 2026-11-06
## Explorer: Mira (Mechanics Explorer)

---

## Core Verb

### Primary Action
**The player primarily STEERS.**

Examples of this verb in action:
1. Turning toward a fruit that just spawned on the other side of the arena
2. Snaking back and forth in tight rows to fit a long body into a small space
3. Turning at the last moment to avoid your own tail (*The Near-Miss*)

### Supporting Verbs
| Verb | Frequency | Context |
|------|-----------|---------|
| Eat | Frequent | Moving onto a fruit cell (automatic) |
| Plan | Constant | Reading the route between the head, the fruit and your own body |
| Restart | Occasional | After death, one key or tap to try again |

---

## Core Loop

### Loop Diagram
```
┌──────────────────────────┐
│         ACTION           │
│   Steer toward fruit     │
└────────────┬─────────────┘
             ▼
┌──────────────────────────┐
│        FEEDBACK          │
│ Chomp, squash, particles │
└────────────┬─────────────┘
             ▼
┌──────────────────────────┐
│         REWARD           │
│  +points, +1 segment     │
└────────────┬─────────────┘
             ▼
┌──────────────────────────┐
│       MOTIVATION         │
│ New fruit spawns, beat   │
│ the high score           │
└────────────┬─────────────┘
             └──────────► (back to ACTION)
```

### Loop Details

**ACTION:** Steer the snake to the fruit
- Input required: 4 directions (arrows/WASD on PC, swipe on touch)
- Skill involved: route planning and timing turns to the grid ticks

**FEEDBACK:** The game reacts loudly to everything
- Visual feedback: squash-and-stretch on the snake, a fruit particle burst, a small screen bump, the snake's happy face
- Audio feedback: a crunchy "chomp" whose pitch rises slightly with combos (see Feel & Juice)
- Haptic feedback: short vibration on mobile web (if supported; nice-to-have)

**REWARD:**
- Immediate rewards: +10 points and a "+10" popup, one more body segment
- Delayed rewards: length milestones (10 / 25 / 50) with a celebratory reaction, and a new high score

**MOTIVATION:**
- Intrinsic: mastery (fitting a long snake into a full arena), comedy (seeing what the snake does next)
- Extrinsic: beating the saved high score

### Loop Timing
- Single loop duration: about 3-8 seconds (fruit to fruit)
- Loops per session: about 20-80
- Session duration target: 1-5 minutes per run, with several runs per sitting

---

## Secondary Loops

### Micro Loop (Moment-to-Moment)
```
[Read board] → [Queue turn] → [Snake moves 1 cell] → [Repeat]
```
Duration: one tick (about 0.07-0.14 s)
Example: Pressing ↑ then → quickly to make a tight U-turn on two consecutive ticks.

### Session Loop (Per Run)
```
[Start: short and slow] → [Build: longer and faster] → [Climax: arena crowded] → [Dramatic death] → [Score screen]
```
Duration: 1-5 minutes
Example: Easy early fruit, tense mid-game routing, then a crash and a cartoon faint at length 30.

### Meta Loop (Across Runs)
```
[Play] → [See score vs. high score] → [Instant restart] → [Play better]
```
Duration: one sitting
Example: "I got 240, my best is 310... one more."

---

## Systems Inventory

### Core Systems (Must Have)

#### System 1: Grid Movement
- **Purpose:** Moves the snake one cell per tick in the current direction
- **Player Interaction:** Direction input changes the heading
- **Complexity:** Low
- **Priority:** Critical
- **Dependencies:** Input, Tick Timer
- **Rules:**
  - Arena: **20 × 15 cells** (landscape; to be tuned in Design)
  - The snake starts with 3 segments at the center, heading right
  - A 180° reversal is ignored (you can't turn back into your own neck)
  - **Input buffer of 2 turns**, so fast double-turns aren't lost between ticks
  - The body follows the head. The tail cell is freed on the same tick the head moves (unless growing).

#### System 2: Input (Keyboard + Touch)
- **Purpose:** Turns player intent into direction changes
- **Player Interaction:** Arrows/WASD; **swipe** on touch screens
- **Complexity:** Low-Medium
- **Priority:** Critical
- **Dependencies:** None
- **Rules:**
  - Use Godot Input Map actions (`move_up`, `move_down`, `move_left`, `move_right`) so all devices share one code path
  - Swipe = the dominant axis of the drag once it passes about 30 px (scaled to DPI); one swipe produces one turn
  - Tap or any key restarts on the game-over screen

#### System 3: Eat & Grow
- **Purpose:** Gives the reward and increases the challenge
- **Player Interaction:** Moving the head onto the fruit cell
- **Complexity:** Low
- **Priority:** Critical
- **Dependencies:** Grid Movement, Fruit Spawner, Score
- **Rules:** +1 segment (the tail doesn't move on the next tick) and +10 points

#### System 4: Fruit Spawner
- **Purpose:** Places exactly one fruit on a random **free** cell
- **Complexity:** Low
- **Priority:** Critical
- **Dependencies:** Grid state
- **Rules:**
  - Pick from the list of empty cells, never retry randomly in a loop
  - If no free cell is left, the player **wins** (perfect game, a special celebration)
  - Fruit type (apple / cherry / banana) is **cosmetic only**

#### System 5: Collision & Death
- **Purpose:** Ends the run
- **Complexity:** Low
- **Priority:** Critical
- **Dependencies:** Grid Movement
- **Rules:**
  - **Walls kill.** Hitting the arena edge ends the run.
  - Hitting your own body ends the run. **Exception:** moving into the cell the tail is leaving on this tick is safe (unless growing).
  - On death: freeze → faint animation → score screen

### Supporting Systems (Important)

#### System 6: Speed Ramp
- **Purpose:** Raises tension over the run
- **Complexity:** Low
- **Priority:** High
- **Can be simplified:** Yes (a constant speed works as a fallback)
- **Rules (starting values, tune in playtests):**
  - Start: **7 moves/sec**
  - **+0.5 moves/sec every 3 fruits**
  - Cap: **15 moves/sec**

#### System 7: Score & High Score
- **Purpose:** The goal and the reason to replay
- **Complexity:** Low
- **Priority:** High
- **Can be simplified:** Yes
- **Rules:** +10 per fruit. The high score is saved to `user://` with `ConfigFile` (also works on web through browser storage).

#### System 8: Game Flow (States)
- **Purpose:** Title → Playing → Paused → Game Over → Playing
- **Complexity:** Low
- **Priority:** High
- **Rules:** Pause with Esc/P on PC and a pause button on touch. Pause automatically when the window or tab loses focus.

#### System 9: Juice & Reactions (the hook)
- **Purpose:** Delivers the "chaotic fun" feeling
- **Complexity:** Medium (many small pieces, each simple)
- **Priority:** High. **This is the hook, so build it right after the core loop works.**
- **Can be simplified:** Yes, add effects one by one

### Nice-to-Have Systems

#### System 10: Fruit Panic (cosmetic)
- **Purpose:** Personality. Fruit shakes and makes a scared face when the head is 3 cells away or closer.
- **Complexity:** Low
- **Priority:** Medium
- **MVP inclusion:** After the core juice is done
- **Rule:** The fruit **never moves**; the effect is visual only

#### System 11: Combo Pitch
- **Purpose:** Rewards eating quickly. The chomp pitch rises when fruits are eaten within about 3 s of each other.
- **Complexity:** Low
- **Priority:** Low
- **MVP inclusion:** No (no score effect, audio only)

#### System 12: Settings
- **Purpose:** Volume, screen shake on/off (accessibility)
- **Complexity:** Low
- **Priority:** Medium
- **MVP inclusion:** Screen shake toggle recommended

---

## Systems Interaction Map

```
            ┌───────────┐
            │   Input   │ (keys / swipe)
            └─────┬─────┘
                  ▼ queued turns
┌───────────┐  ┌──────────────┐   ┌──────────────┐
│Speed Ramp │─►│ Tick Timer → │──►│  Collision   │──► Game Over ─► Score screen
└─────▲─────┘  │Grid Movement │   │  & Death     │
      │        └──────┬───────┘   └──────────────┘
      │               ▼ head on fruit
      │        ┌──────────────┐   ┌──────────────┐
      └────────│  Eat & Grow  │──►│ Score / High │
               └──────┬───────┘   └──────────────┘
                      ▼
               ┌──────────────┐
               │Fruit Spawner │
               └──────────────┘

   Juice & Reactions listens to ALL events (signals) ─ never changes rules
```

### Key Interactions
1. **Eat & Grow → Speed Ramp:** eating fruit increases the tick rate, so success makes the game harder (a self-balancing loop)
2. **Eat & Grow → Grid:** a longer body means fewer free cells, so routing gets harder
3. **Juice ← every system:** the game emits signals (`fruit_eaten`, `near_miss`, `died`, `milestone_reached`) and the juice layer only *reacts* to them. This keeps the rules simple and lets effects be added or removed safely.

---

## Feel & Juice

### Core Action Feel

**Target Feel:** *Snappy controls, bouncy cartoon reactions.* The input must feel instant. Everything else should feel over the top.

**Reference Games:**
1. Google Snake: precise grid controls and clear readability
2. Celeste / Downwell: input forgiveness through buffering, and screen shake used with intent
3. Cartoon mascot mobile games: exaggerated faces and squash-and-stretch

### Feedback Requirements

| Action | Visual | Audio | Timing |
|--------|--------|-------|--------|
| Turn | Head rotates with a small overshoot | Soft "tick" (optional) | Same tick as input |
| Eat fruit | Head squash, fruit particle burst, "+10" popup, happy face | Crunchy chomp | Instant, about 150 ms of animation |
| Near-miss (head next to own body or wall after turning) | Gasp face, sweat drop, tiny camera nudge | "Gasp" / whoosh | Instant, about 300 ms |
| Length milestone (10/25/50) | Rainbow flash on the body, confetti | Fanfare sting | About 600 ms |
| Death | Freeze for about 0.3 s, X-eyes faint, strong screen shake, body segments bounce | Comic "bonk" and slide whistle | About 1 s before the score screen |
| New high score | Crown on the snake, sparkles on the score | Victory jingle | On the score screen |

### Polish Notes
- **Screen shake:** small on eating, large on death. **Must have an off toggle.**
- **Particles:** fruit burst on eating, confetti at milestones, stars at death
- **Animation:** the head faces (neutral, happy, gasp, faint) come first, then body wobble, then fruit idle bounce
- **Sound:** chomp, gasp, bonk, milestone, high score, plus one looping upbeat track
- **Readability rule:** effects must **never hide the grid or the snake's path**. Visual effects go behind the game layer or are short.

---

## Progression Systems

### Player Progression
- **Type:** Skill-based (no unlocks)
- **Curve:** Linear speed ramp up to a cap, while length adds natural difficulty
- **Pacing:** Fast start (first fruit within about 2 s), slow build

### Content Progression
- **How content unlocks:** Not in the MVP. *Later idea:* unlock cosmetic snake hats or skins at score thresholds.
- **Gating mechanisms:** None
- **Content milestones:** Length 10 / 25 / 50 celebrations within a run

### Difficulty Progression
- **Scaling method:** Speed ramp plus a longer body
- **Difficulty options:** None in the MVP (open question)
- **Rubber-banding:** No

---

## Risk Analysis

### Technical Risks
| Risk | Likelihood | Impact | Mitigation |
|------|------------|--------|------------|
| Lost inputs when turning quickly between ticks | H | H | 2-turn input buffer, process the queue once per tick |
| Swipe detection feels unreliable on phones | M | M | Dominant-axis plus threshold, test on a real phone early |
| Web export issues (audio autoplay, saving) | M | L | Start audio after the first input, test the web export in week 1 |
| Godot learning curve (beginner) | M | M | Logic-only grid model in plain GDScript, with visuals as a separate layer |

### Design Risks
| Risk | Likelihood | Impact | Mitigation |
|------|------------|--------|------------|
| Juice hurts readability (chaos becomes confusion) | M | H | Readability rule, short effects, shake toggle |
| Feels like "just Snake" without the juice | M | M | Build the juice right after the core loop, not "at the end" |

### Balance Risks
| Risk | Likelihood | Impact | Mitigation |
|------|------------|--------|------------|
| Speed cap too high on touch (swipes are slower than keys) | M | M | Separate lower cap for touch, or tune the cap in playtests |
| Start too slow, so it's boring | L | M | Start at 7 moves/sec and tune |

---

## Prototype Priorities

### Priority 1: Grid Movement & Input
- **Goal:** Prove the controls feel snappy
- **Scope:** Squares on a grid, keyboard input with buffer, walls kill, self-collision
- **Success Criteria:** No lost turns, no unfair deaths, a quick U-turn always works
- **Time Estimate:** 1-2 days (includes learning Godot basics)

### Priority 2: Loop Validation
- **Goal:** Prove "one more run"
- **Scope:** Fruit spawner, grow, score, speed ramp, game over, instant restart, high score save
- **Success Criteria:** You *want* to restart after dying
- **Time Estimate:** 1-2 days

### Priority 3: Hook (Juice & Reactions)
- **Goal:** Prove the "chaotic fun" feeling
- **Scope:** Head faces, chomp/death effects, screen shake, particles, sounds
- **Success Criteria:** A playtester laughs or reacts out loud at least once per run
- **Time Estimate:** 3-5 days

### Priority 4: Platforms
- **Goal:** Shareable build
- **Scope:** Swipe input, web export, pause/focus handling
- **Success Criteria:** Playable in a phone browser with swipes
- **Time Estimate:** 1-2 days

---

## Reference Games

### Primary References
| Game | What to Learn | What to Avoid |
|------|---------------|---------------|
| Classic Nokia Snake | Pure, readable rules | Bland feedback |
| Google Snake | Smooth grid motion, clear visuals, good touch controls | Copying its look |
| Downwell / Celeste | Input forgiveness and intentional screen shake | Too much shake |

### Secondary References
| Game | Specific Element |
|------|------------------|
| Cartoon mascot mobile games | Expressive faces and squash-and-stretch |
| "Juice it or lose it" (GDC talk) | Layering small effects onto simple mechanics |

---

## Open Questions

1. Exact grid size and cell size for both PC (landscape) and phone. Should phones force landscape? *(Design Team)*
2. Should touch have a lower speed cap than keyboard? *(Design Team / playtest)*
3. Does the snake move smoothly between cells (interpolated) or jump cell to cell? *(Design and Engine Team: smooth looks cuter but is more work)*
4. Is a difficulty selector wanted later? *(Design Team)*

---

## Recommendations

### For Design Team
1. Keep the rule set **exactly** as defined here. All "chaos" goes in the presentation layer.
2. Define the snake's expression set and the effects list (art and audio bible can stay short).

### For Engine Team (Godot)
1. Separate **grid logic** (pure GDScript data: an array of `Vector2i`) from **visuals** (nodes that react to signals). This is easier to debug and makes juice easy to add.
2. Use a `Timer` (or an accumulator in `_process`) for ticks, an Input Map for all controls, and `ConfigFile` at `user://` for the high score.
3. Test the **web export early** (week 1), not at the end.

---

*Exploration by Mira, GD-KS Mechanics Explorer*
*Generated: 2026-11-06*
