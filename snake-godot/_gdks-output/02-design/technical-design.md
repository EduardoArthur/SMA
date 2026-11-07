# Snake - Technical Design

> **Author:** Theo (Technical Game Designer), GD-KS Design Team
> **Date:** 2026-11-06 · **Version:** 1.0
> **Engine:** Godot 4.7 · GDScript · Renderer: GL Compatibility (already set, needed for web)
> **Implements:** `02-design/gdd/gdd.md` v1.0. Section numbers like "GDD §3.4" refer to it.

---

## 1. Architecture in one picture

```
 ┌──────────────────────── LOGIC (no nodes, testable) ───────────────────────┐
 │  SnakeModel (RefCounted)                                                  │
 │   grid, body[], direction, fruit, score  ──  step(turn) → StepResult      │
 └───────────────────────────────▲───────────────────────────┬───────────────┘
                                 │ step()                    │ result
 ┌───────────── CONTROL ─────────┴───────────────────────────▼───────────────┐
 │  Game (Node2D, main scene): state machine, tick timer, input buffer,      │
 │  turns StepResult into signals on the Events bus                          │
 └──────▲────────────────────────────────────────────────────┬───────────────┘
        │ turn requests                                      │ Events.* signals
 ┌──────┴──────┐                     ┌───────────────────────▼───────────────┐
 │InputHandler │                     │ PRESENTATION (only listens)           │
 │ keys+swipe  │                     │ BoardView, SnakeView, FruitView,      │
 └─────────────┘                     │ Juice (shake, particles, popups),     │
                                     │ HUD/Screens, AudioManager             │
                                     └───────────────────────────────────────┘
```

**Golden rule (GDD §5):** presentation **reads** state and **listens** to signals. It **never** changes game state. Only `Game` calls `SnakeModel.step()`.

Why this split helps a Godot beginner:
- All the tricky rules live in **one plain script** you can test without running the game.
- You can add, remove or break any visual effect without touching the rules.

---

## 2. Project Structure

```
res://
├─ config/
│  ├─ game_config.gd          # class_name GameConfig extends Resource
│  └─ game_config.tres        # the single source of truth for tuning (GDD §9)
├─ autoload/
│  ├─ events.gd               # Events: global signal bus
│  ├─ save_data.gd            # SaveData: best score + settings (ConfigFile)
│  └─ audio_manager.gd        # AudioManager: listens to Events, plays SFX/music
├─ game/
│  ├─ snake_model.gd          # class_name SnakeModel extends RefCounted (pure logic)
│  ├─ step_result.gd          # class_name StepResult extends RefCounted
│  ├─ input_handler.gd        # keyboard + swipe → turn requests
│  ├─ game.gd / game.tscn     # MAIN SCENE: states, tick loop, emits Events
├─ view/
│  ├─ board_view.gd           # draws lawn checkerboard + hedge
│  ├─ snake_view.gd           # draws interpolated snake + face
│  ├─ fruit_view.gd           # fruit sprite + idle/panic
│  └─ juice/
│     ├─ camera_shake.gd
│     ├─ score_popup.tscn
│     └─ particles/*.tscn     # GPUParticles2D won't work everywhere → use CPUParticles2D
├─ ui/
│  ├─ hud.tscn, title.tscn, pause.tscn, game_over.tscn, settings.tscn
├─ assets/
│  ├─ art/  └─ audio/
└─ tests/
   └─ test_snake_model.gd     # headless rule tests (§10)
```

**Autoloads** (Project Settings → Globals → Autoload), in this order: `Events`, `SaveData`, `AudioManager`.

> Note: use **CPUParticles2D**, not GPUParticles2D. GPU particles have limited support with the Compatibility renderer, especially on web and older phones.

---

## 3. Configuration: `GameConfig` resource

