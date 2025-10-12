# =============================================================================
# Script Name:        Global.gd
# Author(s):          Chrischn89
# Godot Version:      4.5
# Description:        
#	Global.gd provides globally available arrays for storing loaded assets and
#	provides globally avaiable functions for basic operations like saving
#	and loading values from the config file
#
# TODO:              
#     -
#
#
# Notes:              
#     This script is autoloaded (Project>Project Settings>Globals>Autoload)
# 
# License:
#	Released under the terms of the GNU General Public License version 3.0
#
# =============================================================================

extends Node

# Loaded assets are cached in these arrays
var textures := {}
var fonts := {}
var cursors := {}

var assets := {}
var assets0 := {}
var assets1 := {}
var assets2 := {}
var assets3 := {}


# Path to the config file saved in the user's local Godot data directory (C:\Users\username\AppData\Roaming\Godot\app_userdata\PROJECTNAME)
const CONFIG_PATH := "user://Tessera.ini"


## Saves the input parameter 'value' to the config file at 'CONFIG_PATH' (e.g. user://Tessera.ini) next to the key entry 'CONFIG_KEY' under the section 'CONFIG_SECTION' (e.g. [game])
func save_value_to_config(value, CONFIG_SECTION: String, CONFIG_KEY: String) -> void:
	var config := ConfigFile.new()
	
	# Load existing config file (if it exists)
	var err = config.load(CONFIG_PATH)
	if err != OK and err == ERR_DOES_NOT_EXIST:
		push_error("Could not load config: %s" % err)
	
	# Modify or add entry to config file
	config.set_value(CONFIG_SECTION, CONFIG_KEY, value)
	
	# Save config file
	err = config.save(CONFIG_PATH)
	if err != OK:
		push_error("Failed to save config: %s" % err)
		
		
## Tries to load a value from the config file 'CONFIG_PATH' (e.g. user://Tessera.ini) next to the provided key entry 'CONFIG_KEY' under the section 'CONFIG_SECTION' (e.g. [game])
func load_value_from_config(CONFIG_SECTION: String, CONFIG_KEY: String) -> String:
	var config := ConfigFile.new()
	var err := config.load(CONFIG_PATH)
	
	#If loading the value is successful return the value else return NULL
	if err == OK and config.has_section_key(CONFIG_SECTION, CONFIG_KEY):
		return config.get_value(CONFIG_SECTION, CONFIG_KEY)
	return "<null>"
