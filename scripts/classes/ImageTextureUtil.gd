# =============================================================================
# Script Name:        Class_ImageTextureUtil.gd
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

extends ImageTexture
class_name ImageTextureUtil

## Reads and parses a ByteArray as a .dds file and returns it as an ImageTexture
static func load_dds_from_buffer(ByteArray: PackedByteArray) -> ImageTexture:
	var tmp := "user://__tmp__.dds"
	var f := FileAccess.open(tmp, FileAccess.WRITE)
	f.store_buffer(ByteArray)
	f.close()
	var img = ResourceLoader.load(tmp,"ImageTexture",ResourceLoader.CACHE_MODE_IGNORE)
	DirAccess.remove_absolute(ProjectSettings.globalize_path(tmp))
	return img
	
	
## Reads a file and tries to return it as an ImageTexture
static func load_texture_from_file(path: String) -> ImageTexture:
	# Try to access image file behind path and convert it to texture else error
	if FileAccess.file_exists(path):
		var image := Image.load_from_file(path)
		return ImageTexture.create_from_image(image)
	else:
		push_error("Missing: " + path)
		return