```gdscript
# config/game_config.gd
class_name GameConfig
extends Resource

@export var grid_landscape := Vector2i(20, 15)
@export var grid_portrait := Vector2i(15, 20)
@export var start_length := 3
@export var start_delay := 1.0            # s, or first input
@export var speed_start := 7.0            # moves/sec
@export var speed_step := 0.5
@export var fruits_per_step := 3
@export var speed_cap := 15.0
@export var input_buffer_size := 2
@export var points_per_fruit := 10
@export var milestones: Array[int] = [10, 25, 50, 100]
@export var near_miss_cooldown := 1.5
@export var fruit_panic_range := 3        # Manhattan cells
@export var swipe_threshold := 30.0       # canvas units (see §6.2)
@export var gameover_input_lock := 0.5
@export var death_freeze := 0.3
@export var death_sequence := 1.2         # total DYING time before the Game Over screen

func moves_per_sec(fruits_eaten: int) -> float:
    return minf(speed_start + speed_step * floori(fruits_eaten / float(fruits_per_step)), speed_cap)
```

Create `game_config.tres` from this script (FileSystem → New Resource → GameConfig). `Game` loads it with `@export var config: GameConfig`, and passes it to whatever needs it. **No magic numbers anywhere else.**

---

## 4. Logic: `SnakeModel`

### 4.1 Data
| Field | Type | Meaning |
|-------|------|---------|
| `grid_size` | `Vector2i` | 20×15 or 15×20, fixed for the run |
| `body` | `Array[Vector2i]` | `body[0]` = head, last = tail |
| `prev_body` | `Array[Vector2i]` | Copy of `body` before the last step (for interpolation) |
| `direction` | `Vector2i` | Current moving direction (`Vector2i.RIGHT`, …) |
| `fruit` | `Vector2i` | Fruit cell (`Vector2i(-1, -1)` = none / board full) |
| `fruit_type` | `int` | 0 apple, 1 cherry, 2 grapes (cosmetic; see art bible) |
| `fruits_eaten` | `int` | Drives the speed ramp |
| `score` | `int` | |
| `rng` | `RandomNumberGenerator` | Seedable, so **tests are deterministic** |
| `config` | `GameConfig` | Passed in `_init(config)`, used for points, milestones and speed |

> Grid coordinates: `(0,0)` is top-left, x goes right, y goes down. This matches Godot's 2D axes, so `Vector2i.UP == (0,-1)`.

### 4.2 `StepResult`
```gdscript
class_name StepResult
extends RefCounted
var turned := false
var ate := false
var died := false
var death_cause := ""          # "wall" | "self"
var near_miss := false
var near_miss_cell := Vector2i.ZERO
var board_cleared := false
var milestone := 0             # 0 = none, else length reached
var speed_changed := false
```

### 4.3 `reset(grid_size)`
1. Clear the body. Head = grid center `Vector2i(grid_size.x / 2, grid_size.y / 2)`.
2. Direction = `RIGHT` if landscape (`x >= y`), else `UP`.
3. Build `start_length` segments going **backwards** from the head (opposite of the direction).
4. `prev_body = body.duplicate()`, `score = 0`, `fruits_eaten = 0`, `spawn_fruit()`.

### 4.4 `step(turn: Vector2i) -> StepResult`: the heart of the game

`turn` is `Vector2i.ZERO` when there is no buffered input. `Game` has already filtered reversals (§5.3), but the model checks again for safety.

