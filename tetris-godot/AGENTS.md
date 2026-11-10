# GD-KS instructions for ChatGPT Codex

Apply this protocol when the user invokes a GD-KS command or asks to use a
GD-KS agent or workflow. Use the communication language and output folder
configured in `_gdks/_config/project-config.yaml`.

## Activate and route the agent

1. Read the project configuration, `_gdks/_config/manifest.yaml`, and
   `_gdks/_state/project-state.yaml` if present. Use installed modules, the
   selected engine, and confirmed decisions as context.
2. Treat `@<agent-id>` as a request to activate that installed GD-KS agent.
   For core commands or when no agent is active, start with
   `_gdks/core/agents/gdks-master.md`. Resolve `*<command>` through the
   active agent's menu, consulting its `.agent.yaml` for action mappings.
   Resolve `{module}` to the installed directory under `_gdks/`. If the
   active menu has no match, look for the exact trigger in other installed
   agents' menus. If routing is ambiguous, ask the user to choose.
3. Read the selected agent's Markdown before responding. Apply its persona,
   communication style, principles, and critical actions. Identify the
   active agent by name in its first response, and keep that agent active
   for subsequent workflow steps until the user switches. For a bare
   `@<agent-id>` or `*help`, show the agent's menu and wait for a choice.
4. When activating a specialist for an explicitly requested agent or
   workflow, delegate a read-only contribution for the current step to the
   matching registered Codex subagent when available. The name is the agent
   ID with a single `gdks-` prefix (e.g. `gdks-concept-brainstormer`).
   Wait for its result and use it in the main chat. Keep the user dialogue,
   workflow progress, and file writes in the main chat. If native delegation
   is unavailable, apply the loaded GD-KS agent's instructions directly.
5. For workflows, read `workflow.yaml`, `instructions.md`, and the files
   referenced by the current step. Use the ordered steps and templates.
   Check prerequisites and report missing dependencies.

## Run workflows interactively

- Start at the first pending step. Pause at each input or confirmation point.
- For a decision with predefined choices, use the session's native question
  tool when exposed (such as `request_user_input` or
  `request_user_input_async`). Send the workflow's choices as selectable
  options. Await the actual selection before continuing. If the tool is
  unavailable or cannot show all the choices, show a numbered menu in the
  chat and wait for a reply.
  Ask for open-ended inputs in the chat.
- Reuse explicit answers and confirmed decisions. Installer defaults, example
  content, proposed choices, and unanswered or cancelled prompts do not
  count as consent. End the reply if input is still pending.
- Complete only the authorized steps. Write deliverables to the configured
  output folder at the workflow's generation step. Preserve existing user
  work and wait for required approvals before saving decisions, completing
  steps, or starting a handoff. Use the GD-KS contract check before handoff.

## Core workflows

- `*init`: Follow `_gdks/core/workflows/workflow-init/`. Gather the project
  name, genre, description, team size, timeline, experience with the selected
  engine, and documentation needs. Ask for anything missing and wait. Then
  recommend Quick, Standard, or Full and wait for an explicit track choice
  before writing `gdks-workflow-status.yaml`. The installed preset does not
  select the workflow track.
- `*tutorial`: Follow `_gdks/core/workflows/tutorial/` and its step files.
  In normal mode, wait for the user's continuation before showing the next
  step. Use fast, resume, or skip behavior only when explicitly requested.
  Keep tutorial progress and sample decisions in the tutorial sandbox.
