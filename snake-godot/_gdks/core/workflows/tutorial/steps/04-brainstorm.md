# Step 4: Run *brainstorm 💡

**Duration:** ~3 minutes

---

In a real project, typing `*brainstorm` in your IDE launches Sparky's brainstorm workflow. She walks you through questions like:

- What emotional journey should the player take?
- What's the core verb?
- What genres do you want to blend?
- Who's the target audience?

For Cosmic Explorer, here's what she would produce. Open the sample:

```
_tutorial-output/01-ideation/concept-brief.md
```

## What's in the sample concept-brief

```markdown
# Concept Brief: Cosmic Explorer

## Core Concept

A 2D top-down puzzle game where the player pilots a small ship through
debris fields. Rather than "fly fast," the core verb is **nudge** — apply
small thrusts to redirect your trajectory past obstacles that would
destroy you on contact.

## Target Audience

Puzzle fans, people who enjoy FTL's tactical pause-and-think pacing.
Age 14+, no gore.

## Hook

Every asteroid has a subtle visual signature (rotation, color shift)
that telegraphs its mass and trajectory. Skilled players "read" the field
before acting. Low actions per minute, high insight per action.

## Scope

- 12 hand-designed levels
- 3 enemy types (asteroid classes)
- ~3 hours of content
- Solo developer, 6-month timeline
```

Notice: **no mechanics details yet, no systems map, no art direction.** That's deliberate. Sparky's job is just the concept. Mira (mechanics-explorer) and Ivy (ideation-coordinator) fill in the rest.

## What gets tracked in state

After `*brainstorm` completes, `project-state.yaml` updates:

```yaml
phase_progress:
  "1":
    status: "in_progress"
    completion_pct: 25
    deliverables:
      - id: "concept-brief"
        path: "_gdks-output/01-ideation/concept-brief.md"
        verified: true
```

And an event is logged:

```json
{"timestamp":"...","type":"deliverable_added","agent":"sparky","id":"concept-brief"}
```

## The handoff rule

After all 3 ideation deliverables (`concept-brief`, `mechanics-exploration`, `ideation-handoff`) are ready, you run:

```bash
gd-ks validate --phase=1    # check we're ready to hand off
gd-ks handoff --from=1 --to=2    # advance to Design
```

Skipping this is possible (`--force`), but the contract is there for a reason.

---

**Next up:** Step 5 — the handoff to Design, with Diana the Game Design Director. Type `continue` or `next`.