```gdscript
func step(turn: Vector2i) -> StepResult:
    var r := StepResult.new()
    # prev_body is only updated in step 4 (a real move), so on death the view freezes in place.

    # 1. Apply turn (never a reversal, never "same direction")
    var prev_dir := direction
    if turn != Vector2i.ZERO and turn != direction and turn != -direction:
        direction = turn
        r.turned = true

    var head := body[0]
    var new_head := head + direction
    var growing := new_head == fruit

    # 2. Death checks (GDD §3.4) – tail cell is free unless growing
    if not _in_bounds(new_head):
        r.died = true; r.death_cause = "wall"
        return r
    if _is_blocked_by_body(new_head, growing):
        r.died = true; r.death_cause = "self"
        return r

    # 3. Near-miss (GDD §6.2) – only on a turn, never with eating
    if r.turned and not growing:
        var would_have := head + prev_dir
        if not _in_bounds(would_have) or _is_blocked_by_body(would_have, false):
            r.near_miss = true
            r.near_miss_cell = would_have

    # 4. Move
    prev_body = body.duplicate()
    body.push_front(new_head)
    if growing:
        r.ate = true
        fruits_eaten += 1
        score += config.points_per_fruit
        r.speed_changed = fruits_eaten % config.fruits_per_step == 0 \
            and config.moves_per_sec(fruits_eaten) > config.moves_per_sec(fruits_eaten - 1)
        if body.size() in config.milestones:
            r.milestone = body.size()
        if not spawn_fruit():
            r.board_cleared = true
    else:
        body.pop_back()
    return r

func _in_bounds(c: Vector2i) -> bool:
    return c.x >= 0 and c.y >= 0 and c.x < grid_size.x and c.y < grid_size.y

func _is_blocked_by_body(c: Vector2i, growing: bool) -> bool:
    # Exclude the current tail: it moves away this tick (unless we grow).
    var last := body.size() if growing else body.size() - 1
    for i in last:
        if body[i] == c:
            return true
    return false
```

> **Near-miss is checked against the pre-move body**, so it uses the same tail rule as death. The near-miss cooldown (1.5 s) is **presentation timing**, so it lives in `Game`/`SnakeView`, not in the model.
> **Death returns early:** the body is **not** moved, so the view freezes the snake exactly as it was when it hit.

### 4.5 `spawn_fruit() -> bool`
```gdscript
func spawn_fruit() -> bool:
    var free: Array[Vector2i] = []
    for y in grid_size.y:
        for x in grid_size.x:
            var c := Vector2i(x, y)
            if not body.has(c):
                free.append(c)
    if free.is_empty():
        fruit = Vector2i(-1, -1)
        return false
    fruit = free[rng.randi_range(0, free.size() - 1)]
    fruit_type = rng.randi_range(0, 2)
    return true
```
300 cells × body length is tiny, so there's no need to optimize. **Never** use "pick random cell, retry if occupied". It gets slower and slower as the board fills up.

---

## 5. Control: `Game` (main scene)

### 5.1 State machine
```gdscript
enum State { TITLE, READY, PLAYING, PAUSED, DYING, GAME_OVER }
```
| State | Tick runs? | Enter | Exit |
|-------|-----------|-------|------|
| `TITLE` | No | Show title | Any input → `READY` |
| `READY` | No | `model.reset(grid)`, show snake | First direction input **or** `start_delay` → `PLAYING` |
| `PLAYING` | **Yes** | Emit `run_started` once | Death → `DYING`, pause → `PAUSED` |
| `PAUSED` | No | Show pause overlay | Resume → `PLAYING` |
| `DYING` | No | Emit `died`, wait `death_sequence` (**1.2 s**: 0.3 s freeze + bonk + slide whistle, audio bible §9) | → `GAME_OVER` |
| `GAME_OVER` | No | Save best, maybe `new_best`, lock input `gameover_input_lock` | Any input → `READY` |

Use a `match state:` block in `_process` and one `_enter_state(new_state)` function. Don't scatter `if` checks around.

