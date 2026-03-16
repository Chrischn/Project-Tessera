# =============================================================================
# Script Name:        AssetCycler.gd
# Author(s):          Chrischn89
# Godot Version:      4.5
# Description:
#	Testing utility: cycle through all known NIFs in the 3D world.
#	Press J to go to the previous asset, L to go to the next asset.
#	Only one asset is visible at a time; each replacement appears at the
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

const START_ASSET := "axeman.nif"

var _asset_paths: Array[String] = []
var _current_index: int = 0
var _asset_container: Node3D = null
var _spawn_position: Vector3 = Vector3.ZERO
var _base_path: String

# Debug visualization cycling (press I)
# Modes: 0=lines only, 1=spheres only, 2=labels only, 3=all debug,
#         4=hide mesh (debug only), 5=show mesh (hide debug), 6=everything visible
var _debug_mode: int = 5  # start with mesh only (debug hidden)
var _debug_mode_names: Array[String] = [
	"Lines only", "Spheres only", "Labels only", "All debug overlays",
	"Debug only (mesh hidden)", "Mesh only (debug hidden)", "Everything visible",
	"Grid",
]
var _debug_hud_label: Label = null
var _asset_name_label: Label = null
var _asset_counter_label: Label = null
var _keybinds_label: Label = null
var _grid_node: MeshInstance3D = null

# Animation playback (press P)
var _anim_player: AnimationPlayer = null
var _anim_clip_names: Array[StringName] = []
var _anim_clip_index: int = 0
var _anim_hud_label: Label = null

# Material override diagnostic (press T)
var _material_override_active := false


func setup(base_path: String) -> void:
	_setup_debug_hud()
	_base_path = base_path
	_asset_paths = VFS.find_files("art", "nif")
	print("AssetCycler: %d NIFs found" % _asset_paths.size())
	if _asset_paths.is_empty():
		push_error("AssetCycler: no NIFs found in VFS")
		return
	for i in _asset_paths.size():
		if _asset_paths[i].get_file().to_lower() == START_ASSET.to_lower():
			_current_index = i
			break
	_create_grid_overlay()
	_load_asset(_current_index)


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
		KEY_T:
			_toggle_material_override()
		KEY_P:
			_cycle_animation()


func _cycle_animation() -> void:
	if not _anim_player or _anim_clip_names.is_empty():
		print("AssetCycler: no AnimationPlayer / clips on this asset")
		return
	_anim_clip_index = (_anim_clip_index + 1) % _anim_clip_names.size()
	var clip: String = _anim_clip_names[_anim_clip_index]
	if clip == "(none)":
		_anim_player.stop()
		# Reset bone poses to rest so we see the pure rest pose
		var skel := _find_skeleton(_asset_container)
		if skel:
			skel.reset_bone_poses()
		_update_anim_hud()
		print("AssetCycler [ANIM]: stopped — showing REST POSE")
		return
	_anim_player.play(clip)
	_update_anim_hud()
	print("AssetCycler [ANIM]: playing '%s'" % clip)
	_print_clip_diagnostics(clip)


func _print_clip_diagnostics(clip_name: String) -> void:
	var skel := _find_skeleton(_asset_container)
	if not skel or not _anim_player:
		return
	var lib := _anim_player.get_animation_library("")
	if not lib:
		return
	var anim := lib.get_animation(clip_name)
	if not anim:
		return

	print("--- [DIAG] Clip: '%s'  tracks=%d  length=%.3f ---" \
		% [clip_name, anim.get_track_count(), anim.length])

	for t_idx in anim.get_track_count():
		var path := anim.track_get_path(t_idx)
		var type := anim.track_get_type(t_idx)
		# Extract bone name from path (last component after ':')
		var path_str := str(path)
		var bone_name := path_str.get_slice(":", path_str.get_slice_count(":") - 1)
		var bone_idx := skel.find_bone(bone_name)
		var key_count := anim.track_get_key_count(t_idx)

		if type == Animation.TYPE_ROTATION_3D and key_count > 0:
			var first_rot: Quaternion = anim.track_get_key_value(t_idx, 0)
			var rest_rot := skel.get_bone_rest(bone_idx).basis.get_rotation_quaternion() \
				if bone_idx >= 0 else Quaternion.IDENTITY
			var angle_diff := rad_to_deg(first_rot.angle_to(rest_rot))
			print("  ROT  %-25s  first=(%.3f, %.3f, %.3f, %.3f)  rest=(%.3f, %.3f, %.3f, %.3f)  diff=%.1fdeg" \
				% [bone_name,
				   first_rot.x, first_rot.y, first_rot.z, first_rot.w,
				   rest_rot.x, rest_rot.y, rest_rot.z, rest_rot.w,
				   angle_diff])
		elif type == Animation.TYPE_POSITION_3D and key_count > 0:
			var first_pos: Vector3 = anim.track_get_key_value(t_idx, 0)
			var rest_pos := skel.get_bone_rest(bone_idx).origin \
				if bone_idx >= 0 else Vector3.ZERO
			var dist := first_pos.distance_to(rest_pos)
			print("  POS  %-25s  first=(%.3f, %.3f, %.3f)  rest=(%.3f, %.3f, %.3f)  dist=%.3f" \
				% [bone_name,
				   first_pos.x, first_pos.y, first_pos.z,
				   rest_pos.x, rest_pos.y, rest_pos.z,
				   dist])
	print("--- [DIAG] end ---")


