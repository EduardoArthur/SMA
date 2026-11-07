# Snake - Art Bible

> **Author:** Aurora (Art Director), GD-KS Design Team
> **Date:** 2026-11-06 · **Version:** 1.0
> **Production method:** **Everything is drawn in code** with Godot `_draw()` (circles, arcs, polygons, polylines). There are no sprite files. The only external assets are 1 font and the app icon.
> **Mood:** **Sunny garden.** Cheerful daytime, fresh greens, a golden snake, juicy fruit.
> **Implements:** GDD §6-7 and Technical Design §7.

---

## 1. Style Pillars

| # | Pillar | In practice |
|---|--------|-------------|
| A1 | **Round & soft** | No sharp corners anywhere: circles, capsules, rounded rectangles |
| A2 | **Outlined & readable** | Every game object has the same dark-brown outline, so it pops off the lawn |
| A3 | **The face is the star** | The snake's head is the most detailed thing on screen. Everything else stays simple. |
| A4 | **Calm board, loud events** | The lawn and hedge are quiet. Color and motion are saved for events (eating, gasps, deaths). |

**Readability rule (from the GDD):** the snake and fruit must always be the highest-contrast things on the board. Decorations and effects never cover the head's next 3 cells.

---

## 2. Palette

All colors live in **one script**, `config/palette.gd` (`class_name Palette`, constants only), so the whole game can be recolored in one place.

### 2.1 Board
| Name | Hex | Use |
|------|-----|-----|
| `LAWN_LIGHT` | `#A6DB78` | Checkerboard cell A |
| `LAWN_DARK` | `#93CF66` | Checkerboard cell B (subtle, ≈ 8% darker) |
| `HEDGE` | `#4F9E45` | Wall (deadly border) |
| `HEDGE_DARK` | `#3B7D33` | Hedge bumps, shading |
| `BACKGROUND` | `#FFF4D6` | Screen area outside the board (warm cream) |

### 2.2 Snake
| Name | Hex | Use |
|------|-----|-----|
| `SNAKE_BODY` | `#FFB830` | Main body (golden orange) |
| `SNAKE_BELLY` | `#FFD976` | Highlight stripe along the body |
| `SNAKE_CHEEK` | `#FF8FA3` | Blush on the head |
| `TONGUE` | `#FF5C7A` | Tongue (faint, idle flick) |

### 2.3 Fruit
| Fruit | Main | Detail | Shape |
|-------|------|--------|-------|
| 🍎 Apple | `#E8394B` | Leaf `#5DBB4A`, stem `#7A4A22` | One big circle, stem and leaf on top |
| 🍒 Cherry | `#C2185B` | Stems `#5DBB4A` | Two small circles joined by a V of stems |
| 🍇 **Grapes** *(replaces banana)* | `#8E5BD6` | Leaf `#5DBB4A` | A cluster of 6 small circles in a triangle |

> **Why grapes instead of banana:** yellow fruit is too close to the golden snake. Grapes add a **third distinct color and shape**, which also helps colorblind players. Each fruit has its own shape, not just its own color.

### 2.4 Shared
| Name | Hex | Use |
|------|-----|-----|
| `OUTLINE` | `#6B3A12` | Outline of all game objects; text outline |
| `EYE_WHITE` | `#FFFFFF` | Eyes, highlights |
| `PUPIL` | `#2B1D14` | Pupils, X-eyes, mouths |
| `SWEAT` | `#6EC6FF` | Sweat drop (gasp) |
| `GOLD` | `#FFD23F` | Crown, sparkles, "NEW BEST" |
| `UI_PANEL` | `#FFF4D6` | Panels and buttons (same as background) |
| `UI_TEXT` | `#4A2C0F` | Text on panels |
| `UI_ACCENT` | `#FF6B6B` | Primary button / highlights |
| `DIM` | `#000000` at 45% alpha | Pause overlay |

### 2.5 Contrast checks
- Snake `#FFB830` on lawn `#A6DB78`: different hue *and* a dark outline, so it reads clearly in grayscale too.
- Fruit: every fruit has the dark outline, a white shine and a **unique silhouette** (1 circle / 2 circles / cluster), so it's still identifiable with red-green colorblindness.
- **Test:** take a screenshot, desaturate it to grayscale, and check that the snake, fruit and wall are still easy to tell apart.

---

## 3. Sizes & Proportions

Everything scales from the cell size **`c`** (pixels per cell, computed by `BoardView.layout()`). **Never use fixed pixel sizes.**