### 5.2 Tick loop (fixed timestep with accumulator)
```gdscript
var _acc := 0.0
var _interval := 1.0 / 7.0

func _process(delta: float) -> void:
    if state != State.PLAYING:
        return
    _acc += delta
    var ticks := 0
    while _acc >= _interval and ticks < 2:       # cap catch-up after a lag spike
        _acc -= _interval
        _do_tick()
        ticks += 1
        if state != State.PLAYING:
            return
    if ticks == 2:
        _acc = minf(_acc, _interval)             # drop extra lag, no "fast-forward"
    tick_alpha = _acc / _interval                # 0..1, read by SnakeView (§7)
```
**Why not a `Timer` node?** The accumulator gives `tick_alpha` for free (needed for smooth sliding) and handles speed changes cleanly. Set `_interval = 1.0 / config.moves_per_sec(model.fruits_eaten)` after every tick that ate a fruit.

### 5.3 Input buffer (GDD §3.2)
```gdscript
var _buffer: Array[Vector2i] = []

func request_turn(dir: Vector2i) -> void:          # called by InputHandler
    if state == State.READY:
        _enter_state(State.PLAYING)                 # first input starts the run
    if state != State.PLAYING:
        return
    var last: Vector2i = _buffer.back() if not _buffer.is_empty() else model.direction
    if dir == last or dir == -last:                 # same dir or reversal of the last planned dir
        return
    if _buffer.size() < config.input_buffer_size:
        _buffer.append(dir)

func _do_tick() -> void:
    _eaten_type = model.fruit_type                  # remember before step() respawns the fruit
    var turn: Vector2i = _buffer.pop_front() if not _buffer.is_empty() else Vector2i.ZERO
    var r := model.step(turn)
    _emit_events(r)
```
Checking reversals against the **last planned** direction is what makes "↑ then ←" from heading right work as a quick U-turn, while "→ then ←" (a buffered reversal) is rejected.

### 5.4 Turning `StepResult` into signals
```gdscript
func _emit_events(r: StepResult) -> void:
    if r.died:
        Events.died.emit(r.death_cause); _enter_state(State.DYING); return
    if r.turned: Events.turned.emit(model.direction)
    if r.ate: Events.fruit_eaten.emit(model.body[0], _eaten_type)   # store type before respawn
    if r.near_miss and _near_miss_cd <= 0.0:
        Events.near_miss.emit(r.near_miss_cell); _near_miss_cd = config.near_miss_cooldown
    if r.milestone > 0: Events.milestone_reached.emit(r.milestone)
    if r.speed_changed:
        _interval = 1.0 / config.moves_per_sec(model.fruits_eaten)
        Events.speed_changed.emit(config.moves_per_sec(model.fruits_eaten))
    if r.board_cleared:
        Events.board_cleared.emit(); _enter_state(State.GAME_OVER)
    Events.score_changed.emit(model.score)
```
> ⚠️ `_eaten_type` (a member `var _eaten_type := 0`) is saved at the start of `_do_tick()`, **before** `step()`, because the model respawns the fruit inside `step`.

---

## 6. Input: `InputHandler`

### 6.1 Keyboard
Input Map actions (Project Settings → Input Map):

| Action | Keys |
|--------|------|
| `move_up` | ↑, W |
| `move_down` | ↓, S |
| `move_left` | ←, A |
| `move_right` | →, D |
| `pause` | Esc, P |
| `confirm` | Enter, Space |

```gdscript
func _unhandled_input(event: InputEvent) -> void:
    if event.is_action_pressed("move_up"):    game.request_turn(Vector2i.UP)
    elif event.is_action_pressed("move_down"): game.request_turn(Vector2i.DOWN)
    elif event.is_action_pressed("move_left"): game.request_turn(Vector2i.LEFT)
    elif event.is_action_pressed("move_right"): game.request_turn(Vector2i.RIGHT)
    elif event.is_action_pressed("pause"):    game.toggle_pause()
```
Use `is_action_pressed` (which ignores key echo when held), so holding a key doesn't flood the buffer.

