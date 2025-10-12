# =============================================================================
# Script Name:        FPK_Loader.gd
# Author(s):          Chrischn89
# Godot Version:      4.5
# Description:        
#     Reads and parses the header, filelist and files of a .fpk archive, 
#     prints its information to the console and returns a Dictionary with
#     every dissected file as byte code
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
class_name FPK_Loader

## Parses an entire .fpk archive and returns its files as a Dictionary
static func parse_fpk_archive(fpk_file_path: String) -> Dictionary:
	if fpk_file_path == "":
		push_error("No .fpk file path specified.")
		return {}

	var header = parse_fpk_header(fpk_file_path)
	var filelist = parse_fpk_filelist(fpk_file_path, header.file_list_offset, header.file_count)
	return filelist

## Read header fields and return basic information including where filelist begins
static func parse_fpk_header(path: String) -> Dictionary:
	var file = FileAccess.open(path, FileAccess.READ)
	if file == null:
		push_error("Failed to open file: %s" % path)
		return {}
	
	# Each get_ call moves the file cursor ahead
	var version = file.get_32()
	var magic = file.get_buffer(4).get_string_from_utf8()
	var unknown = file.get_8()
	var file_count = file.get_32()
	var list_offset = file.get_position()
	file.close()
	
	if magic != "FPK_": # Validate reported Magic field in header equals "FPK_"
		push_error("Error reading .fpk header: %s" % path)
		return {}
	
	print("--- FPK Header Info ---")
	print("Version        : %d" % version)
	print("Magic          : %s" % magic)
	print("Unknown byte   : %d" % unknown)
	print("Number of files: %d" % file_count)

	return {
		"version": version,
		"magic": magic,
		"unknown": unknown,
		"file_count": file_count,
		"file_list_offset": list_offset
	}

## Iterate over each filelist entry, detect (unknown length) 'signature' by finding 'bsu' marker, and write signature
static func parse_fpk_filelist(path: String, start_offset: int, entry_count: int) -> Dictionary:
	
	# Open .fpk archive file
	var file := FileAccess.open(path, FileAccess.READ)
	if file == null:
		push_error("Failed to open file for filelist: %s" % path)
		return {}
	
	# Open output text-file to store debug information
	#var sig_file := FileAccess.open("res://debug/fpk_signatures.txt", FileAccess.READ_WRITE)
	#if sig_file == null:
		#push_error("Failed to open signature output file.")
		#file.close()
		#return {}
	
	var filelist := {}
	file.seek(start_offset)	# Jump to start of filelist (comes after header)
	
	for i in entry_count:	# Iterate over every entry in filelist
		
		# Read next entry's path length, actual path and file name
		var path_len := file.get_32()
		var path_bytes := file.get_buffer(path_len)
		var file_path := rotate_packed_byte_array(path_bytes,-1).get_string_from_utf8()
		var file_folder := file_path.get_slice("\\", file_path.get_slice_count("\\")-2)
		var file_name = file_path.get_file()
		var pos_after_path := file.get_position()
		
		# Read a preview buffer to locate 'bsu' (0x62 0x73 0x75) =  unencrypted 'art' to determine start of next file
		var preview := file.get_buffer(30)
		var found_idx := -1
		for j in range(preview.size() - 3, -1, -1):
			if preview[j] == 0x62 and preview[j + 1] == 0x73 and preview[j + 2] == 0x75:	# Check for 'bsu' marker -> start of next filelist entry
				found_idx = j
				break
			if preview[j] == 0x03 and preview[j + 1] == 0x00 and preview[j + 2] == 0x00:	# Check for EOF marker = end of file list -> start of files section
				found_idx = j + 4		# +4 because there is no next file path size because we reached the end of the filelist
				break
		if found_idx < 0:
			push_warning("Could not find 'bsu' or 'EOF' marker after path: %s" % file_path)
			break

		# Calculate 'signature' boundaries
		var bsu_pos := pos_after_path + found_idx # 'bsu' marker position = position after File Length Position + 30 bytes preview - Position of found_idx marker
		var sig_end := bsu_pos - 12 # Position of Signature end is always 12 Bytes away from the next 'bsu' # 12 Bytes = [File Size](4 Bytes) + [File Offset](4 Bytes) + ['Art' = 'bsu' position](4 Bytes)
		var sig_len := sig_end - pos_after_path # Signature length
		if sig_len < 0:
			push_warning("Negative signature length for entry: %s" % file_path)
			break

		# Extract Signature bytes
		file.seek(pos_after_path)
		var _signature_bytes := file.get_buffer(sig_len) # Now that Signature Length is determined we can read the Signature
		
		
		# Read the file size and file offset fields
		var file_size := file.get_32()
		var file_offset := file.get_32()
		var pos_before_next_file := file.get_position()
		
		# Read the actual file data bytes
		file.seek(file_offset) 		# Jump to the file section at the exact position where the file is according to the file list
		var file_data := file.get_buffer(file_size) # Read file as byte code
		filelist[file_folder + "\\" + file_name] = file_data 	# Add file to Dictionary. Filefolder + Filename is the key : Data is the value. CAUTION: Key might not be unique if there are two folders with two files with the same names inside the same .fpk archive! To change: adjust slice count for var file_folder
		#sig_file.store_line("%s | %s | sig_len=%d | %s | %d | %s" % [file_path, rotate_packed_byte_array(path_bytes,+1).get_string_from_utf8(),sig_len, _signature_bytes.hex_encode(),filelist.size(),file_folder]) # Debug output
		file.seek(pos_before_next_file) 	# Jump to start of next file in filelist
	# Loop continues 
	#sig_file.store_line("%s" % [filelist.keys()]) # Debug output
	#sig_file.close() # Debug output
	print("filelist.size: ", filelist.size(), " | entry_count: ", entry_count)
	if filelist.size() == entry_count: # Validate number of files reported in the header equals number of entries returned via Dictionary
		print(".fpk archive loaded successfully")
		file.close()
		return filelist
	push_error("Error unpacking .fpk file: %s" % path)
	return {}

## Rotate each byte from input (PackedByteArray) by amount of second input (integer) and return rotated PackedByteArray (aka ROT-1 decryption).
static func rotate_packed_byte_array(data: PackedByteArray, rotate: int) -> PackedByteArray:
	var n := data.size()
	if n == 0:
		return PackedByteArray()
	
	for i in n:
		data[i] = data[i] + rotate
	return data
