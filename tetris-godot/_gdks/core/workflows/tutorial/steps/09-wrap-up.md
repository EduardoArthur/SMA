# Step 9: Wrap-up 🎉

**Duration:** ~2 minutes

---

You did it. Here's what you now know:

## Mental model you built

```
💡 Ideation  →  🎨 Design  →  📋 Planning  →  ⚙️ Engine
  (Sparky,       (Diana,         (Sam,          (Ulysses,
   Mira,          Marco,          Peter)         Eric)
   Ivy)           Lucas, ...)
     ↓              ↓              ↓              ↓
 concept-brief   GDD             sprint-plan    code-prompt
 mechanics       pillars         stories        (→ paste into LLM)
 handoff         mechanics       roadmap
                 loops
```

Plus **contracts** at every arrow blocking premature handoffs, and
**state** tracking everything across sessions.

## Next steps on a real project

### 1. Reset the tutorial sandbox

```bash
gd-ks tutorial reset
```

This wipes `_gdks/_state/tutorial-state.yaml` and `_tutorial-output/`.
Your real `project-state.yaml` (if any) is untouched.

### 2. Start a real project

```bash
mkdir my-new-game
cd my-new-game
npx gd-ks install
```

Pick a preset (`solo-indie` is a good default), pick your engine
(Unreal Engine 5 for now), and follow the wizard.

### 3. Meet Sparky for real

In your IDE, open `_gdks/ideation/agents/concept-brainstormer.md`, load
it as context, and type `*brainstorm`. She'll guide you through your
actual concept.

### 4. Keep state in sync

After every major change, run:

```bash
gd-ks state inject
```

This refreshes the `<project_state_context>` block inside every
compiled agent.md, so when you next invoke an agent they see the
latest decisions, open questions, and deliverables.

### 5. Recommended reading

- `docs/state-and-contracts.md` — how the memory system works
- `docs/presets-guide.md` — which preset fits your project
- `docs/adding-new-engine.md` — extending GD-KS beyond UE5
- `docs/SPRINT-04-SUMMARY.md` — the Presets & Profiles milestone

### 6. Commands you might use often

```bash
gd-ks state show              # where am I?
gd-ks state history --last=30 # what happened recently?
gd-ks validate --phase=2      # am I ready to hand off?
gd-ks handoff --from=2 --to=3 # advance a phase
gd-ks rollback                # undo a handoff
gd-ks preset switch solo-indie
gd-ks preset show
```

## Feedback welcome

GD-KS is under active development. File issues at
https://github.com/muriloms/gd-ks/issues — especially about:

- Agents that felt redundant (→ preset refinement)
- Docs that were confusing
- Workflows that failed silently
- Ideas for new engines, new workflows, new agents

---

**Thanks for playing.** 🎮

To exit the tutorial:

```bash
gd-ks tutorial --exit
```

Or just type `*exit-tutorial` in your IDE. Your tutorial progress is
saved — you can come back anytime.
