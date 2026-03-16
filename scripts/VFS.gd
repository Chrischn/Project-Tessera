# =============================================================================
# Script Name:        VFS.gd
# Author(s):          Chrischn89
# Godot Version:      4.5
# Description:
#	Virtual File System that unifies loose files on disk and FPK archive
#	contents behind a single lookup interface. Resolves file paths regardless
#	of whether the file exists as a loose file or inside an .fpk archive.
#
# Priority order (highest first):
#	1. Loose files on disk  (mods/expansions override base)
#	2. FPK archives: Assets3 > Assets2 > Assets1 > Assets0
#
# Notes:
#	This script is autoloaded (Project>Project Settings>Globals>Autoload).
#	FPK contents are indexed eagerly at startup. Loose files are resolved
#	lazily on demand (no upfront disk scan).
#
# License:
#	Released under the terms of the GNU General Public License version 3.0
#
# =============================================================================

extends Node


## Register as Engine singleton so C++ (GDExtension) code can find us
## via Engine::get_singleton()->get_singleton("VFS")
func _ready() -> void:
	Engine.register_singleton("VFS", self)

func _exit_tree() -> void:
	Engine.unregister_singleton("VFS")


## FPK index: normalized_path -> PackedByteArray
var _fpk_data := {}

## Base path to the original Civ IV installation (e.g. "E:/Programming/Civ4")
var _base_path := ""

## Cache directory for FPK files extracted to disk (needed by C++ Image::load())
var _cache_dir := "user://vfs_cache/"

## Disk path cache: normalized_path -> absolute disk path (avoids repeated FileAccess.file_exists checks)
var _disk_path_cache := {}

## Filename reverse index: bare_filename -> full_normalized_path
## Used when NIF files reference textures by filename only (no directory).
var _filename_index := {}

## Whether the VFS has been initialized
var _initialized := false


## Initializes the VFS by indexing FPK archive contents.
## Call this after FPK archives have been loaded.
## Assets are indexed in order 0->1->2->3 so that later archives overwrite
## earlier ones (Assets3 has highest priority among FPKs).
func initialize(base_path: String, assets0: Dictionary, assets1: Dictionary, assets2: Dictionary, assets3: Dictionary) -> void:
	_base_path = base_path
	_fpk_data.clear()
	_disk_path_cache.clear()
	_filename_index.clear()

	# Ensure cache directory exists
	DirAccess.make_dir_recursive_absolute(_cache_dir)

	# Index FPK contents: later archives overwrite earlier (correct priority)
	var total := 0
	for fpk_dict in [assets0, assets1, assets2, assets3]:
		for key in fpk_dict:
			var normalized := _normalize_path(key)
			_fpk_data[normalized] = fpk_dict[key]
			# Build filename reverse index (bare filename -> full path)
			var filename := normalized.get_file()
			_filename_index[filename] = normalized
		total += fpk_dict.size()

	_initialized = true
	print("[VFS] Initialized: %d FPK entries indexed (%d unique paths, %d filename lookups)" % [total, _fpk_data.size(), _filename_index.size()])


## Normalizes a file path for consistent lookups.
## Handles mixed slashes, case differences, and optional "assets/" prefix.
func _normalize_path(path: String) -> String:
	var result := path.to_lower()
	result = result.replace("\\", "/")
	# Strip leading slash
	if result.begins_with("/"):
		result = result.substr(1)
	# Strip leading "assets/" prefix (NIF refs sometimes include it, sometimes not)
	if result.begins_with("assets/"):
		result = result.substr(7)  # len("assets/") == 7
	return result


## Resolves a normalized path to its full FPK index key.
## If the path is already a full path in the index, returns it as-is.
## If not, tries a filename-only lookup (NIF files often reference textures
## by bare filename without any directory).
## Returns empty string if not found in FPK index at all.
func _resolve_fpk_path(normalized: String) -> String:
	# Direct full-path lookup
	if _fpk_data.has(normalized):
		return normalized
	# Filename-only fallback (e.g. "submarine_alpha_ 256.dds" -> "art/units/submarine/submarine_alpha_ 256.dds")
	var filename := normalized.get_file()
	if _filename_index.has(filename):
		return _filename_index[filename]
	return ""


## Returns true if the file exists in any source (disk or FPK).
func has_file(path: String) -> bool:
	var normalized := _normalize_path(path)
	# Check disk first (loose files override FPK)
	if _check_disk(normalized) != "":
		return true
	# Check FPK index (with filename fallback)
	return _resolve_fpk_path(normalized) != ""


## Returns the raw file data as a PackedByteArray.
## Checks loose files on disk first, then FPK archives.
## Returns an empty PackedByteArray if the file is not found.
func get_file_data(path: String) -> PackedByteArray:
	var normalized := _normalize_path(path)

	# Check disk first (loose files have highest priority)
	var disk_path := _check_disk(normalized)
	if disk_path != "":
		var file := FileAccess.open(disk_path, FileAccess.READ)
		if file != null:
			var data := file.get_buffer(file.get_length())
			file.close()
			return data

	# Check FPK index (with filename fallback)
	var fpk_key := _resolve_fpk_path(normalized)
	if fpk_key != "":
		return _fpk_data[fpk_key]

	return PackedByteArray()


