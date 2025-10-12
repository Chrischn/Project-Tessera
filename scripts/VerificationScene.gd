# =============================================================================
# Script Name:        VerificationScene.gd
# Author(s):          Chrischn89
# Godot Version:      4.5
# Description:        
#	VerificationScene.gd forces the user to point to a valid Civ4BeyondSword.exe
#	in order to load original assets from the relative folder path
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

## Called when the scene is ready; attempts to load stored exe path
func _ready() -> void:
	
	# Call global function load_value_from_config to check if a path to the Civ4BeyondSword.exe has already been provided in the past by the user and saved to the config
	#var original_exe_path : String = Global.load_value_from_config("GAME","original_exe_path")
	var bts_exe_path : String = Global.load_value_from_config("GAME","bts_exe_path")
	#var warlords_exe_path : String = Global.load_value_from_config("GAME","warlords_exe_path")
	
	if bts_exe_path != "<null>":	# Check if a path was previously saved succesfully
		hash_file(bts_exe_path) 		# Check hash of provided .exe
		await get_tree().create_timer(2.5).timeout
		call_deferred("_proceed_to_init") 	# Proceed to Initialization scene function
	else:
		$FileDialog.visible = true		# Else show the FileDialog to let the user point to the Civ4BeyondSword.exe

## Called when the user selects a file from the FileDialog
func _on_FileDialog_file_selected(selected_path: String) -> void:
	
	# Save original .exe and folder paths to the config file for future use
	Global.save_value_to_config((selected_path.get_base_dir()).get_base_dir(),"GAME","original_folder_path")
	Global.save_value_to_config(((selected_path.get_base_dir()).get_base_dir()).path_join("Warlords"),"GAME","warlords_folder_path")	
	Global.save_value_to_config(selected_path.get_base_dir(),"GAME","bts_folder_path")
	
	Global.save_value_to_config(Global.load_value_from_config("GAME","original_folder_path").path_join("Civilization4.exe"),"GAME","original_exe_path")
	Global.save_value_to_config(Global.load_value_from_config("GAME","warlords_folder_path").path_join("Civ4Warlords.exe"),"GAME","warlords_exe_path")
	Global.save_value_to_config(Global.load_value_from_config("GAME","bts_folder_path").path_join("Civ4BeyondSword.exe"),"GAME","bts_exe_path")
		
	call_deferred("_proceed_to_init") 	# Proceed to initialization scene function

## Changes the scene to the initializatiion scene after short delay
func _proceed_to_init() -> void:
	get_tree().change_scene_to_file("res://scenes/02_InitScene.tscn")	# Load the initializatiion scene
		
	

# hash_file function source: https://docs.godotengine.org/en/4.4/classes/class_hashingcontext.html
## Hashes the provided file via path and checks it against a hardcoded list of legitimate hashes
func hash_file(path) -> void:
	const CHUNK_SIZE = 1024
	
	if not FileAccess.file_exists(path):	# Check that file exists.
		print("Aborted hashing ",path)
		push_error("No valid .exe found: ", path)
		get_tree().quit()
		return
		
	var ctx = HashingContext.new()	# Start an SHA-256 context.
	ctx.start(HashingContext.HASH_SHA256)
		
	var file = FileAccess.open(path, FileAccess.READ)	# Open the file to hash.
		
	while file.get_position() < file.get_length():	# Update the context after reading each chunk.
		var remaining = file.get_length() - file.get_position()
		ctx.update(file.get_buffer(min(remaining, CHUNK_SIZE)))
	
	var res = ctx.finish()	# Get the computed hash.
	
	# Print the result as hex string and array.
	#printt(res.hex_encode().to_upper(), Array(res))
	
	# Verify the calculated hash is among the hardcoded valid ones
	match res.hex_encode().to_upper():
		#"DF81D1D5DB59CAC3541BBE707F11BD48DC0EE2D26D03996726799BE939743DA3": 			#Civilization4.exe
		#"FB5A2BC7EAE112F79DA6DABD9DD0B9CED1152D25EA46DC2D92D9DCAA208C530D": 			#Civ4Warlords.exe
		#"C5FB4BF377BBA2C80C7AFAE21BE34AFE0648E4ED87DA24E3A65C393B9F2263CB": 			#Civ4BeyondSword.exe with LAA Flag 4GB?
		"90DBDBDCBD6C631C9D575881D0241D9A2012B1A5957BACBE68212745DA22174E":				#Civ4BeyondSword.exe

			print("Validation of hash Civ4BeyondSword.exe successful ", Global.load_value_from_config("GAME","bts_exe_path"))
		_:
			push_error("Invalid hash of .exe: ", path)
			get_tree().quit()
