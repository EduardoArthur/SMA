# GD-KS instructions for ChatGPT Codex

This repository uses the Game Development Knowledge System (GD-KS). Treat the
`_gdks/` directory as framework source material and save generated deliverables
under `_gdks-output/`.

## Starting a GD-KS workflow

1. Read `_gdks/_config/project-config.yaml` and
   `_gdks/_config/manifest.yaml` to understand the installed modules,
   selected engine, language, and output directory.
2. For a requested role or command, read the relevant agent file from
   `_gdks/<module>/agents/*.md` and follow its instructions.
3. For a `*` command, locate the matching workflow below
   `_gdks/<module>/workflows/`, read its `instructions.md`, and use its
   templates when they exist.
4. Write workflow deliverables only in the matching phase folder beneath
   `_gdks-output/` unless the user explicitly requests another location.

## GD-KS command conventions

- `*help`: show the active agent's available commands.
- `*init`: initialize the GD-KS project workflow.
- `*status`: inspect the current project state.
- `*teams`: list installed teams and agents.
- `*tutorial`: run the guided GD-KS tutorial.

## Project-state discipline

- Use `_gdks/_state/project-state.yaml` as the source of truth for current
  phase and established decisions.
- Review prior-phase outputs before creating a new deliverable.
- Run the relevant GD-KS validation or handoff command before claiming a phase
  is ready to advance.
- Preserve existing user work; do not rewrite prior deliverables unless asked.

## Output layout

- `_gdks-output/01-ideation/`: concepts and research.
- `_gdks-output/02-design/`: game design documents and creative direction.
- `_gdks-output/03-planning/`: epics, stories, sprints, and risks.
- `_gdks-output/04-engine/`: implementation architecture and specifications.
