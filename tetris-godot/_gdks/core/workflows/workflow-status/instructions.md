# Workflow: Check Workflow Status

## Purpose
This workflow displays your current progress through the GD-KS development pipeline and provides recommendations for next steps.

## When to Use
- To see where you are in the development process
- To get recommendations for what to do next
- To review completed deliverables
- To plan your next work session

---

## What This Workflow Does

### 1. Loads Your Status
Reads `_gdks-output/gdks-workflow-status.yaml` to understand your current progress.

### 2. Analyzes Progress
- Which phases are complete
- Which deliverables exist
- Current phase and agent
- Time since last activity

### 3. Recommends Next Steps
Based on your track and progress, suggests:
- Which agent to load
- Which workflow to run
- What deliverables to create next

---

## Status Report Format

```
╔══════════════════════════════════════════════════════════════╗
║                    GD-KS WORKFLOW STATUS                      ║
╚══════════════════════════════════════════════════════════════╝

Project: [Your Project Name]
Track: [Quick/Standard/Full]
Current Phase: [Ideation/Design/Planning/Engine]

PROGRESS
────────────────────────────────────────
🧠 Ideation    [██████████] 100%  ✓
🎨 Design      [████░░░░░░]  40%  ◐
📋 Planning    [░░░░░░░░░░]   0%  ○
⚙️ Engine      [░░░░░░░░░░]   0%  ○

DELIVERABLES
────────────────────────────────────────
✓ concept-brief.md
✓ market-analysis.md
◐ gdd/core-gameplay.md (in progress)
○ level-design/level-01.md

RECOMMENDATION
────────────────────────────────────────
Continue with: @game-design-director
Suggested workflow: *create-gdd
Next deliverable: Complete GDD core sections
```

---

## Understanding the Icons

| Icon | Meaning |
|------|---------|
| ✓ | Completed |
| ◐ | In Progress |
| ○ | Not Started |
| █ | Progress filled |
| ░ | Progress empty |

---

## Quick Actions After Status Check

Based on the recommendations:

1. **Load suggested agent**: Copy the `@agent-name` from the recommendation
2. **Run suggested workflow**: Type the `*workflow` command shown
3. **Create missing deliverable**: Follow the workflow instructions

---

## If Status File Doesn't Exist

If you haven't initialized the project yet, run:
```
*init
```

This will create the workflow status file and set up tracking.

---

## Troubleshooting

### "Status file not found"
- Run `*init` to create the status file
- Or check if `_gdks-output/` folder exists

### "Outdated status"
- The status may not reflect manual file changes
- Update status manually or re-run relevant workflows

---

*This workflow is read-only - it displays information but doesn't modify your project.*
