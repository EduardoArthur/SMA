# Snake - Ideation Handoff

> **From:** Ideation Team (Sparky, Mira, Ivy)
> **To:** Design Team (Diana and team)
> **Date:** 2026-11-06
> **Status:** Ready for Design Phase
> **Track:** Standard (Ideation → Design → Engine) · Solo · Godot beginner · 1-4 week prototype

---

## 📋 Executive Summary

### The Game in One Sentence
> **Snake: Same old rules, a very dramatic little snake.**

### Core Concept
A single-player arcade Snake game on a grid arena. Classic rules: steer, eat fruit, grow longer, don't hit walls or yourself. The rules are kept exactly classic. All the personality comes from a cute cartoon presentation and heavy game feel ("juice").

### Unique Hook
**The snake is a character, not a line.** It makes happy faces when it eats, gasps at near-misses and faints dramatically when it dies. The fruit makes scared faces as the snake gets close. Screen shake, squash-and-stretch, particles and comic sounds make every run feel like chaotic fun, without changing a single rule.

### Target Audience
Casual players of all ages who want short 1-5 minute sessions and the pull of beating their high score. Achiever-type motivation.

---

## 🎯 Concept Foundation

### Genre & Style
- **Primary Genre:** Arcade / Casual (score attack)
- **Tone:** Playful, silly, energetic
- **Art Direction:** Cute / cartoon. Round shapes, bright saturated colors, a snake with big expressive eyes, fruit with faces.

### Core Fantasy
> "In Snake, you get to be **a greedy little snake who can't stop eating, even when it really should.**"

### Setting
One colorful garden-style arena seen from above. No story is needed.

### Key Emotions
1. **Chaotic fun:** laughing at near-misses and over-the-top reactions
2. **Rising tension:** space shrinks as the snake grows and speeds up
3. **"One more run":** instant restart and a high score to beat

---

## 📊 Market Position

> ⚠️ **Market analysis was skipped on purpose.** This is a learning prototype and portfolio piece, not a commercial release.

### Competitive Landscape
Snake is a well-known, public-domain style of game (Nokia Snake, Google Snake, many clones). Competing on rules isn't the goal.

### Our Position
Familiar rules with personality: the "juiciest" small Snake you can share in a browser.

### Target Price Point
Free (web / portfolio)

---

## ⚙️ Mechanics Foundation

### Core Loop
```
[Steer] → [Eat: chomp + juice] → [Grow + score + speed up] → [New fruit / beat the high score] → repeat
```

### Locked Decisions
| Topic | Decision |
|-------|----------|
| Walls | **Kill the snake** (no wrap-around) |
| Self-collision | Kills the snake. **Moving into the tail cell as it leaves is safe.** |
| Turning | 4 directions, **180° reversal ignored**, **2-turn input buffer** |
| Fruit | One at a time, on a random free cell. Type is cosmetic. **Panic is visual only, fruit never moves.** |
| Scoring | +10 per fruit. High score saved locally. |
| Difficulty | **Speed ramp:** 7 moves/sec, +0.5 every 3 fruits, cap 15 |
| Win | Filling the whole board = perfect game |
| Platforms | **PC (keyboard) + Web export + touch swipe** |
| Grid (proposed) | 20 × 15 cells, snake starts with 3 segments at the center heading right |

### Key Mechanics
1. **Grid movement:** one cell per tick, buffered turns
2. **Eat & grow:** +1 segment, +10 points, a new fruit spawns on a free cell
3. **Collision & death:** walls and own body end the run, followed by a dramatic death sequence
4. **Speed ramp:** success makes the game faster
5. **Juice & reactions (the hook):** a separate layer that only *reacts* to game events

### Systems Overview
Core: Grid Movement, Input (keyboard + swipe), Eat & Grow, Fruit Spawner, Collision & Death.
Supporting: Speed Ramp, Score & High Score, Game Flow (Title / Playing / Paused / Game Over), Juice & Reactions.
Nice-to-have: Fruit panic animation, combo pitch on the chomp sound, Settings (volume, screen shake toggle).

**Architecture principle:** the game logic sends events (`fruit_eaten`, `near_miss`, `milestone_reached`, `died`), and visuals and audio only listen to them. The juice must never change the rules.

