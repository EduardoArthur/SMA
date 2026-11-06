# Step 8: Engine Team Tour ⚙️

**Duration:** ~2 minutes

---

Final phase. This is where specs turn into implementation guidance.

## Who's active for Cosmic Explorer?

With `minimal` preset + `target_engine: unreal-5`:

- **Ulysses** 🏗️ — UE5 Architect
- **Eric** 📋 — Engine Coordinator

(Priscilla, Simon-UE5, Bella join in bigger presets.)

## Ulysses's workflows

```
*ue5-architecture      Design the UE5 class hierarchy
*class-specs           C++ class specifications
*blueprint-specs       Blueprint specifications
*code-prompt           Generate LLM prompt for implementation
```

## The "code prompt" trick

This is the most unique workflow in GD-KS. When you run `*code-prompt`, Ulysses doesn't write code. Instead, he produces a **meticulously formatted prompt** for you to paste into Claude, ChatGPT, or any LLM.

Open the sample:

```
_tutorial-output/04-engine/code-prompt.md
```

Excerpt:

```markdown
# Code Generation Prompt — Story CE-017: Nudge Mechanic

You are implementing a single story from a larger Unreal Engine 5
project. Read the context carefully before writing any code.

## Project Context

- **Engine:** Unreal Engine 5.3+
- **Game:** Cosmic Explorer (2D top-down puzzle)
- **Pillar:** "Execute with precision"

## Story

### CE-017: Implement Nudge Mechanic

[... full acceptance criteria ...]

## Existing Code Structure

- `Source/CosmicExplorer/Core/` — base classes
- `Source/CosmicExplorer/Player/` — player types (add your new class here)
- `Content/Data/` — DataTables for tuning

## What to produce

1. **One new C++ class:** `UNudgeMovementComponent` extending
   `UCharacterMovementComponent`
2. **Properties:**
   - `float ThrustDurationMs = 150.f` (UPROPERTY EditAnywhere)
   - `float MaxVelocityUnitsPerSecond = 400.f` (UPROPERTY EditAnywhere)
   - ... etc

## Reject if

- You suggest using UE4 APIs — we're UE5-only
- You produce Blueprints instead of C++ — this should be C++
- You skip the EditAnywhere decorators — designers need to tune

## Output format

Header and .cpp, with no explanation before or after the code blocks.
```

## Why this workflow

Three reasons:

1. **Auditable** — every prompt and its response gets saved.
2. **Safe** — no runaway code generation. You review prompts before sending.
3. **Portable** — works with any LLM. Not tied to one vendor.

In Sprint 6 (planned for v0.5) we'll add an `--auto-implement` flag that sends the prompt to an API and saves the response. But the manual flow always remains available.

## The last handoff

After Engine generates prompts for each story, you're out of GD-KS's scope — the real implementation happens in your IDE / LLM conversations. GD-KS is done when the blueprint for building is clear.

---

**Next up:** Step 9 — wrap-up and next steps. Type `continue` or `next`.
