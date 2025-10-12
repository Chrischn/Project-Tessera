# =============================================================================
# Script Name:        SplashScreenScene.gd
# Author(s):          Chrischn89
# Godot Version:      4.5
# Description:        
#     SplashScreenScene shows important legal notices
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

extends Control

# Called when the scene is ready
func _ready() -> void:
	#find_child("MenuBackground").texture = ImageTextureUtil.load_dds_from_buffer(Global.assets3["main menu\\stars.dds"])	# Load main menu background
	find_child("TextureRect_Ref1920x1080").texture = ImageTexture.create_from_image(Image.load_from_file("E:/Programming/Documentation/Civ4_Reference/TextureRect_Reference/2025-07-25 21_28_52-Greenshot.png"))	# Load Screenshot as reference for GUI creation
	
	var bts_exe_path : String = Global.load_value_from_config("GAME","bts_exe_path")
	
	if bts_exe_path != "<null>":	# Check if a path was previously saved succesfully
		await get_tree().create_timer(2.5).timeout # If Splashscreen was shown before shorten time
	else:
		await get_tree().create_timer(10.5).timeout
	call_deferred("_proceed_to_menu") # Proceed to MenuScene function
	

# Changes the scene to the main menu after short delay
func _proceed_to_menu() -> void:
	get_tree().change_scene_to_file("res://scenes/01_VerificationScene.tscn")	# Load the Menu scene
