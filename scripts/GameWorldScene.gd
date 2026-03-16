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

	var cycler := preload("res://scripts/AssetCycler.gd").new()
	cycler.name = "AssetCycler"
	add_child(cycler)
	cycler.setup(base_path)