| Element | Size |
|---------|------|
| Outline width `o` | `max(2.0, c * 0.08)` |
| Body width | `c * 0.80` (circle radius `c * 0.40`) |
| Head radius `r` | `c * 0.50` (bigger than the body, so it reads as the head) |
| Belly stripe | Polyline `c * 0.25` wide, offset slightly toward the top-left (fake light from the top-left) |
| Fruit (apple) | Radius `c * 0.38` |
| Fruit (cherry) | 2 circles, radius `c * 0.20` |
| Fruit (grapes) | 6 circles, radius `c * 0.13` |
| Hedge thickness | `c * 0.5` outside the grid |
| Shine highlight | White circle, `0.25 ×` the object radius, at the top-left, 70% alpha |

**Light direction:** always from the **top-left**. Shines go top-left. Shading, if any, goes bottom-right.

---

## 4. Drawing technique (outline pass)

Every object is drawn **twice**: first a bigger shape in `OUTLINE`, then the normal shape in its fill color on top.

```gdscript
# Body (in SnakeView._draw):
draw_polyline(points, Palette.OUTLINE, c * 0.80 + o * 2.0, true)
for p in points: draw_circle(p, c * 0.40 + o, Palette.OUTLINE)
draw_polyline(points, Palette.SNAKE_BODY, c * 0.80, true)
for p in points: draw_circle(p, c * 0.40, Palette.SNAKE_BODY)
draw_polyline(belly_points, Palette.SNAKE_BELLY, c * 0.25, true)   # belly stripe on top
# Head last (outline circle, fill circle, then the face)
```
- Turn on **antialiasing** (the last `true` argument of `draw_polyline`; `draw_circle` is smooth with the Compatibility renderer when MSAA 2D is set to 2× under **Project Settings → Rendering → Anti Aliasing → MSAA 2D**).
- Draw order is in Technical Design §7.4: board → effects → fruit → snake.

---

## 5. The Snake Head & Face

The head is a circle of radius `r`. The face is built from simple shapes and **rotates to face `direction`**: positions are in a local frame where "forward" = direction.

### 5.1 Base layout (local coordinates, forward = +x)
| Part | Position | Size |
|------|----------|------|
| Eyes (×2) | `(+0.20r, ±0.38r)` | White circle `0.30r`, outlined |
| Pupils | Eye center + `0.10r` toward forward | Circle `0.15r` |
| Cheeks (×2) | `(-0.05r, ±0.62r)` | `SNAKE_CHEEK` circle `0.16r`, 60% alpha |
| Mouth | `(+0.70r, 0)` | Depends on expression |

> When the head faces **left**, flip the face vertically (not just a rotation), so the snake isn't upside-down. This is a simple scale `(1, -1)` in the local transform when `direction == LEFT`.

### 5.2 Expression sheet (GDD §6.3)
| Expression | Eyes | Mouth | Extra |
|------------|------|-------|-------|
| 😐 **Neutral** | Round, pupils forward | Small smile arc | **Blink** every 2-4 s (random): eyes squash to 10% height for 0.1 s. Optional tongue flick every 5-8 s. |
| 😋 **Happy chomp** | Happy `^` arcs (closed) | Open "D" half-circle, `PUPIL` color, with a `TONGUE` blob | Head squash (from Technical Design §7.4) |
| 😱 **Gasp** | **1.3× bigger**, tiny pupils (`0.08r`) | Small "O" circle | `SWEAT` teardrop at the back of the head; pupils look **toward the danger cell** |
| 🤩 **Proud** | Eyes become 4-point `GOLD` stars | Big smile arc | 3 small sparkles pop around the head |
| 😵 **Faint** | **X** marks (two lines each) | Wavy line | `TONGUE` hanging out to the side; 3 small stars circle above the head |
| 😎 **Cool** | Sunglasses: one rounded `PUPIL` bar over both eyes, with a white shine stripe | Smirk (asymmetric arc) | `GOLD` crown on top (3 points, outlined) |
| 😴 **Sleepy** | Closed `‿` arcs | Tiny "o" | A "z" floats up and fades (a small label or 3 lines) |

### 5.3 Body personality
- **Idle wobble:** body points get a tiny sine offset perpendicular to the path (amplitude `c * 0.04`, a wave running from head to tail). It's alive but never off the grid.
- **Milestone rainbow:** a hue wave travels from head to tail over 0.6 s, overriding `SNAKE_BODY` per segment, then returns to normal.
- **Death bounce:** segments hop `c * 0.15` up one after another, head to tail, 30 ms apart.

---

## 6. Fruit Art (GDD §6.4)

