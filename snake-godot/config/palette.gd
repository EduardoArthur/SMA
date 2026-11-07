class_name Palette
extends RefCounted
## Every color in the game (Art Bible §2). No hex values anywhere else.

# Board
const LAWN_LIGHT := Color("#A6DB78")
const LAWN_DARK := Color("#93CF66")
const HEDGE := Color("#4F9E45")
const HEDGE_DARK := Color("#3B7D33")
const BACKGROUND := Color("#FFF4D6")

# Snake
const SNAKE_BODY := Color("#FFB830")
const SNAKE_BELLY := Color("#FFD976")
const SNAKE_CHEEK := Color("#FF8FA3")
const TONGUE := Color("#FF5C7A")

# Fruit (index = fruit_type: 0 apple, 1 cherry, 2 grapes)
const FRUIT := [Color("#E8394B"), Color("#C2185B"), Color("#8E5BD6")]
const LEAF := Color("#5DBB4A")
const STEM := Color("#7A4A22")

# Shared
const OUTLINE := Color("#6B3A12")
const EYE_WHITE := Color("#FFFFFF")
const PUPIL := Color("#2B1D14")
const SWEAT := Color("#6EC6FF")
const GOLD := Color("#FFD23F")
const UI_PANEL := Color("#FFF4D6")
const UI_TEXT := Color("#4A2C0F")
const UI_ACCENT := Color("#FF6B6B")
const DIM := Color(0, 0, 0, 0.45)