func _find_skeleton(node: Node) -> Skeleton3D:
	if node is Skeleton3D:
		return node
	for child in node.get_children():
		var found := _find_skeleton(child)
		if found:
			return found
	return null


func _cycle(direction: int) -> void:
	if _asset_paths.is_empty():
		return
	_current_index = (_current_index + direction + _asset_paths.size()) % _asset_paths.size()
	if _asset_container:
		_spawn_position = _asset_container.position
		_asset_container.free()
		_asset_container = null
	_load_asset(_current_index)


func _load_asset(index: int) -> void:
	var nif_path: String = _asset_paths[index]
	var disk_path: String = VFS.get_file_as_disk_path(nif_path)
	if disk_path == "":
		push_error("AssetCycler: NIF not found in VFS: %s" % nif_path)
		return

	var container := Node3D.new()
	container.name = "AssetContainer"
	container.position = _spawn_position
	get_parent().add_child(container)
	_asset_container = container

	# Extract .kfm and .kf files to disk so build_animations() can find them
	# alongside the .nif (they may be packed inside FPK archives)
	var asset_dir := nif_path.get_base_dir()
	for kfm_path in VFS.find_files(asset_dir, "kfm"):
		VFS.get_file_as_disk_path(kfm_path)
	for kf_path in VFS.find_files(asset_dir, "kf"):
		VFS.get_file_as_disk_path(kf_path)

	var niflib := GdextNiflib.new()
	# TODO: drive team_color from game state (player civilization color).
	# Blue used here as a visible test value; white = no tinting.
	niflib.team_color = Color(0.2, 0.4, 1.0)
	niflib.load_nif_scene(disk_path, container, _base_path)
	print("AssetCycler [%d/%d]: %s" % [index + 1, _asset_paths.size(), nif_path])

	_anim_player = null
	_anim_clip_names = []
	_anim_clip_index = 0
	_anim_player = _find_animation_player(container)
	if _anim_player:
		var lib := _anim_player.get_animation_library("")
		if lib:
			_anim_clip_names = Array(lib.get_animation_list())
			_anim_clip_names.sort()
		# Insert "(none)" at index 0 — shows pure rest pose (no animation)
		_anim_clip_names.insert(0, &"(none)")
		if _anim_clip_names.size() > 1:
			# Auto-start with "(none)" = rest pose for diagnostic comparison
			_anim_clip_index = 0
			print("AssetCycler [ANIM]: showing REST POSE (%d clips + none)" \
				% [_anim_clip_names.size() - 1])
		else:
			print("AssetCycler [ANIM]: AnimationPlayer present but no clips")
	else:
		print("AssetCycler [ANIM]: no AnimationPlayer (static model)")
	_update_anim_hud()
	_apply_debug_mode()
	if _material_override_active:
		_apply_override_recursive(_asset_container, _make_diagnostic_mat())
	_update_asset_hud()


# --- Debug visualization toggle (press I) ---

func _cycle_debug_mode() -> void:
	_debug_mode = (_debug_mode + 1) % _debug_mode_names.size()
	print("Debug mode: %s" % _debug_mode_names[_debug_mode])
	_apply_debug_mode()
	_update_hud()


