# =============================================================================
# Script Name:        AxisIndicator.gd
# Author(s):          Chrischn89
# Godot Version:      4.5
# Description:
#   Draws a colour-coded XYZ axis indicator in the bottom-right corner.
#   Always reflects the current camera orientation.
#
# Usage:
#   Attach to a Control node inside a CanvasLayer under Camera3D.
#   K: toggle visibility
#
# License:
#   Released under the terms of the GNU General Public License version 3.0
# =============================================================================

extends Control

@export var camera: Camera3D
@export var axis_length: float  = 75.0
@export var corner_margin: float = 110.0
@export var line_width: float   = 3.0
@export var font_size: int      = 16

const _AXES := [
	{"dir": Vector3(1, 0, 0), "color": Color(1.0,  0.1,  0.1 ), "label": "X"},
	{"dir": Vector3(0, 1, 0), "color": Color(0.1,  1.0,  0.1 ), "label": "Y"},
	{"dir": Vector3(0, 0, 1), "color": Color(0.15, 0.45, 1.0 ), "label": "Z"},
]

var _font: Font

func _ready() -> void:
	if camera == null:
		camera = get_parent().get_parent() as Camera3D
	_font = ThemeDB.fallback_font
	set_anchors_and_offsets_preset(Control.PRESET_FULL_RECT)
	mouse_filter = Control.MOUSE_FILTER_IGNORE

func _unhandled_input(event: InputEvent) -> void:
	if event is InputEventKey and event.pressed and not event.echo:
		if event.keycode == KEY_K:
			visible = !visible

func _process(_delta: float) -> void:
	queue_redraw()

func _draw() -> void:
	if camera == null:
		return

	var center := Vector2(size.x - corner_margin, size.y - corner_margin)
	var inv_basis := camera.global_transform.basis.inverse()

	# Sort back-to-front so occluded axes render first (appear behind)
	var sorted: Array = []
	for axis in _AXES:
		var cam_dir: Vector3 = inv_basis * axis["dir"]
		sorted.append({"axis": axis, "cam_dir": cam_dir})
	sorted.sort_custom(func(a, b): return a["cam_dir"].z > b["cam_dir"].z)

	for entry in sorted:
		var cam_dir: Vector3 = entry["cam_dir"]
		var axis: Dictionary  = entry["axis"]
		# Project to 2D: screen-right = cam +X, screen-up = cam +Y (flip Y)
		var screen_dir := Vector2(cam_dir.x, -cam_dir.y)
		var tip := center + screen_dir * axis_length
		# Fade axes that recede into the screen
		var alpha: float = remap(cam_dir.z, -1.0, 1.0, 1.0, 0.5)
		var col: Color = axis["color"]
		col.a = alpha
		draw_line(center, tip, col, line_width, true)
		draw_string(_font, tip + Vector2(4, -6), axis["label"],
				HORIZONTAL_ALIGNMENT_LEFT, -1, font_size, col)
