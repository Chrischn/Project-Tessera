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


func setup(base_path: String) -> void:
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
	niflib.load_nif_scene(disk_path, container, _base_path)
	print("UnitCycler [%d/%d]: %s" % [index + 1, _unit_paths.size(), nif_path])
