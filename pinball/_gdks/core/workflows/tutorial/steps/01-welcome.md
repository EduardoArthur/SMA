# Step 1: Welcome to GD-KS 🎮

**Duration:** ~2 minutes | **Difficulty:** ⭐ Beginner

---

Hey there! I'm **GameMaster** 🎮, your guide through the GD-KS (Game Development Knowledge System).

In the next 15-20 minutes, I'll walk you through building a tiny sample game called **"Cosmic Explorer"** — a 2D space exploration puzzle. By the end, you'll understand:

- How the 4 phases fit together (Ideation → Design → Planning → Engine)
- What the specialized agents do (and which to call when)
- How contracts prevent you from skipping important documentation
- How the state tracker keeps your project coherent across sessions

## The 4-Phase Pipeline

```
💡 Ideation  →  🎨 Design  →  📋 Planning  →  ⚙️ Engine
    (1)          (2)           (3)            (4)
```

Each phase has its own team, its own deliverables, and a **contract** that must be satisfied before you can advance. Think of it as guard rails, not a straitjacket — you can `--force` past any check when you know what you're doing.

## Modes

- **Normal (default):** ~15-20 minutes with pauses
- **Fast:** `*tutorial --fast` ~5-7 min, no pauses
- **Resume:** `*tutorial --resume` from last checkpoint
- **Skip to:** `*tutorial --skip-to=5` jump to step 5

## Safety

This tutorial runs in **sandbox mode** — it writes to `_gdks/_state/tutorial-state.yaml`, never touching your real `project-state.yaml`. When you're done, you can keep the sample project or run `gd-ks tutorial reset` to clear it.

---

**Ready?** Type `continue` or `next` to move to Step 2 where we'll set up the Cosmic Explorer sandbox. Or type `*exit-tutorial` to leave (your progress will be saved).
