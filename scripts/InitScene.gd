# =============================================================================
# Script Name:        InitScene.gd
# Author(s):          Chrischn89
# Godot Version:      4.5
# Description:        
#     InitScene loads and set ups important assets to start the game
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
	await get_tree().process_frame
	
	var base_path : String = Global.load_value_from_config("GAME","original_folder_path")	# Load the original folder path from the config
	Font_Loader.load_fonts(base_path)		# Call function to load fonts
		
	#find_child("InitBGTexture").texture = ImageTextureUtil.load_texture_from_file(base_path.path_join("Assets/res/CivIV-INIT.bmp"))	 # Set texture for progress window background
	find_child("ProgressText").add_theme_font_override("font", Global.fonts.arial)	# Set font for progress window text
	
	find_child("ProgressText").text = "Init File Catalog"	# Set progress window text
	
	await get_tree().create_timer(1.0).timeout
	await get_tree().process_frame
	# Load original .fpk files into global arrays (base_path selected via the FileDialog and loaded from config)
	Global.assets0 = FPK_Loader.parse_fpk_archive(base_path.path_join("Assets/Assets0.fpk"))
	Global.assets1 = FPK_Loader.parse_fpk_archive(base_path.path_join("Assets/Assets1.fpk"))
	Global.assets2 = FPK_Loader.parse_fpk_archive(base_path.path_join("Assets/Assets2.fpk"))
	Global.assets3 = FPK_Loader.parse_fpk_archive(base_path.path_join("Assets/Assets3.fpk"))
	#print(Global.assets0)
	#print(Global.assets0["spy.kfm"].hex_encode())
	#print(Global.assets1["civtitle.dds"].hex_encode())
	#print(Global.assets3["maceman_md_fidget.kf"].hex_encode())
	
	await get_tree().create_timer(0.1).timeout
	find_child("ProgressText").text = "Init Check XML"
	await get_tree().create_timer(0.2).timeout
	find_child("ProgressText").text = "Init Audio"
	await get_tree().create_timer(0.2).timeout
	find_child("ProgressText").text = "Init Python"
	await get_tree().create_timer(0.2).timeout
	find_child("ProgressText").text = "Init XML (cached)"
	await get_tree().create_timer(0.2).timeout
	find_child("ProgressText").text = "Init Engine"
	await get_tree().create_timer(0.2).timeout
	find_child("ProgressText").text = "Init Fonts"
	await get_tree().create_timer(0.2).timeout
	
	call_deferred("_proceed_to_menu") # Proceed to MenuScene function

# Changes the scene to the main menu after short delay
func _proceed_to_menu() -> void:
	get_tree().change_scene_to_file("res://scenes/03_MenuScene.tscn")	# Load the main menu scene
