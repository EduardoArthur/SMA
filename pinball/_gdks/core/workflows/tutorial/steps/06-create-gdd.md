# Step 6: Create GDD — Design Team 🎨

**Duration:** ~3 minutes

---

Welcome to **Phase 2 — Design**. You're now in the biggest team (up to 18 agents in the `studio` preset). For Cosmic Explorer with `minimal` preset, only 5-7 design agents are active:

- **Diana** 🎮 — Game Design Director (GDD)
- **Marco** ⚙️ — Mechanics Designer
- **Lucas** 🔄 — Core Loop Specialist

The other 15 design agents are hidden from this project's `_gdks/design/agents/` folder. If Cosmic Explorer grew later, you'd `gd-ks preset switch small-studio` to bring them in.

## Meet Diana

Diana's job is the **Game Design Document** — the source-of-truth
document that everyone else references. Her menu:

- `*gdd` — create the main GDD
- `*systems` — define game systems
- `*balance` — balance framework
- `*progression` — progression design
- `*features` — feature breakdown
- `*handoff` — prepare handoff to Planning

## What Cosmic Explorer's GDD looks like

Open the sample:

```
_tutorial-output/02-design/gdd/main.md
```

It's ~600 words covering:

- **Pillars:** Read the board → Plan the path → Execute with precision
- **Core Mechanics:** nudge (3 speed), scan (passive info), emergency brake (limited charges)
- **Target Audience:** puzzle fans (as established by Sparky)
- **Scope breakdown:** 12 levels, 3 enemy classes, 3h content

Critical: **nothing here describes how to build it in UE5**. The GDD is engine-agnostic. That happens in Phase 4.

## The contract for Design → Planning

```yaml
# _gdks/_contracts/phase-02-to-03.contract.yaml
required_deliverables:
  - id: gdd-main
    path_glob: "_gdks-output/02-design/gdd/main.md"
    min_word_count: 500
    required_sections: ["Pillars", "Core Mechanics", "Target Audience"]

  - id: design-pillars
    path_glob: "_gdks-output/02-design/core-design/pillars/design-pillars.md"

  - id: core-mechanics-spec
    path_glob: "_gdks-output/02-design/core-design/mechanics/core-mechanics.md"
    min_word_count: 200

  - id: gameplay-loops
    path_glob: "_gdks-output/02-design/core-design/loops/gameplay-loops.md"

  # Narrative docs only required for narrative-heavy / studio presets
  - id: story-bible
    path_glob: "_gdks-output/02-design/narrative/story-bible.md"
    required_when:
      preset: ["narrative-heavy", "studio"]

  # Art bible only required for studios and solo indies
  - id: art-bible
    path_glob: "_gdks-output/02-design/art/art-bible.md"
    required_when:
      preset: ["studio", "small-studio", "solo-indie", "narrative-heavy"]
```

Notice `required_when`. For the `minimal` preset Cosmic Explorer uses,
**story-bible and art-bible are NOT required**. That's the power of
presets — contracts adapt to your scale.

---

**Next up:** Step 7 — Planning team. Meet Sam. Type `continue` or `next`.