## Returns an absolute disk path for the file. This is the primary method
## for C++ code (Godot's Image::load() requires a file path).
## For loose files: returns the actual disk path.
## For FPK files: extracts to a cache directory and returns the cache path.
## Returns an empty string if the file is not found.
func get_file_as_disk_path(path: String) -> String:
	var normalized := _normalize_path(path)

	# Check disk first (loose files have highest priority)
	var disk_path := _check_disk(normalized)
	if disk_path != "":
		return disk_path

	# Check FPK index (with filename fallback) -> extract to cache
	var fpk_key := _resolve_fpk_path(normalized)
	if fpk_key != "":
		return _extract_to_cache(fpk_key)

	return ""


## Checks if a file exists as a loose file on disk.
## Returns the absolute path if found, empty string otherwise.
## Results are cached to avoid repeated FileAccess.file_exists calls.
func _check_disk(normalized_path: String) -> String:
	# Check cache first
	if _disk_path_cache.has(normalized_path):
		return _disk_path_cache[normalized_path]

	# Try base_path/Assets/<normalized_path>
	var candidate := _base_path.path_join("Assets").path_join(normalized_path)
	if FileAccess.file_exists(candidate):
		_disk_path_cache[normalized_path] = candidate
		return candidate

	# Not found on disk
	_disk_path_cache[normalized_path] = ""
	return ""


## Returns a fully-loaded ImageTexture for any file resolvable via VFS.
## Handles DDS (via ResourceLoader) and standard formats (BMP, TGA, PNG) via Image.
## This is the method C++ should call instead of resolving a path and using Image::load().
func load_image_texture(path: String) -> ImageTexture:
	var data := get_file_data(path)
	if data.is_empty():
		return null
	var ext := path.get_extension().to_lower()
	if ext == "dds":
		return ImageTextureUtil.load_dds_from_buffer(data)
	# BMP, TGA, PNG, JPG — use Image's built-in buffer loaders
	var img := Image.new()
	var err : Error
	match ext:
		"bmp":
			err = img.load_bmp_from_buffer(data)
		"tga":
			err = img.load_tga_from_buffer(data)
		"png":
			err = img.load_png_from_buffer(data)
		"jpg", "jpeg":
			err = img.load_jpg_from_buffer(data)
		_:
			push_warning("[VFS] Unsupported image format: %s" % ext)
			return null
	if err != OK:
		push_warning("[VFS] Failed to load %s as %s (error %d)" % [path, ext, err])
		return null
	return ImageTexture.create_from_image(img)


## Returns all VFS-known paths whose normalized form starts with dir_prefix
## and ends with the given extension. Searches both FPK index and loose files.
## dir_prefix should use forward slashes (e.g. "art/units"). Case-insensitive.
## Returns normalized (lowercase, forward-slash) paths, sorted.
func find_files(dir_prefix: String, extension: String) -> Array[String]:
	var prefix := _normalize_path(dir_prefix)
	var ext := ("." + extension.to_lower().trim_prefix("."))
	var results: Array[String] = []
	var seen := {}

	# FPK index
	for key: String in _fpk_data:
		if key.begins_with(prefix) and key.ends_with(ext):
			results.append(key)
			seen[key] = true

	# Loose files on disk
	if _base_path != "":
		var scan_root := _base_path.path_join("Assets").path_join(dir_prefix)
		_scan_disk_recursive(scan_root, prefix + "/", ext, results, seen)

	results.sort()
	return results


func _scan_disk_recursive(abs_path: String, rel_prefix: String, ext: String, results: Array[String], seen: Dictionary) -> void:
	var dir := DirAccess.open(abs_path)
	if dir == null:
		return
	dir.list_dir_begin()
	var entry := dir.get_next()
	while entry != "":
		if dir.current_is_dir():
			_scan_disk_recursive(
				abs_path.path_join(entry),
				rel_prefix + entry.to_lower() + "/",
				ext, results, seen
			)
		elif entry.to_lower().ends_with(ext):
			var normalized := rel_prefix + entry.to_lower()
			if not seen.has(normalized):
				results.append(normalized)
				seen[normalized] = true
		entry = dir.get_next()
	dir.list_dir_end()


## Extracts an FPK file to the cache directory and returns the globalized path.
## Cached files persist for the session (no re-extraction on repeated lookups).
func _extract_to_cache(normalized_path: String) -> String:
	var cache_path := _cache_dir.path_join(normalized_path)
	var global_path := ProjectSettings.globalize_path(cache_path)

	# Check if already cached
	if FileAccess.file_exists(cache_path):
		return global_path

	# Ensure parent directories exist
	var dir := cache_path.get_base_dir()
	DirAccess.make_dir_recursive_absolute(dir)

	# Write FPK data to cache file
	var file := FileAccess.open(cache_path, FileAccess.WRITE)
	if file == null:
		push_error("[VFS] Failed to write cache file: %s" % cache_path)
		return ""
	file.store_buffer(_fpk_data[normalized_path])
	file.close()

	return global_path