---

## 📁 Document References

| Document | Status | Location |
|----------|--------|----------|
| Concept Brief | ✅ Complete | `01-ideation/concept-brief.md` |
| Market Analysis | ⏭️ Skipped (learning prototype) | — |
| Mechanics Exploration | ✅ Complete | `01-ideation/mechanics-exploration.md` |

---

## ❓ Open Questions

### High Priority (Design Team)
1. **Grid and screen layout:** final grid size and cell size, and how it fits on phones. Should phones force landscape, or should portrait use a different grid?
2. **Movement presentation:** should the snake move smoothly between cells (cuter, more work) or jump cell to cell (simpler)?
3. **Snake expression set:** confirm the list of faces (neutral, happy, gasp, faint, plus milestone and high-score reactions) and when each triggers. Define "near-miss" exactly.

### Medium Priority (Design Team)
1. Should touch have a lower speed cap than keyboard?
2. Effects list with priority and duration, including the screen shake toggle and the readability rule (effects never hide the grid or the snake's path).
3. The UI screens needed: title, HUD (score + best), pause, game over / new high score.

### For Later Teams (Engine)
1. Web export check in week 1: audio autoplay (start after the first input), saving to `user://` in the browser, touch input.
2. Tick implementation (`Timer` vs. accumulator) and how the input buffer works with it.

### Unresolved (low impact)
- Final game title: does "Snake" stay as the working title?
- Difficulty selector: post-MVP?

---

## 🎨 Design Team Brief

### Immediate Priorities
1. **GDD (lightweight):** lock the rules above and add the UI flow and screens
2. **Technical design:** grid data model, signals and events, tuning values in one place
3. **Art bible (short):** snake and fruit style, expressions, color palette, effects list
4. **Audio bible (short):** about 6 sound effects and 1 music loop

### Suggested scope for this project (Standard track, solo, 1-4 weeks)
| Deliverable | Recommendation |
|-------------|----------------|
| GDD | ✅ Yes, short (a few pages) |
| Technical design | ✅ Yes, the most useful document for a Godot beginner |
| Art bible | ✅ Short (1-2 pages) |
| Audio bible | ✅ Short (a list) |
| Level design | ⚪ Minimal: only one arena, can be a GDD section |
| Narrative design | ⚪ Skip: no story; snake "personality" goes in the art bible |

### Recommended Starting Point
Begin with **Diana** (`@game-design-director`) using `*create-gdd`.

### Key Decisions Needed
- Grid size and phone orientation
- Smooth vs. cell-to-cell movement
- The final expression and effects lists

---

## 👋 Meet Your Design Team

| Agent | Role | Specialty |
|-------|------|-----------|
| **Diana** 🎨 | Game Design Director | GDD, systems design |
| **Leo** 🗺️ | Level Designer | Level documentation (minimal here) |
| **Nina** 📖 | Narrative Designer | Story, characters (skip here) |
| **Theo** 📐 | Technical Game Designer | Specs, data tables |
| **Aurora** 🎨 | Art Director | Visual direction |
| **Anton** 🎵 | Audio Director | Sound design |
| **Dylan** 📋 | Design Coordinator | Team coordination |

---

## ✅ Handoff Checklist

- [x] Concept clearly defined
- [x] Hook identified (presentation-driven, rules stay classic)
- [ ] Market research complete: *skipped on purpose (learning prototype)*
- [x] Target audience profiled
- [x] Core mechanics designed, with locked decisions
- [x] Scope assessed (MVP, 1-4 weeks, solo)
- [x] Open questions documented and prioritized
- [x] All documents linked

### Consistency check (Ivy)
- ✅ Concept brief and mechanics doc agree on walls, fruit, speed ramp and platforms
- ✅ No contradictions found. Terminology is consistent (fruit, segment, tick, juice).
- ✅ Every "post-MVP" idea (power-ups, arenas, tail-trap, rhythm) is in the concept brief's notes and none has slipped into scope

---

**Ideation Phase Complete! 🎉**

*Proceed to the Design Team. Good luck!*

---

*Generated by GD-KS Ideation Team (Ivy, Ideation Coordinator)*
