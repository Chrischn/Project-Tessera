# =============================================================================
# Script Name:        UnitCycler.gd
# Author(s):          Chrischn89
# Godot Version:      4.5
# Description:
#	Testing utility: cycle through all known unit NIFs in the 3D world.
#	Press J to go to the previous unit, L to go to the next unit.
#	Only one unit is visible at a time; each replacement appears at the
#	same world position as its predecessor.
#
# Usage:
#	Instantiated and added as a child of GameWorld by GameWorldScene.gd.
#	Call setup(base_path) after add_child().
#
# License:
#	Released under the terms of the GNU General Public License version 3.0
#
# =============================================================================

extends Node

var _unit_paths: Array[String] = []
var _current_index: int = 0
var _unit_container: Node3D = null
var _spawn_position: Vector3 = Vector3.ZERO
var _base_path: String

# Debug visualization cycling (press I)
# Modes: 0=lines only, 1=spheres only, 2=labels only, 3=all debug,
#         4=hide mesh (debug only), 5=show mesh (hide debug), 6=everything visible
var _debug_mode: int = 6  # start with everything visible
var _debug_mode_names: Array[String] = [
	"Lines only", "Spheres only", "Labels only", "All debug overlays",
	"Debug only (mesh hidden)", "Mesh only (debug hidden)", "Everything visible"
]
var _debug_hud_label: Label = null
var _unit_name_label: Label = null
var _unit_counter_label: Label = null


func setup(base_path: String) -> void:
	_setup_debug_hud()
	_base_path = base_path
	_unit_paths = VFS.find_files("art/units", "nif")
	print("UnitCycler: %d unit NIFs found" % _unit_paths.size())
	if _unit_paths.is_empty():
		push_error("UnitCycler: no unit NIFs found in VFS")
		return
	_load_unit(_current_index)


func _unhandled_input(event: InputEvent) -> void:
	if not (event is InputEventKey and event.pressed and not event.echo):
		return
	match event.keycode:
		KEY_J:
			_cycle(-1)
		KEY_L:
			_cycle(1)
		KEY_I:
			_cycle_debug_mode()


func _cycle(direction: int) -> void:
	if _unit_paths.is_empty():
		return
	_current_index = (_current_index + direction + _unit_paths.size()) % _unit_paths.size()
	if _unit_container:
		_spawn_position = _unit_container.position
		_unit_container.queue_free()
		_unit_container = null
	_load_unit(_current_index)


func _load_unit(index: int) -> void:
	var nif_path: String = _unit_paths[index]
	var disk_path: String = VFS.get_file_as_disk_path(nif_path)
	if disk_path == "":
		push_error("UnitCycler: NIF not found in VFS: %s" % nif_path)
		return

	var container := Node3D.new()
	container.name = "UnitContainer"
	container.position = _spawn_position
	get_parent().add_child(container)
	_unit_container = container

	var niflib := GdextNiflib.new()
	# TODO: drive team_color from game state (player civilization color).
	# Blue used here as a visible test value; white = no tinting.
	niflib.team_color = Color(0.2, 0.4, 1.0)
	niflib.load_nif_scene(disk_path, container, _base_path)
	print("UnitCycler [%d/%d]: %s" % [index + 1, _unit_paths.size(), nif_path])
	_apply_debug_mode()
	_update_unit_hud()


# --- Debug visualization toggle (press I) ---

func _cycle_debug_mode() -> void:
	_debug_mode = (_debug_mode + 1) % _debug_mode_names.size()
	print("Debug mode: %s" % _debug_mode_names[_debug_mode])
	_apply_debug_mode()
	_update_hud()


func _apply_debug_mode() -> void:
	if not _unit_container:
		return

	# Find all _BoneDebug nodes and mesh nodes in the scene tree
	var debug_nodes := _find_nodes_by_name(_unit_container, "_BoneDebug")
	var mesh_nodes := _find_mesh_instances(_unit_container)

	for dbg in debug_nodes:
		var lines_node := dbg.get_node_or_null("_Lines")
		var spheres_node := dbg.get_node_or_null("_Spheres")
		var labels_node := dbg.get_node_or_null("_Labels")

		match _debug_mode:
			0:  # Lines only
				if lines_node: lines_node.visible = true
				if spheres_node: spheres_node.visible = false
				if labels_node: labels_node.visible = false
				dbg.visible = true
			1:  # Spheres only
				if lines_node: lines_node.visible = false
				if spheres_node: spheres_node.visible = true
				if labels_node: labels_node.visible = false
				dbg.visible = true
			2:  # Labels only
				if lines_node: lines_node.visible = false
				if spheres_node: spheres_node.visible = false
				if labels_node: labels_node.visible = true
				dbg.visible = true
			3:  # All debug overlays
				if lines_node: lines_node.visible = true
				if spheres_node: spheres_node.visible = true
				if labels_node: labels_node.visible = true
				dbg.visible = true
			4:  # Debug only (mesh hidden)
				if lines_node: lines_node.visible = true
				if spheres_node: spheres_node.visible = true
				if labels_node: labels_node.visible = true
				dbg.visible = true
			5:  # Mesh only (debug hidden)
				dbg.visible = false
			6:  # Everything visible
				if lines_node: lines_node.visible = true
				if spheres_node: spheres_node.visible = true
				if labels_node: labels_node.visible = false
				dbg.visible = true

	# Show/hide mesh nodes
	for mi in mesh_nodes:
		match _debug_mode:
			4:  # Debug only (mesh hidden)
				mi.visible = false
			_:
				mi.visible = true


