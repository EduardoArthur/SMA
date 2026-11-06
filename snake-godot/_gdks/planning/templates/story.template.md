# Story: {{story.title}}

> Template for engine-aware user stories. Rendered via
> `src/core/templates/template-conditionals.js`.
>
> Use `{{#engine <id>}}...{{/engine}}` for engine-specific notes.

## Description

{{story.description}}

## Acceptance Criteria

- [ ] {{story.criterion_1}}
- [ ] {{story.criterion_2}}

## Implementation Notes

{{#engine unreal-5}}
### Unreal Engine 5 Implementation

- Implement core gameplay logic as a C++ `UObject` subclass
- Expose designer-tunable parameters via `UPROPERTY(EditAnywhere, BlueprintReadWrite)`
- Create a Blueprint child class in `Content/` for content-side configuration
- Profile with Unreal Insights; investigate hot paths with `stat unit` / `stat gpu`
- Test in PIE (Play In Editor) first, then in a packaged build

**Suggested class location:** `Source/<ProjectName>/Gameplay/`
{{/engine}}

{{#engine godot-4}}
### Godot 4 Implementation

- Implement core logic in GDScript with **static typing** (`var speed: float = 100.0`)
- Expose designer tunables with `@export` annotations
- Use signals (`signal name(arg: Type)`) for loose coupling between nodes
- Use `_physics_process(delta)` for physics-bound logic, `_process(delta)` for everything else
- For global state, use an autoload (GameManager, EventBus, AudioManager)
- Custom Resources (`.tres`) for content the designer edits
- Profile with Godot's built-in profiler (Debugger → Profiler tab)

**Suggested scene location:** `scenes/gameplay/`
**Suggested script location:** `scripts/gameplay/<name>.gd`
**Scene root node:** choose `CharacterBody2D`/`CharacterBody3D` for player/enemies, `Area2D`/`Area3D` for triggers
{{/engine}}

{{#engine unity-6}}
### Unity 6 Implementation

- Implement core logic as a `MonoBehaviour` component (or `ScriptableObject` for data)
- Expose tunables with `[SerializeField] private` fields, grouped with `[Header]`
- Cache `GetComponent<T>()` calls in `Awake()` — never in `Update()`
- Use ScriptableObject-based event channels over singletons where possible
- Addressables over `Resources.Load` for runtime asset loading
- Never allocate in `Update()`/`FixedUpdate()` (no `new`, no LINQ in hot paths)
- Profile with the Unity Profiler (Window → Analysis → Profiler)

**Suggested folder:** `Assets/_Project/Scripts/Gameplay/`
**Suggested prefab location:** `Assets/_Project/Prefabs/`
**Namespace:** `Game.Gameplay` (or `Game.UI`, `Game.Data`, etc.)
{{/engine}}

{{#engine-not unreal-5}}
{{#engine-not godot-4}}
{{#engine-not unity-6}}
### Engine-Agnostic Implementation

This project is configured for engine-agnostic planning. Implementation
specifics will need to be filled in once an engine is chosen.
{{/engine-not}}
{{/engine-not}}
{{/engine-not}}

## Definition of Done

- [ ] Code implemented and tested
- [ ] Code review approved
- [ ] Tested in editor and in a packaged build
- [ ] Documentation updated

## Dependencies

{{story.dependencies}}
