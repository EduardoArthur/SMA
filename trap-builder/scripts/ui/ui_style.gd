class_name UiStyle
## Estilos de UI "catálogo de papel" (art-bible.md §3.4 e §8), criados em código
## até existir um Theme de verdade (S21).

const PAPER := Color("f2e3c2")
const PAPER_DARK := Color("e2cf9f")
const INK := Color("2b1b17")
const COIN := Color("e8b33a")
const DANGER := Color("d63a2f")
const STAR_ON := Color("ffd84d")
const STAR_OFF := Color("8a7a5c")


static func panel(bg: Color = PAPER) -> StyleBoxFlat:
	var sb := StyleBoxFlat.new()
	sb.bg_color = bg
	sb.border_color = INK
	sb.set_border_width_all(3)
	sb.set_corner_radius_all(10)
	sb.set_content_margin_all(12)
	return sb


static func style_panel(node: Control, bg: Color = PAPER) -> void:
	node.add_theme_stylebox_override("panel", panel(bg))


static func style_button(button: Button, bg: Color = PAPER) -> void:
	# Sem foco de teclado: Espaço/Enter ficam livres para os atalhos do jogo.
	button.focus_mode = Control.FOCUS_NONE
	var normal := panel(bg)
	normal.set_content_margin_all(8)
	var hover := normal.duplicate() as StyleBoxFlat
	hover.bg_color = bg.lightened(0.15)
	var pressed := normal.duplicate() as StyleBoxFlat
	pressed.bg_color = COIN
	var disabled := normal.duplicate() as StyleBoxFlat
	disabled.bg_color = bg.darkened(0.25)
	disabled.border_color = INK.lightened(0.5)
	button.add_theme_stylebox_override("normal", normal)
	button.add_theme_stylebox_override("hover", hover)
	button.add_theme_stylebox_override("pressed", pressed)
	button.add_theme_stylebox_override("hover_pressed", pressed)
	button.add_theme_stylebox_override("disabled", disabled)
	button.add_theme_stylebox_override("focus", StyleBoxEmpty.new())
	for color_name in ["font_color", "font_hover_color", "font_pressed_color", "font_hover_pressed_color", "font_focus_color"]:
		button.add_theme_color_override(color_name, INK)
	button.add_theme_color_override("font_disabled_color", INK.lightened(0.45))


static func label(text: String, size: int = 16, color: Color = INK) -> Label:
	var l := Label.new()
	l.text = text
	l.add_theme_font_size_override("font_size", size)
	l.add_theme_color_override("font_color", color)
	return l
