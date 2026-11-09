# Step 3: Meet Sparky (Ideation Team) 💡

**Duration:** ~2 minutes

---

## Who's Sparky?

**Sparky** 💡 is our **Concept Brainstormer**. She's the first agent you'll meet on any real project. Her job is to help you explore ideas before you commit to anything concrete.

```yaml
# From: _gdks/ideation/agents/concept-brainstormer.agent.yaml
name: "Sparky"
title: "Concept Brainstormer"
icon: "💡"
role: "Creative Concept Developer & Idea Catalyst"
```

## How do I "talk" to Sparky?

In your IDE (Cursor, Windsurf, VS Code with Continue, Claude Code), you:

1. **Load the compiled agent markdown** into your AI context.
   Path: `_gdks/ideation/agents/concept-brainstormer.md`

2. **Type a command starting with `*`.** Sparky listens for:
   - `*brainstorm` — creative ideation session
   - `*find-hook` — identify a unique selling point
   - `*scope-analysis` — assess if your idea fits your timeline
   - `*help` — show her menu
   - `*chat` — free-form conversation

## Why does she have that placeholder section?

When you open `concept-brainstormer.md` you'll see a line near the top:

```markdown
<!-- GDKS_STATE_CONTEXT_PLACEHOLDER -->
```

That placeholder gets replaced by the current project state (decisions, deliverables, phase progress) whenever you run `gd-ks state inject`. This is how Sparky "remembers" what decisions you've already made — she just reads the injected context.

In this tutorial, I've already run `gd-ks state inject` for you, so the placeholder is filled with Cosmic Explorer's current state.

## Try it yourself (mentally)

Read the compiled agent and skim her persona. You don't have to actually invoke her right now — we'll simulate her output in the next step.

---

**Next up:** Step 4 — we'll "run" `*brainstorm` and see what concept-brief looks like. Type `continue` or `next`.