| State | Look |
|-------|------|
| **Idle** | Bounce: scale Y between 1.0 and 1.06, X between 1.0 and 0.97, period 1.2 s. Tiny face: two dot eyes plus a small smile. |
| **Panic** (head ≤ 3 cells) | Eyes become wide circles with tiny pupils looking at the snake. Mouth becomes a wobbly line. Shake: ±`c * 0.05` horizontal at 18 Hz. A small `SWEAT` drop. |
| **Spawn** | Pop in: scale 0 → 1.15 → 1.0 over 0.2 s (`TRANS_BACK`) |

The face sits on the main circle (for cherries: on the bigger front cherry; for grapes: on the center grape).

---

## 7. Board & Arena (GDD §7)

- **Lawn:** checkerboard of `LAWN_LIGHT` / `LAWN_DARK`. **No** other decorations inside the grid. It must stay calm (A4).
- **Hedge (wall):** a rounded rectangle band `c * 0.5` thick around the grid, in `HEDGE` with the outline. Along the outer edge, a row of `HEDGE_DARK` half-circles (radius `c * 0.25`) every `c * 0.5`, so it looks bushy.
- **Outside the board:** flat `BACKGROUND` cream. *Optional later:* a few flowers drawn in the background area only, never on the grid.
- **Death on a wall:** the hedge shakes locally near the impact point (the camera shake already covers most of this).

---

## 8. UI Style

| Element | Style |
|---------|-------|
| **Font** | **Fredoka** (Google Fonts, SIL Open Font License, free for games). Download the font file and put it in `res://assets/fonts/`. Fallback: Godot's default font. |
| Title "Snake" | Fredoka Bold, `GOLD` fill, 6 px `OUTLINE` outline, gentle bob animation |
| HUD | Score top-left, Best top-right; Fredoka SemiBold; white text with a 4 px `OUTLINE` outline (readable on any color). The pause button is a rounded square with two bars. |
| Panels (Pause, Game Over, Settings) | `UI_PANEL` fill, 4 px `OUTLINE` border, corner radius 24, a slight drop shadow (`OUTLINE` at 25%, offset 0, 6) |
| Buttons | Rounded (radius 18). Primary uses `UI_ACCENT` with white text. Pressed state scales to 0.95. Hovered gets a 5% brighter fill. |
| "NEW BEST!" | `GOLD` text, 3 small sparkles, scale-pulse 1.0 ↔ 1.08 |
| Minimum text size | 24 px at the 720 p base resolution (readable on phones) |

Use Godot **Themes**: create one `ui/theme.tres` with the font, `StyleBoxFlat` panels and buttons, and set it on the root `CanvasLayer`, so every screen matches automatically.

---

## 9. VFX Style (GDD §6.5)

All effects use **CPUParticles2D** or tweens (Technical Design §7.4). The shapes are the same round style: circles and stars, never sharp sparks.

| Effect | Look |
|--------|------|
| Chomp burst | 8-12 circles in the **fruit's color** plus 3 white circles, radius `c * 0.08`, spread 360°, 0.35 s, shrink to 0 |
| "+10" popup | Fredoka Bold, white with `OUTLINE` outline, rises 1 cell, fades over 0.6 s |
| Speed lines | 3 short white lines at 60% alpha behind the head, 0.25 s |
| Confetti (milestone, perfect) | Small rotating rectangles in the colors `GOLD`, `UI_ACCENT`, `#8E5BD6`, `#6EC6FF` and `#5DBB4A`, falling with gravity, 1.2 s |
| Death stars | 3 `GOLD` stars circling above the fainted head |
| Pause | `DIM` overlay + sleepy snake |

**Budget:** at most about **60 particles** alive at once. Effects last ≤ 0.6 s on the board (except confetti, which is drawn above the board but falls quickly and is only used when play has stopped or at a milestone).

---

## 10. Asset List

| Asset | Source | Status |
|-------|--------|--------|
| Snake, faces, fruit, lawn, hedge, effects | **Code** (`_draw()` + CPUParticles2D) | To build in the Engine phase |
| `Palette` script | Code (`config/palette.gd`) | To build |
| UI theme | `ui/theme.tres` | To build |
| Font: Fredoka | Google Fonts (OFL) | **You download it** into `assets/fonts/` |
| App icon | Replace `icon.svg` with the snake's happy face (export a frame or draw it in Inkscape) | Later (polish) |

---

## 11. Quick checklist for the Engine phase
- [ ] All colors come from `Palette`. No hex values in other scripts.
- [ ] All sizes are `c`-relative. Resizing the window keeps everything proportional.
- [ ] The grayscale screenshot test passes.
- [ ] Snake faces turn correctly in all 4 directions (left isn't upside-down).
- [ ] Effects never cover the head's next 3 cells.

---
*Art Bible by Aurora, GD-KS Design Team. Next: Audio bible (Anton).*
