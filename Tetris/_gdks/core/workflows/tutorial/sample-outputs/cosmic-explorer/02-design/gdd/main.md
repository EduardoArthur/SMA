# Cosmic Explorer — Game Design Document

*Sample document produced by the Design team (Diana + Dylan).
Part of the GD-KS tutorial. Not a real project.*

## Pillars

1. **Weightlessness as Mechanic** — Gravity isn't a bug, it's the
   core feel. Every room reinforces this.
2. **Environmental Storytelling** — No dialogue. Rooms tell stories
   through objects, lighting, and layout.
3. **Quiet Moments Matter** — Pacing rewards stillness. Some rooms
   have no puzzle — just presence.

## Core Mechanics

### Drift (core verb)
- Momentum-based movement in low gravity
- Base speed: 1.5 m/s, max speed: 3.0 m/s
- Drag coefficient: 0.15 (slow deceleration, feels floaty)
- Input: analog stick or WASD, applies continuous force

### Anchor
- Input: hold right trigger / shift
- Raycasts to nearest anchor-able surface within 2m
- Smoothly interpolates position (300ms)
- Player regains precise control while anchored
- Release to resume drift

### Resonance Pulse
- Input: tap A / spacebar
- One use per room (resets on room transition)
- Visual: 8m radius expanding wave
- Effect: highlights hidden paths for 3 seconds
- Audio: soft harmonic chord, tuned per environment

## Core Loop

```
Enter room
  ↓
Drift cautiously
  ↓
Read environment (silent beats — 5-30s)
  ↓
Solve navigation puzzle (optional, not every room)
  ↓
Discover narrative fragment
  ↓
Transition to next room
```

## Target Audience

(See `01-ideation/concept-brief.md`)

## Out of Scope

- Combat, in any form
- Dialogue, spoken or written
- Character progression (no XP, no unlocks)
- Branching paths (linear room sequence)