### 6.2 Swipe (GDD §4)
```gdscript
var _anchor := Vector2.ZERO
var _touching := false

func _unhandled_input(event: InputEvent) -> void:
    if event is InputEventScreenTouch:
        _touching = event.pressed
        _anchor = event.position
        if event.pressed: game.on_tap()           # starts/restarts outside of PLAYING
    elif event is InputEventScreenDrag and _touching:
        var d: Vector2 = event.position - _anchor
        if d.length() >= config.swipe_threshold:
            var dir := Vector2i.RIGHT if d.x > 0 else Vector2i.LEFT
            if absf(d.y) > absf(d.x):
                dir = Vector2i.DOWN if d.y > 0 else Vector2i.UP
            game.request_turn(dir)
            _anchor = event.position              # re-anchor → allows L-shaped double swipe
```
- With `stretch/mode = canvas_items` (already set), event positions arrive in **canvas units**, so a fixed `30.0` behaves the same on every screen size.
- To test swipes on PC, turn on **Project Settings → Input Devices → Pointing → Emulate Touch From Mouse**.
- `game.on_tap()` must do nothing while `PLAYING` (GDD: taps don't pause).
- **"Press any key"** (Title, Game Over): in `InputHandler._unhandled_input`, when the state is `TITLE` or `GAME_OVER`, treat any `InputEventKey` with `pressed and not echo` as `game.on_tap()`, so every key starts the game, not only the mapped actions.

---

## 7. Presentation

### 7.1 Layout & the two grids (GDD §3.1)
- Base window: **1280 × 720**, `stretch/mode = canvas_items`, `aspect = expand` (already set). On a portrait phone the canvas expands vertically.
- `Game` picks the grid **at `READY`**: `grid_landscape if vp.x >= vp.y else grid_portrait` (with `vp = get_viewport_rect().size`).
- `BoardView.layout(vp_size, grid_size)`:
  - HUD band: 80 units at the top
  - `cell = floorf(minf(avail.x / grid.x, avail.y / grid.y))`
  - Board centered in the remaining area. `board_origin` and `cell` are shared with all views (`BoardView.cell_to_pos(c: Vector2) -> Vector2` returns the cell **center**).
- Connect `get_viewport().size_changed` → recompute the layout. If `PLAYING` **and** the orientation flipped → `toggle_pause()`. The grid only changes on the next `READY`.

### 7.2 Smooth slide: `SnakeView` (GDD §5)
Every frame (`_process` → `queue_redraw()`), each segment `i` is drawn at:

```
from = prev_body[i] if i < prev_body.size() else body[i]   # new tail segment appears in place
to   = body[i]
pos  = cell_to_pos(lerp(Vector2(from), Vector2(to), game.tick_alpha))
```
This works because, after a step, `body[i] == prev_body[i-1]`: each segment slides into the cell its neighbor just left, so the body follows the head's exact path.

Drawing in `_draw()`:
1. `draw_polyline(points, body_color, cell * 0.8, true)` through all segment positions
2. `draw_circle(p, cell * 0.4, body_color)` at every point, which gives **rounded corners and ends**
3. Head on top: a slightly bigger circle, with eyes rotated to `direction`, and the face texture/state from `ExpressionController`

- **Death:** `Game` stops calling `step()` and freezes `tick_alpha` at its current value, so the snake freezes mid-slide. The "bump" is a short tween of the head offset toward `direction` (about 0.2 cell).
- **Face turns instantly:** the head's facing uses `model.direction` (already updated at the tick), not the interpolated motion.
- Performance: 300 circles + 1 polyline per frame is trivial.

### 7.3 Expressions (GDD §6.3)
`ExpressionController` (child of `SnakeView`) holds `current: StringName` and a `Timer`. When an event arrives, it changes expression only if the new one has **higher or equal priority** than the current temporary one. It returns to `neutral` when the timer runs out. Priority array: `[&"faint", &"cool", &"gasp", &"proud", &"happy", &"neutral"]`.

### 7.4 Juice (GDD §6.5)
| Piece | Implementation |
|-------|---------------|
| Screen shake | `Camera2D` + `camera_shake.gd`: `shake(strength_px, duration)` → random `offset` that decays to 0 each frame. Return early if `SaveData.screen_shake == false`. |
| Squash | `Tween` on the head draw scale (`Vector2(1.3, 0.7)` → `ONE`, 0.15 s, `TRANS_BACK`) |
| Particles | `CPUParticles2D` scenes with `one_shot = true`, instantiated at `cell_to_pos(cell)`, freed on `finished` |
| "+10" popup | `Label` scene, tween position up 1 cell and fade alpha over 0.6 s, then `queue_free()` |
| Draw order | `BoardView` (z 0) → particles/popups (z 1) → `FruitView` (z 2) → `SnakeView` (z 3). Effects stay **under** the snake (readability rule). |

### 7.5 Fruit (GDD §6.4)
`FruitView` reads `model.fruit` and `model.fruit_type`. Panic = Manhattan distance `abs(dx) + abs(dy) <= config.fruit_panic_range` from the head, which switches to a panic face and adds a sine shake to the draw offset. It is visual only.

---

## 8. Events bus (`autoload/events.gd`)

```gdscript
extends Node
signal run_started
signal turned(dir: Vector2i)
signal fruit_eaten(cell: Vector2i, fruit_type: int)
signal near_miss(cell: Vector2i)
signal milestone_reached(length: int)
signal speed_changed(moves_per_sec: float)
signal died(cause: String)
signal board_cleared
signal new_best(score: int)
signal score_changed(score: int)
signal state_changed(state: int)
```
Only `Game` **emits**. Everyone else **connects** in `_ready()`, for example `Events.fruit_eaten.connect(_on_fruit_eaten)`.

---

## 9. Persistence & Platform

### 9.1 `SaveData` (autoload)
```gdscript
extends Node
const PATH := "user://save.cfg"
var best := 0
var music_volume := 0.8
var sfx_volume := 0.8
var screen_shake := true

func _ready() -> void: load_data()

func load_data() -> void:
    var cf := ConfigFile.new()
    if cf.load(PATH) != OK: return               # first launch: keep defaults
    best = cf.get_value("score", "best", 0)
    music_volume = cf.get_value("settings", "music", 0.8)
    sfx_volume = cf.get_value("settings", "sfx", 0.8)
    screen_shake = cf.get_value("settings", "shake", true)

func save_data() -> void:
    var cf := ConfigFile.new()
    cf.set_value("score", "best", best)
    cf.set_value("settings", "music", music_volume)
    cf.set_value("settings", "sfx", sfx_volume)
    cf.set_value("settings", "shake", screen_shake)
    cf.save(PATH)
```
Save on `GAME_OVER` and whenever a setting changes. On web, `user://` is stored in the browser's **IndexedDB**, so no special code is needed.

### 9.2 Focus & pause
```gdscript
func _notification(what: int) -> void:
    if what == NOTIFICATION_APPLICATION_FOCUS_OUT and state == State.PLAYING:
        toggle_pause()
```

### 9.3 Audio (`AudioManager`)
- Buses: `Master` → `Music`, `SFX` (create them in the Audio tab). Volume uses `AudioServer.set_bus_volume_db(idx, linear_to_db(v))`.
- A small pool of about 4 `AudioStreamPlayer` nodes for SFX, so overlapping chomps don't cut each other off.
- **Web:** browsers block audio until the user interacts. Start the music on the **first input** (title screen "press any key / tap"). The flow already requires that input.

### 9.4 Web export checklist (do this in **week 1**, GDD F14)
1. Editor → Manage Export Templates → download the templates for 4.7.
2. Project → Export → Add **Web**. Leave **Thread Support OFF**: a single-threaded build runs on any static host (itch.io, GitHub Pages) without special headers.
3. Export to `build/web/` (add `build/` to `.gitignore`). Test with the editor's **Remote Debug → Run in Browser** button, or any local web server (opening `index.html` directly from the disk **won't work**).
4. Check on a phone: swipes, audio after the first tap, best score kept after a reload, and the pause when switching tabs.

