# GD-KS instructions for ChatGPT Codex

Apply this protocol when the user invokes a GD-KS command or asks to use a
GD-KS agent or workflow. Use the communication language and output folder
configured in `_gdks/_config/project-config.yaml`.

## Load and route the request

1. Read the project configuration, `_gdks/_config/manifest.yaml`, and
   `_gdks/_state/project-state.yaml` if present. Use installed modules, the
   selected engine, and recorded decisions as context. Installer defaults and
   tutorial examples are not confirmed answers to workflow questions.
2. Start with `_gdks/core/agents/gdks-master.md` for core commands or when
   no agent is active. Resolve the command through the active agent's menu.
   If the compiled Markdown omits the action mapping, check the corresponding
   `.agent.yaml`. Resolve `{module}` to the installed module directory under
   `_gdks/`. If the active menu has no match, look for the exact trigger in
   the installed agents' menus. If there is no match or multiple matches,
   ask which agent or workflow to use. For `*help`, show the menu and wait
   for the user to choose.
3. For a workflow, read its `workflow.yaml`, `instructions.md`, and the
   referenced files for the current step. Use the workflow's inputs, ordered
   steps, and templates. Check prerequisites and report missing dependencies.

## Run workflows interactively

- Start at the first pending step, using saved progress when resuming. Follow
  the workflow's sequence and pause at each input or confirmation point.
- Ask the questions for the current step in the chat. Reuse answers the user
  already gave or decisions already confirmed. Ask for missing information
  and present any choices or proposed defaults as options.
- End the reply when waiting for input or approval. Continue in the next
  turn after receiving the answer. Silence, sample content, and suggested
  defaults do not count as acceptance. A request to start a workflow is not
  permission to answer its questions on the user's behalf.
- Complete only the authorized steps. Write deliverables to the configured
  output folder at the workflow's generation step. Preserve existing user work
  and wait for any required approval before saving decisions, marking steps
  complete, or starting a handoff. Use the GD-KS contract check before a phase
  handoff.

## Core workflows

- `*init`: Follow `_gdks/core/workflows/workflow-init/`. Gather the project
  name, genre, description, team size, timeline, experience with the selected
  engine, and documentation needs. Ask for anything missing and wait. Then
  recommend Quick, Standard, or Full and wait for an explicit track choice
  before writing `gdks-workflow-status.yaml`. The installed preset does not
  select the workflow track.
- `*tutorial`: Follow `_gdks/core/workflows/tutorial/` and its step files.
  In normal mode, show the current step and wait for the user's continuation
  before presenting the next one. Use fast, resume, or skip behavior only
  when explicitly requested according to the tutorial instructions. Keep
  tutorial progress and sample decisions in the tutorial sandbox.
