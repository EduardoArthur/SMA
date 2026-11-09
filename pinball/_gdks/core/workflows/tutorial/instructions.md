# 🎓 GD-KS Guided Tutorial

> A walk-through of the complete GD-KS pipeline using a small sample
> project. You'll understand the 4 phases, the key agents, and the
> contract-driven handoff flow in ~15 minutes.
>
> Flags:
>   `*tutorial`             — full tutorial (~15 min)
>   `*tutorial --fast`      — no pauses, no prompts, ~5 min read-through
>   `*tutorial --skip-to=N` — jump to step N (1-9)
>   `*tutorial --resume`    — continue from your last checkpoint
>   `*exit-tutorial`        — quit and keep your progress

---

## How this works

The tutorial runs in **sandbox mode**: it creates a throwaway state at
`_gdks/_state/tutorial-state.yaml` instead of touching your real
`project-state.yaml`. When you finish (or type `*exit-tutorial`), you
can inspect what was generated in `_gdks-output-tutorial/` but your
real project is untouched.

The tutorial uses a pre-baked sample project called **Cosmic
Explorer** — a tiny 2D puzzle-platformer about a lost astronaut
navigating a derelict space station. Every document is already
written; the tutorial unveils them progressively to show what each
phase produces.

---

## Step 1 — Welcome

Welcome to GD-KS! You're about to see how this framework organizes
game development into 4 phases, each run by a specialized team:

| Phase | Team | What they do |
|-------|------|--------------|
| 1 | Ideation | Brainstorm, validate concept |
| 2 | Design | GDD, narrative, art, audio, mechanics |
| 3 | Planning | Epics, stories, sprints |
| 4 | Engine | UE5 (or other) implementation specs |

Between phases, **handoff contracts** verify that the previous phase
has produced everything the next phase needs.

**Your job during the tutorial:** just read and confirm each step. No
writing required.

👉 Ready? Type `*next` to continue. Or `*exit-tutorial` to leave.

---

## Step 2 — Project Setup

I'm creating a sandbox project called **Cosmic Explorer** for the
tutorial. This won't touch your real project.

```
_gdks/_state/tutorial-state.yaml            ← sandbox state
_gdks-output-tutorial/                       ← sandbox outputs
├── 01-ideation/
│   └── concept-brief.md
├── 02-design/
│   ├── gdd/main.md
│   ├── core-design/
│   │   ├── pillars/design-pillars.md
│   │   ├── mechanics/core-mechanics.md
│   │   ├── loops/gameplay-loops.md
│   │   └── progression/progression-system.md
│   └── level-design/levels.md
├── 03-planning/
│   ├── roadmap.md
│   ├── epics.md
│   ├── stories.md
│   └── sprint-plan.md
└── 04-engine/
    └── architecture.md
```

