# Step 5: Handoff to Design 📤

**Duration:** ~2 minutes

---

Let's talk about the most important architectural concept in GD-KS v0.4:
**handoff contracts**.

## The problem contracts solve

Before v0.4, it was easy to leave Ideation prematurely. The Design team
would start writing a GDD without:

- A clear concept
- Explored mechanics
- A real target audience

Result: halfway into Design, someone realizes the concept is wrong, and
everyone backtracks. Expensive.

## How contracts work

In `_gdks/_contracts/phase-01-to-02.contract.yaml`:

```yaml
schema_version: "1.0"
from_phase: 1
to_phase: 2
name: "Ideation → Design Handoff"

required_deliverables:
  - id: concept-brief
    path_glob: "_gdks-output/01-ideation/concept-brief.md"
    min_word_count: 300
    required_sections:
      - "Core Concept"
      - "Target Audience"

  - id: mechanics-exploration
    path_glob: "_gdks-output/01-ideation/mechanics-exploration.md"
    min_word_count: 200
    required_sections: ["Core Mechanics"]

  - id: ideation-handoff
    path_glob: "_gdks-output/01-ideation/ideation-handoff.md"

quality_gates:
  - type: no_open_blockers
  - type: min_completion_pct
    value: 75
```

## Validating

```bash
$ gd-ks validate --phase=1

🔍 Validating handoff: Phase 1 → Phase 2

  ✓ concept-brief
  ✓ mechanics-exploration
  ✓ ideation-handoff
  ✓ gate:no_open_blockers
  ✓ gate:min_completion_pct

✓ PASSED — 5/5 checks
```

## Executing the handoff

```bash
$ gd-ks handoff --from=1 --to=2

🚦 Handoff: Phase 1 → Phase 2

✓ Handoff completed
  5/5 contract checks passed
  Checkpoint: _gdks/_state/checkpoints/phase-01-ideation-2026-04-20-....yaml

→ You are now in Phase 2. Load the appropriate agents and continue.
```

A few things happened in order:

1. **Contract validated** — all required deliverables + gates pass.
2. **Checkpoint saved** — if something breaks later, you can
   `gd-ks rollback` to here.
3. **State mutated** — phase 1 marked `completed`, phase 2 marked
   `in_progress`, `current_phase` = 2.
4. **Event logged** — `type: "handoff"` in `events.ndjson`.

## If the contract fails

You see what's missing:

```
✗ FAILED — 2 check(s) failed, 3 passed
  ✗ mechanics-exploration — Too short: 80 words (need 200+)
  ✗ gate:min_completion_pct — Phase completion 50% < required 75%
```

Options:

- **Fix it:** update the docs, re-run validate.
- **Force it:** `--force` flag. Logged as `forced: true` in audit.
- **Abandon:** stay in phase 1 and iterate.

---

**Next up:** Step 6 — we're in Design phase. Meet Diana. Type `continue` or `next`.
