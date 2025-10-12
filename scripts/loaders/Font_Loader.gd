# =============================================================================
# Script Name:        Font_Loader.gd
# Author(s):          Chrischn89
# Godot Version:      4.5
# Description:        
#     Loads all fonts into the global array Global.fonts {}
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

extends RefCounted
class_name Font_Loader

## Loads all the fonts used by the game
static func load_fonts(base_path: String) -> void:
	
	# Load fonts
	var font_path_arial := base_path.path_join("Resource/Fonts/arial.ttf")
	var font_path_sylfaen := base_path.path_join("Resource/Fonts/sylfaen.ttf")
	#var font_path_sylfaenb := base_path.path_join("Resource/Civ4/sylfaenb.ttf")
	#var font_path_sylfaenbi := base_path.path_join("Resource/Civ4/sylfaenbi.ttf")
	#var font_path_sylfaeni := base_path.path_join("Resource/Civ4/sylfaeni.ttf")
		
	# Load font Arial
	if FileAccess.file_exists(font_path_arial):
		var arial = FontFile.new()
		arial.load_dynamic_font(font_path_arial)
		Global.fonts.arial = arial
	else:
		push_error("Missing: " + font_path_arial)
	
	# Load font Sylfaen
	if FileAccess.file_exists(font_path_sylfaen):
		var sylfaen = FontFile.new()
		sylfaen.load_dynamic_font(font_path_sylfaen)
		Global.fonts.sylfaen = sylfaen
	else:
		push_error("Missing: " + font_path_sylfaen)