func _find_nodes_by_name(root: Node, target_name: String) -> Array[Node]:
	var result: Array[Node] = []
	if root.name == target_name:
		result.append(root)
	for child in root.get_children():
		result.append_array(_find_nodes_by_name(child, target_name))
	return result


func _find_mesh_instances(root: Node) -> Array[Node]:
	var result: Array[Node] = []
	# Only collect MeshInstance3D nodes that are NOT inside _BoneDebug
	if root is MeshInstance3D and not _is_inside_bone_debug(root):
		result.append(root)
	for child in root.get_children():
		if child.name == "_BoneDebug":
			continue  # skip debug sub-tree
		result.append_array(_find_mesh_instances(child))
	return result


func _is_inside_bone_debug(node: Node) -> bool:
	var current := node.get_parent()
	while current:
		if current.name == "_BoneDebug":
			return true
		current = current.get_parent()
	return false


# --- Debug HUD ---

func _setup_debug_hud() -> void:
	var canvas := CanvasLayer.new()
	canvas.name = "DebugHUD"
	canvas.layer = 100
	add_child(canvas)

	_debug_hud_label = Label.new()
	_debug_hud_label.text = "Debug: %s" % _debug_mode_names[_debug_mode]
	_debug_hud_label.add_theme_font_size_override("font_size", 18)
	_debug_hud_label.add_theme_color_override("font_color", Color.WHITE)
	_debug_hud_label.add_theme_color_override("font_shadow_color", Color.BLACK)
	_debug_hud_label.add_theme_constant_override("shadow_offset_x", 1)
	_debug_hud_label.add_theme_constant_override("shadow_offset_y", 1)
	_debug_hud_label.anchor_left = 0.0
	_debug_hud_label.anchor_top = 1.0
	_debug_hud_label.anchor_right = 0.0
	_debug_hud_label.anchor_bottom = 1.0
	_debug_hud_label.grow_horizontal = Control.GROW_DIRECTION_END
	_debug_hud_label.grow_vertical = Control.GROW_DIRECTION_BEGIN
	_debug_hud_label.offset_left = 10
	_debug_hud_label.offset_bottom = -10
	canvas.add_child(_debug_hud_label)

	# Top-center: current unit filename
	_unit_name_label = Label.new()
	_unit_name_label.add_theme_font_size_override("font_size", 20)
	_unit_name_label.add_theme_color_override("font_color", Color.WHITE)
	_unit_name_label.add_theme_color_override("font_shadow_color", Color.BLACK)
	_unit_name_label.add_theme_constant_override("shadow_offset_x", 1)
	_unit_name_label.add_theme_constant_override("shadow_offset_y", 1)
	_unit_name_label.anchor_left = 0.5
	_unit_name_label.anchor_right = 0.5
	_unit_name_label.anchor_top = 0.0
	_unit_name_label.anchor_bottom = 0.0
	_unit_name_label.grow_horizontal = Control.GROW_DIRECTION_BOTH
	_unit_name_label.offset_top = 10
	_unit_name_label.horizontal_alignment = HORIZONTAL_ALIGNMENT_CENTER
	canvas.add_child(_unit_name_label)

	# Top-right: counter (current / total)
	_unit_counter_label = Label.new()
	_unit_counter_label.add_theme_font_size_override("font_size", 20)
	_unit_counter_label.add_theme_color_override("font_color", Color.WHITE)
	_unit_counter_label.add_theme_color_override("font_shadow_color", Color.BLACK)
	_unit_counter_label.add_theme_constant_override("shadow_offset_x", 1)
	_unit_counter_label.add_theme_constant_override("shadow_offset_y", 1)
	_unit_counter_label.anchor_left = 1.0
	_unit_counter_label.anchor_right = 1.0
	_unit_counter_label.anchor_top = 0.0
	_unit_counter_label.anchor_bottom = 0.0
	_unit_counter_label.grow_horizontal = Control.GROW_DIRECTION_BEGIN
	_unit_counter_label.offset_top = 10
	_unit_counter_label.offset_right = -10
	_unit_counter_label.horizontal_alignment = HORIZONTAL_ALIGNMENT_RIGHT
	canvas.add_child(_unit_counter_label)


func _update_unit_hud() -> void:
	if _unit_name_label:
		_unit_name_label.text = _unit_paths[_current_index].get_file()
	if _unit_counter_label:
		_unit_counter_label.text = "%d / %d" % [_current_index + 1, _unit_paths.size()]


func _update_hud() -> void:
	if _debug_hud_label:
		_debug_hud_label.text = "Debug: %s" % _debug_mode_names[_debug_mode]