These are example files so you can see what a completed GDKS project
looks like. In your real project, **you** create these files (with
the agents' help).

👉 Type `*next` to meet your first agent.

---

## Step 3 — Meet Sparky (Ideation)

Phase 1 is run by the **Ideation team**. Let me introduce one of its
agents:

> ### 💡 Sparky — Concept Brainstormer
>
> Sparky helps you explore the space of possible games before
> committing. They ask questions like: "What's the core emotional
> fantasy? What do players feel after 30 seconds? 10 minutes?"

Sparky has a menu of commands (triggers):
- `*brainstorm` — Creative brainstorming session
- `*find-hook` — Identify what makes your game unique
- `*scope-analysis` — Reality-check the project size

In your real project, you'd load Sparky into your IDE and chat with
them. For the tutorial, I'll show you the output Sparky produces.

👉 Type `*next` to see the `concept-brief.md` Sparky generates.

---

## Step 4 — Run `*brainstorm`

Here's the `concept-brief.md` Sparky helped create for Cosmic
Explorer. Read it — this is what your ideation output will look like:

```markdown
# Cosmic Explorer — Concept Brief

## Core Concept
A lost astronaut navigates a derelict space station using
low-gravity mobility and light-puzzle mechanics. Quiet, atmospheric,
introspective — not a shooter.

## Target Audience
Players 18-35 who enjoy introspective puzzle games (Gris,
Journey, Firewatch). Tolerant of slow pacing, drawn to aesthetic
immersion.

## Hook
"What if exploring a dead space station felt like solving a quiet
poem?" — every room tells a story through environment, not dialogue.

## Core Verb
**Drift** — players move in low-gravity with momentum-based control.
Not running, not walking. Intentionally unfamiliar.

## Reference Games
- Gris (art direction)
- Journey (pacing)
- The Witness (environmental storytelling)

## Rough Scope
Solo dev, 6 months, ~2 hours of gameplay.
```

**Key insight:** notice what Sparky did NOT do — they didn't lock in
specific levels or enemies. That's Design's job (phase 2). Sparky
stays at the "vision" altitude.

👉 Type `*next` to see the handoff.

---

## Step 5 — Handoff to Design

Between phases, GDKS runs a **contract check**. Before Phase 1 can
hand off to Phase 2, you'd run:

```bash
$ gd-ks validate --phase=1
✓ concept-brief — 400 words, has Core Concept, has Target Audience
✓ mechanics-exploration — 250 words, has Core Mechanics
✓ ideation-handoff — 120 words
✓ gate:no_open_blockers
✓ gate:min_completion_pct — Phase completion 85% ≥ required 75%

✓ PASSED — 5/5 checks
  You can proceed with: gd-ks handoff --from=1 --to=2
```

The contract is `_gdks/_contracts/phase-01-to-02.contract.yaml`. You
can customize it: add your own required docs, change word counts,
etc.

**This is the thing that prevents the v0.2 bug** — Design can't
start until Ideation is *actually* done.

👉 Type `*next` to enter Phase 2 (Design).

---

## Step 6 — Create GDD (Design)

Phase 2 is run by a much bigger team — **18 agents** under various
sub-specialties. The lead is Diana.

> ### 🎨 Diana — Game Design Director
>
> Diana owns the GDD (Game Design Document). She coordinates with
> narrative, art, audio, and core-design specialists to produce a
> document the Engine team can implement from.

Here's a slice of the Cosmic Explorer GDD that Diana would build:

```markdown
# Cosmic Explorer — Game Design Document

## Pillars
1. **Weightlessness as Mechanic** — Gravity isn't a bug, it's the
   core feel.
2. **Environmental Storytelling** — No dialogue. Rooms tell stories.
3. **Quiet Moments Matter** — Pacing rewards stillness.

## Core Mechanics
- **Drift** — Momentum-based movement in low gravity
- **Anchor** — Latch onto a surface to regain control
- **Resonance Pulse** — Brief sound emission that reveals hidden
  paths (once per room)

## Core Loop
Enter room → Drift cautiously → Read environment → Solve navigation
puzzle → Discover narrative fragment → Transition to next room.
```

**Notice:** Diana doesn't write code. She produces specs the Engine
team consumes.

In your real project, you'd spend the bulk of your time in Phase 2.
With the `solo-indie` preset you have 9 design agents to help you
(plus the tutorial's pre-baked examples in `core-design/`).

👉 Type `*next` to continue.

---

## Step 7 — Planning Team Tour

Phase 3 turns the Design output into actionable work.

> ### 📋 Sam — Scrum Master
>
> Sam takes the GDD and breaks it into epics, stories, and sprints.
> They estimate effort and manage the backlog.

For Cosmic Explorer, Sam produced:

```markdown
# Cosmic Explorer — Epics

## Epic 1: Drift Mechanic (Priority: Must)
- Story 1.1: Character drifts with momentum
- Story 1.2: Character can anchor to surfaces
- Story 1.3: Anchor transitions feel responsive

## Epic 2: Resonance Pulse (Priority: Must)
- Story 2.1: Player can emit a pulse once per room
- Story 2.2: Pulse reveals hidden paths visually
- Story 2.3: Pulse has audio feedback

## Epic 3: Level 1 - Airlock (Priority: Must)
- Story 3.1: Airlock layout implemented
- Story 3.2: Tutorial beats integrated with level flow
```

**Key point:** Sam doesn't plan art or music yet — the Engine team
builds in passes. First prototype (gray boxes), then alpha (first
art), then beta (polish).

👉 Type `*next` to see Phase 4.

---

## Step 8 — Engine Team Tour

Phase 4 produces implementation-ready specs for whichever engine
you selected at install.

> ### 🏗️ Ulysses — UE5 Architect
>
> Ulysses (for UE5 projects) designs the class hierarchy and tells
> you what's C++ vs Blueprint. His most useful workflow is
> `*code-prompt` — it takes a story and generates a prompt you can
> paste into Claude/GPT to get working code.

For story 1.1 (Drift), Ulysses' architecture decision might be:

```markdown
# Drift Mechanic — UE5 Architecture

## Classes
- UDriftMovementComponent (C++)
  - Extends UCharacterMovementComponent
  - Custom movement mode: MOVE_LowGravity
  - UPROPERTY tunables: Momentum, Damping, AnchorForce

## Blueprints
- BP_CosmicCharacter (child of base character)
- BP_AnchorSurface (actor marker for anchor-able geometry)

## Code prompt output
(Full prompt ready to paste into an LLM — saved to
 04-engine/prompts/story-1-1-drift.md)
```

**For non-UE5 projects:** the same story would produce Godot or
Unity-specific specs (see `{{#engine}}` conditionals in story
templates).

👉 Type `*next` for the wrap-up.

---

## Step 9 — Wrap-up

You've seen the full GD-KS flow!

### What you learned
- **4 phases** — Ideation → Design → Planning → Engine
- **32 agents** — each a specialist; select via presets
- **Contracts** — prevent premature phase transitions
- **State** — `project-state.yaml` is the single source of truth
- **Engine-agnostic** — Design and Planning don't hardcode UE5

### What to do next

1. **Exit the tutorial:** type `*exit-tutorial` (your sandbox is
   kept, you can inspect it or delete it).

2. **Start your real project:**
   ```bash
   cd your-project
   npx gd-ks install
   ```

3. **Load your first real agent** in your IDE:
   - Cursor: open `_gdks/ideation/agents/concept-brainstormer.md`
   - Other IDEs: add that file to your AI context

4. **Type `*brainstorm`** in your IDE chat and follow the agent.

### Reference

- **Full docs:** `docs/` — especially `state-and-contracts.md` and
  `presets-guide.md`
- **Roadmap:** `GD-KS-ROADMAP-v0.4.md`
- **CHANGELOG:** `CHANGELOG.md` (what shipped in each version)
- **Report issues:** https://github.com/muriloms/gd-ks/issues

---

**Thanks for trying the tutorial! 🎮**

`*exit-tutorial` to finish. `*tutorial --resume` to replay any time.
