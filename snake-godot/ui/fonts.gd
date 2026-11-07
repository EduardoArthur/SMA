class_name Fonts
extends RefCounted
## Loads the Fredoka font (Art Bible §8) if you have added it to
## res://assets/fonts/, and makes it the default font for every label and
## button. Without the file, Godot's built-in font is used, so nothing breaks.
##
## Accepted file names (the first one found wins):
##   Fredoka-VariableFont_wdth,wght.ttf   (the variable font from Google Fonts)
##   Fredoka.ttf                          (any Fredoka file, renamed)
##   Fredoka-Regular.ttf + Fredoka-SemiBold.ttf  (the static files)

const DIR := "res://assets/fonts/"
const REGULAR_FILES := ["Fredoka-VariableFont_wdth,wght.ttf", "Fredoka.ttf", "Fredoka-Regular.ttf"]
const BOLD_FILE := "Fredoka-SemiBold.ttf"

static var regular: Font ## null when no Fredoka file was found.
static var bold: Font ## For headlines, buttons and scores; null without Fredoka.
static var _done := false


static func setup() -> void:
	if _done:
		return
	_done = true
	for file: String in REGULAR_FILES:
		if ResourceLoader.exists(DIR + file):
			regular = load(DIR + file) as Font
			break
	if regular == null:
		return
	ThemeDB.fallback_font = regular # Every Control without its own font uses this.
	if ResourceLoader.exists(DIR + BOLD_FILE):
		bold = load(DIR + BOLD_FILE) as Font
	else:
		# Variable font: ask for a heavier weight (no effect on a static file).
		var heavy := FontVariation.new()
		heavy.base_font = regular
		heavy.variation_opentype = {"wght": 600}
		bold = heavy


## Applies the bold font to a control, if Fredoka is available.
static func make_bold(control: Control) -> void:
	if bold != null:
		control.add_theme_font_override("font", bold)