func _apply_debug_mode() -> void:
	if not _asset_container:
		return

	# Find all _BoneDebug nodes and mesh nodes in the scene tree
	var debug_nodes := _find_nodes_by_name(_asset_container, "_BoneDebug")
	var mesh_nodes := _find_mesh_instances(_asset_container)

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
			5, 7:  # Mesh only (debug hidden) / Grid
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

	# Show grid only in mode 7
	if _grid_node:
		_grid_node.visible = (_debug_mode == 7)


func _find_animation_player(node: Node) -> AnimationPlayer:
	if node is AnimationPlayer:
		return node
	for child in node.get_children():
		var found := _find_animation_player(child)
		if found:
			return found
	return null


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


# --- Material override diagnostic (press T) ---
# Replaces every mesh material with a flat red opaque StandardMaterial3D
# (cull_disabled, unshaded) to isolate material from geometry.
# If the model still shows see-through holes under the override → geometry cause.
# If the model becomes fully solid → shader/material cause.

func _toggle_material_override() -> void:
	_material_override_active = !_material_override_active
	var override_mat: StandardMaterial3D = null
	if _material_override_active:
		override_mat = _make_diagnostic_mat()
	if _asset_container:
		_apply_override_recursive(_asset_container, override_mat)
	print("[DIAG] Material override: ", "ON (flat red, cull_disabled)" if _material_override_active else "OFF (restored)")


func _make_diagnostic_mat() -> StandardMaterial3D:
	var mat := StandardMaterial3D.new()
	mat.albedo_color = Color(1.0, 0.0, 0.0, 1.0)
	mat.shading_mode = BaseMaterial3D.SHADING_MODE_UNSHADED
	mat.cull_mode = BaseMaterial3D.CULL_DISABLED
	mat.transparency = BaseMaterial3D.TRANSPARENCY_DISABLED
	return mat


func _apply_override_recursive(node: Node, mat: Material) -> void:
	if node is MeshInstance3D:
		node.material_override = mat
	for child in node.get_children():
		_apply_override_recursive(child, mat)


# --- Grid overlay ---

const GRID_HALF_EXTENT := 1000  # metres; 2km x 2km total, 1m cells

