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
	
	#var path_to_niffile : String = base_path.path_join("Assets/Art/Units/axeman/axeman.nif")
	#var path_to_niffile : String = base_path.path_join("Assets/Art/Units/MachineGun/machinegunner.nif")
	#var path_to_niffile : String = base_path.path_join("Assets/Art/Units/jetfighter/jetfighter.nif")
	#var path_to_niffile : String = base_path.path_join("Assets/Art/Terrain/Routes/Roads/roada00.nif")
	#var path_to_niffile : String = base_path.path_join("Assets/Art/Structures/Buildings/Forge/forge.nif")
	#var path_to_niffile : String = base_path.path_join("Assets/Art/Structures/Buildings/Castle/castle.nif")
	var path_to_niffile : String = base_path.path_join("Assets/Art/Units/Galley/galley_freeze1000.nif")
	print(path_to_niffile)
	
	#print(niflib.ping())  # -> "gdext_niflib OK"
	#print(niflib.get_nif_version_as_string(path_to_niffile))

	var niflib = GdextNiflib.new()
	
	if FileAccess.file_exists(path_to_niffile):
		# Full header
		print("\n")
		var full_header = niflib.get_nif_header(path_to_niffile)
		if full_header.success:
			print("Full Header: " + str(full_header))
			#print("Number of blocks: " + str(full_header.num_blocks))
			#print("Block types: ", full_header.block_types)
			#print("Copyright: ", full_header.copyright)
		var game_world := get_tree().current_scene as Node3D
		niflib.load_nif_scene(path_to_niffile, game_world)
	else:
		push_error("No .nif file found under provided path")
