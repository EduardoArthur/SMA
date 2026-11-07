# Step 7: Planning Team Tour 📋

**Duration:** ~2 minutes

---

Phase 3 turns the GDD into executable work. For `minimal` preset,
only two planning agents are active:

- **Sam** 📊 — Scrum Master
- **Peter** 🎯 — Planning Coordinator

(For `studio` preset you'd also have Tina — Technical Producer — and
Dana — Documentation Specialist.)

## Sam's workflow

```
*create-epics          Read GDD, turn it into epics
*create-stories        Break epics into user stories
*sprint-planning       Organize stories into sprints
```

## What Cosmic Explorer's planning looks like

Open the sample:

```
_tutorial-output/03-planning/sprint-plan.md
_tutorial-output/03-planning/epics.md
_tutorial-output/03-planning/stories.md
_tutorial-output/03-planning/roadmap.md
```

Here's a slice of `stories.md`:

```markdown
# Story CE-017: Implement Nudge mechanic (core verb)

## Description

The player can apply thrust to their ship in 8 directions. Thrust
duration is short (150ms). Momentum carries them through the level
afterward.

## Acceptance Criteria

- [ ] Player moves 8 directions via WASD / analog stick
- [ ] Thrust applies for exactly 150ms
- [ ] Momentum persists until next input
- [ ] Max velocity cap of 400 units/s

## Implementation Notes

{{#engine unreal-5}}
### Unreal Engine 5 Implementation

- Implement as `UCharacterMovementComponent` subclass
- Expose thrust duration via UPROPERTY(EditAnywhere)
- Profile with Unreal Insights
{{/engine}}
```

Notice the **template conditional** `{{#engine unreal-5}}`. When this template renders for Cosmic Explorer (which targets UE5), the block is shown. If Cosmic Explorer was a Godot project, a different block would render.

This is Sprint 3's **engine-agnostic layer** working together with Sprint 4's **presets** — design/planning stay universal, engine specifics live in their module.

## Planning contract

```yaml
# phase-03-to-04.contract.yaml
required_deliverables:
  - id: roadmap
    required_sections: ["Timeline", "Milestones"]
  - id: epics
    required_sections: ["Epic"]
  - id: sprint-plan
  - id: stories
    required_sections: ["Story"]
```

Same structure as all other contracts — Engine team can't start without
them.

---

**Next up:** Step 8 — the Engine team. Meet Ulysses. Type `continue` or `next`.
