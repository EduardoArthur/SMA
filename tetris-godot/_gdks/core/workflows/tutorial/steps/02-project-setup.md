# Step 2: Project Setup 🧪

**Duration:** ~1 minute

---

## What's happening under the hood

Running in **sandbox mode** means I create `_gdks/_state/tutorial-state.yaml` with the seed project **Cosmic Explorer**:

```yaml
project:
  id: "cosmic-explorer"
  name: "Cosmic Explorer"
  description: "A 2D puzzle game where you navigate a ship through asteroid fields."
preset: "minimal"
target_engine: "unreal-5"
current_phase: 1
```

And I copy pre-prepared sample documents into `_gdks-output/` so you can see exactly what each agent produces.

## The Cosmic Explorer concept

**Genre:** 2D top-down puzzle / navigation
**Core verb:** Nudge (apply thrust to drift past obstacles)
**Hook:** Every asteroid has a physical signature you can read to predict its trajectory
**Scope:** ~12 levels, ~3 hours of content, solo dev friendly

It's intentionally simple. The goal of this tutorial is not to design a real game — it's to see the pipeline end-to-end.

## What just happened

1. ✓ Sandbox `_gdks/_state/tutorial-state.yaml` created
2. ✓ Cosmic Explorer seed data loaded
3. ✓ Sample documents placed in a `_tutorial-output/` folder (separate from your real output)

---

**Next up:** meeting **Sparky** 💡, our first agent. Type `continue` or `next`.