func _create_grid_overlay() -> void:
	var verts := PackedVector3Array()
	var e := float(GRID_HALF_EXTENT)
	for i in range(-GRID_HALF_EXTENT, GRID_HALF_EXTENT + 1):
		var f := float(i)
		verts.append(Vector3(-e, 0.0,  f)); verts.append(Vector3( e, 0.0,  f))  # X-parallel lines
		verts.append(Vector3( f, 0.0, -e)); verts.append(Vector3( f, 0.0,  e))  # Z-parallel lines
	var arrays: Array = []
	arrays.resize(Mesh.ARRAY_MAX)
	arrays[Mesh.ARRAY_VERTEX] = verts
	var arr_mesh := ArrayMesh.new()
	arr_mesh.add_surface_from_arrays(Mesh.PRIMITIVE_LINES, arrays)
	var mat := StandardMaterial3D.new()
	mat.albedo_color = Color(0.5, 0.5, 0.5)
	mat.shading_mode = BaseMaterial3D.SHADING_MODE_UNSHADED
	arr_mesh.surface_set_material(0, mat)
	_grid_node = MeshInstance3D.new()
	_grid_node.name = "GridOverlay"
	_grid_node.mesh = arr_mesh
	_grid_node.visible = false
	get_parent().add_child(_grid_node)


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

	# Bottom-right: current animation clip
	_anim_hud_label = Label.new()
	_anim_hud_label.add_theme_font_size_override("font_size", 18)
	_anim_hud_label.add_theme_color_override("font_color", Color.WHITE)
	_anim_hud_label.add_theme_color_override("font_shadow_color", Color.BLACK)
	_anim_hud_label.add_theme_constant_override("shadow_offset_x", 1)
	_anim_hud_label.add_theme_constant_override("shadow_offset_y", 1)
	_anim_hud_label.anchor_left   = 1.0
	_anim_hud_label.anchor_right  = 1.0
	_anim_hud_label.anchor_top    = 1.0
	_anim_hud_label.anchor_bottom = 1.0
	_anim_hud_label.grow_horizontal = Control.GROW_DIRECTION_BEGIN
	_anim_hud_label.grow_vertical   = Control.GROW_DIRECTION_BEGIN
	_anim_hud_label.offset_right  = -10
	_anim_hud_label.offset_bottom = -10
	_anim_hud_label.horizontal_alignment = HORIZONTAL_ALIGNMENT_RIGHT
	canvas.add_child(_anim_hud_label)

	# Left side: keybindings legend
	_keybinds_label = Label.new()
	_keybinds_label.text = \
		"J / L    Prev / Next asset\n" + \
		"P        Next animation clip\n" + \
		"I         Cycle debug mode\n" + \
		"T        Material override\n" + \
		"WASD  Move   Shift  Sprint\n" + \
		"Mouse  Look\n" + \
		"Space / Ctrl  Up / Down\n" + \
		"Esc     Show cursor"
	_keybinds_label.add_theme_font_size_override("font_size", 14)
	_keybinds_label.add_theme_color_override("font_color", Color(1.0, 1.0, 1.0, 0.6))
	_keybinds_label.add_theme_color_override("font_shadow_color", Color.BLACK)
	_keybinds_label.add_theme_constant_override("shadow_offset_x", 1)
	_keybinds_label.add_theme_constant_override("shadow_offset_y", 1)
	_keybinds_label.anchor_left = 0.0
	_keybinds_label.anchor_right = 0.0
	_keybinds_label.anchor_top = 0.5
	_keybinds_label.anchor_bottom = 0.5
	_keybinds_label.grow_horizontal = Control.GROW_DIRECTION_END
	_keybinds_label.grow_vertical = Control.GROW_DIRECTION_BOTH
	_keybinds_label.offset_left = 10
	canvas.add_child(_keybinds_label)

	# Top-center: current asset filename
	_asset_name_label = Label.new()
	_asset_name_label.add_theme_font_size_override("font_size", 20)
	_asset_name_label.add_theme_color_override("font_color", Color.WHITE)
	_asset_name_label.add_theme_color_override("font_shadow_color", Color.BLACK)
	_asset_name_label.add_theme_constant_override("shadow_offset_x", 1)
	_asset_name_label.add_theme_constant_override("shadow_offset_y", 1)
	_asset_name_label.anchor_left = 0.5
	_asset_name_label.anchor_right = 0.5
	_asset_name_label.anchor_top = 0.0
	_asset_name_label.anchor_bottom = 0.0
	_asset_name_label.grow_horizontal = Control.GROW_DIRECTION_BOTH
	_asset_name_label.offset_top = 10
	_asset_name_label.horizontal_alignment = HORIZONTAL_ALIGNMENT_CENTER
	canvas.add_child(_asset_name_label)

	# Top-right: counter (current / total)
	_asset_counter_label = Label.new()
	_asset_counter_label.add_theme_font_size_override("font_size", 20)
	_asset_counter_label.add_theme_color_override("font_color", Color.WHITE)
	_asset_counter_label.add_theme_color_override("font_shadow_color", Color.BLACK)
	_asset_counter_label.add_theme_constant_override("shadow_offset_x", 1)
	_asset_counter_label.add_theme_constant_override("shadow_offset_y", 1)
	_asset_counter_label.anchor_left = 1.0
	_asset_counter_label.anchor_right = 1.0
	_asset_counter_label.anchor_top = 0.0
	_asset_counter_label.anchor_bottom = 0.0
	_asset_counter_label.grow_horizontal = Control.GROW_DIRECTION_BEGIN
	_asset_counter_label.offset_top = 10
	_asset_counter_label.offset_right = -10
	_asset_counter_label.horizontal_alignment = HORIZONTAL_ALIGNMENT_RIGHT
	canvas.add_child(_asset_counter_label)


func _update_asset_hud() -> void:
	if _asset_name_label:
		_asset_name_label.text = _asset_paths[_current_index].get_file()
	if _asset_counter_label:
		_asset_counter_label.text = "%d / %d" % [_current_index + 1, _asset_paths.size()]


func _update_anim_hud() -> void:
	if not _anim_hud_label:
		return
	if _anim_clip_names.is_empty():
		_anim_hud_label.text = "[P] No animations"
	else:
		_anim_hud_label.text = "[P] %s  (%d/%d)" \
			% [_anim_clip_names[_anim_clip_index],
			   _anim_clip_index + 1, _anim_clip_names.size()]


func _update_hud() -> void:
	if _debug_hud_label:
		_debug_hud_label.text = "Debug: %s" % _debug_mode_names[_debug_mode]