---

## 10. Testing the rules (no addon needed)

`tests/test_snake_model.gd` runs headless from the command line:
```bash
godot --headless -s res://tests/test_snake_model.gd
```
```gdscript
extends SceneTree
func _init() -> void:
    var failed := 0
    failed += _test_reversal_ignored()
    # ... one function per case below
    print("FAILED: %d" % failed if failed else "ALL PASSED")
    quit(failed)
```

| # | Case | Expect |
|---|------|--------|
| T1 | Heading right, `step(LEFT)` | Direction stays right, `turned == false` |
| T2 | Head next to the right wall, `step(ZERO)` | `died`, cause `"wall"` |
| T3 | Snake in a tight loop, head moves into the cell the **tail is leaving** | **Not** dead |
| T4 | Same as T3 but the fruit is on that cell (growing) | Dead (`"self"`) |
| T5 | Fruit directly ahead | `ate`, length +1, score +10, new fruit not on the body |
| T6 | Turn away from a wall that was directly ahead | `near_miss == true` |
| T7 | Turn in open space | `near_miss == false` |
| T8 | Eat while turning away from danger | `ate == true`, `near_miss == false` |
| T9 | Fill a tiny grid (3×1) | `board_cleared == true` |
| T10 | `moves_per_sec(0/3/6/48/99)` | 7.0 / 7.5 / 8.0 / 15.0 / 15.0 |
| T11 | Seeded `rng` | Same fruit sequence every run |

