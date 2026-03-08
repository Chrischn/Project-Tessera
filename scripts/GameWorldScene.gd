# =============================================================================
# Script Name:        GameWorldScene.gd
# Author(s):          Chrischn89
# Godot Version:      4.5
# Description:        
#	GameWorldScene.gd controls the game's main 3D world
#
# TODO:              
#     -
#
#
# Notes:              
#     -
# 
# License:
#	Released under the terms of the GNU General Public License version 3.0
#
# =============================================================================

extends Node3D

var base_path : String = Global.load_value_from_config("GAME","original_folder_path")	# Load the original folder path from the config

func _ready() -> void:
	print("GameWorld reached!")
	
	# NIF to load — use relative asset path (VFS resolves loose files + FPK archives)
	var nif_asset_path := "Art/Units/axeman/axeman.nif"
	#var nif_asset_path := "Art/Units/MachineGun/machinegunner.nif"
	#var nif_asset_path := "Art/Units/jetfighter/jetfighter.nif"
	#var nif_asset_path := "Art/Terrain/Routes/Roads/roada00.nif"
	#var nif_asset_path := "Art/Structures/Buildings/Forge/forge.nif"
	#var nif_asset_path := "Art/Structures/Buildings/Castle/castle.nif"
	#var nif_asset_path := "Art/Units/Galley/galley_freeze1000.nif"
	#var nif_asset_path := "Art/Units/Submarine/Submarine.nif"

	# Resolve through VFS (handles both loose files and FPK archives)
	var path_to_niffile := VFS.get_file_as_disk_path(nif_asset_path)
	if path_to_niffile == "":
		push_error("NIF not found in VFS: %s" % nif_asset_path)
		return
	print("Loading NIF: ", path_to_niffile)

	var niflib = GdextNiflib.new()
	var full_header = niflib.get_nif_header(path_to_niffile)
	if full_header.success:
		print("Full Header: " + str(full_header))
	var game_world := get_tree().current_scene as Node3D
	niflib.load_nif_scene(path_to_niffile, game_world, base_path)
