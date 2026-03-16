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

## Reads and parses a ByteArray as a .dds file and returns it as an ImageTexture.
## Pads truncated DDS mipmap chains so Godot's ResourceLoader accepts them.
static func load_dds_from_buffer(buf: PackedByteArray) -> ImageTexture:
	buf = _pad_dds_mipmaps(buf)
	var tmp := "user://__tmp__.dds"
	var f := FileAccess.open(tmp, FileAccess.WRITE)
	f.store_buffer(buf)
	f.close()
	var img = ResourceLoader.load(tmp, "ImageTexture", ResourceLoader.CACHE_MODE_IGNORE)
	DirAccess.remove_absolute(ProjectSettings.globalize_path(tmp))
	return img


## Fixes truncated DDS mipmap chains. Godot's DDS loader reads dwMipMapCount
## levels of data, then Image::initialize_data expects the full chain from
## dimensions. Both the header count AND data size must match the full chain.
static func _pad_dds_mipmaps(buf: PackedByteArray) -> PackedByteArray:
	if buf.size() < 128:
		return buf

	# Verify DDS magic
	if buf[0] != 0x44 or buf[1] != 0x44 or buf[2] != 0x53 or buf[3] != 0x20:
		return buf

	var mip_count: int = buf.decode_u32(28)
	if mip_count <= 1:
		return buf

	var pf_flags: int = buf.decode_u32(80)
	if (pf_flags & 0x4) == 0:
		return buf

	var fourcc := buf.slice(84, 88).get_string_from_ascii()
	var block_size := 0
	match fourcc:
		"DXT1":
			block_size = 8
		"DXT3", "DXT5":
			block_size = 16
		_:
			return buf

	var height: int = buf.decode_u32(12)
	var width: int = buf.decode_u32(16)

	# Compute full mipmap chain: level count and total data size (down to 1x1)
	var expected_data := 0
	var full_levels := 0
	var w := width
	var h := height
	while true:
		var bx: int = maxi(1, ceili(float(w) / 4.0))
		var by: int = maxi(1, ceili(float(h) / 4.0))
		expected_data += bx * by * block_size
		full_levels += 1
		if w == 1 and h == 1:
			break
		w = maxi(1, w >> 1)
		h = maxi(1, h >> 1)

	var data_available: int = buf.size() - 128
	var needs_fix := false

	if mip_count < full_levels:
		needs_fix = true
	if data_available < expected_data:
		needs_fix = true

	if not needs_fix:
		return buf

	#push_warning("[DDS] Fixing %dx%d %s: mips %d->%d, data %d->%d bytes" % [
	#	width, height, fourcc, mip_count, full_levels, data_available, expected_data])

	# Build corrected buffer: copy header, patch mip count, pad data
	var fixed := PackedByteArray()
	fixed.resize(128 + expected_data)
	# Copy original header and as much data as we have
	var copy_len: int = mini(buf.size(), 128 + expected_data)
	for i in copy_len:
		fixed[i] = buf[i]
	# Patch dwMipMapCount to full chain
	fixed.encode_u32(28, full_levels)
	return fixed


## Reads a file and tries to return it as an ImageTexture
static func load_texture_from_file(path: String) -> ImageTexture:
	# Try to access image file behind path and convert it to texture else error
	if FileAccess.file_exists(path):
		var image := Image.load_from_file(path)
		return ImageTexture.create_from_image(image)
	else:
		push_error("Missing: " + path)
		return