The input buffer rules (§5.3) are in `Game`. Test them by hand with the checklist below. Moving `request_turn` into a small `InputBuffer` class is the next step if they become buggy.

**Manual feel checklist:** ↑→ quick U-turn works at 15 mps · holding a key doesn't stack turns · death never feels "early" · the snake slides with no jitter at 7 and 15 mps · a fast double swipe works · rotating a phone mid-run pauses the game.

---

## 11. Implementation Order (maps to GDD §8)

| Step | Build | Done when |
|------|-------|-----------|
| 1 | `GameConfig` + `SnakeModel` + tests T1-T11 | `ALL PASSED` headless |
| 2 | `Game` tick loop + keyboard + `BoardView` + **snap** `SnakeView` (no interpolation yet) + fruit square | Playable with colored squares |
| 3 | States, HUD, Game Over, `SaveData` | Full loop with a saved best score |
| 4 | `Events` bus wired up | Signals print in the Output panel |
| 5 | **Web export check** (§9.4) | Runs in a phone browser |
| 6 | Smooth slide (§7.2) | No jitter at 15 mps |
| 7 | Expressions, juice, audio | GDD §6.5 table done |
| 8 | Swipe, two grids/rotation, settings, fruit panic, PERFECT | GDD "Should/Could" done |

---

## 12. Technical Risks

| Risk | Mitigation |
|------|-----------|
| Input feels laggy (inputs apply at the next tick) | Standard for grid games. The head's facing updates on the tick. At 7 mps the worst case is 143 ms. If it feels bad, apply the turn **immediately** when `tick_alpha < 0.15` (an "early-tick grace"). Treat this as a tuning option. |
| Jitter in the smooth slide | Interpolation uses `tick_alpha` from the same `_process` frame. Turn on V-Sync. **Don't** use `_physics_process` for the tick. |
| Lag spike = several ticks at once = unfair death | Catch-up is capped at 2 ticks per frame (§5.2) and auto-pause on focus loss |
| Web audio silent | Start music after the first input (§9.3) |
| Phone rotation mid-run | Auto-pause + relayout; the grid is fixed per run (§7.1) |

---
*Technical design by Theo, GD-KS Design Team. Next: art bible (Aurora) → audio bible (Anton) → Design handoff (Dylan) → Engine Team.*
