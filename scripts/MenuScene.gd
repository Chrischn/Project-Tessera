# =============================================================================
# Script Name:        MenuScene.gd
# Author(s):          Chrischn89
# Godot Version:      4.5
# Description:        
#	MenuScene.gd controls functionality and partly the design of the game's
#	main menu
#
# TODO:              
#     - create the blinking arrow effect left and right of a highlighted menu button by using /Resource/Temp/arrow_ani_01.tga, /Resource/Temp/arrow_ani_02.tga etc.
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

func _ready() -> void:
	init_MenuScene()

func init_MenuScene() -> void:
	var base_path : String = Global.load_value_from_config("GAME","original_folder_path")
	
	#Duplicate the editor-assigned theme for the menu (primarily buttons) so that it can be edited by script since the static editor-theme is read-only during runtime
	var original_theme = find_child("MenuCenter").theme
	var button_theme = original_theme.duplicate()
	button_theme.set_font("font", "Button", Global.fonts.sylfaen)
	find_child("MenuCenter").theme = button_theme
	find_child("MenuBottomBar").theme = button_theme
	#find_child("SubMenu_Credits").theme = button_theme
	
	#find_child("TextureRect_Ref1042x786").texture = ImageTexture.create_from_image(Image.load_from_file("E:/Programming/Documentation/Civ4_Reference/TextureRect_Reference/2025-07-24 03_09_28-Civilization IV.png"))	# Load Screenshot as reference for GUI creation
	#find_child("TextureRect_Ref1920x1080").texture = ImageTexture.create_from_image(Image.load_from_file("E:/Programming/Documentation/Civ4_Reference/TextureRect_Reference/2025-07-24 10_51_33-Greenshot.png"))	# Load Screenshot as reference for GUI creation
	
	find_child("MenuBackground").texture = ImageTextureUtil.load_dds_from_buffer(Global.assets3["main menu\\stars.dds"])	# Load main menu background
	#find_child("Logo").texture = ImageTextureUtil.load_dds_from_buffer(Global.assets1["main menu\\civtitle.dds"])	# Load main menu logo
	#find_child("Logo").texture = ImageTextureUtil.load_texture_from_file((Global.load_value_from_config("GAME","original_folder_path")).path_join("Resource/Temp/select_c.tga"))
	
	find_child("Logo").texture = load("res://assets/logos/Project_Tessera_Logo.svg")
	find_child("Logo").stretch_mode = TextureRect.EXPAND_FIT_HEIGHT_PROPORTIONAL
	
	
	var music = AudioStreamMP3.load_from_file(base_path.path_join("Assets/Sounds/Soundtrack/OpeningMenu.mp3"))	# Load main menu music
	$AudioStreamPlayer.stream = music
	$AudioStreamPlayer.play(0.0) # Play main menu music
	
	
	# Recursively search through the root and all of its child-Nodes to find all the Buttons
	for button in find_children("*","Button",true,true):
		# Connect every actual Button to the "_on_button_pressed" function
		if button is Button:
			button.pressed.connect(self._on_button_pressed.bind(button))

## Hide alle currently visible menu Nodes
func hide_all_menus() -> void:
	# Array of all hardcoded menus in the main menu
	var menus : Array[CanvasItem] = [$MenuCenter/DefaultMenu, $MenuCenter/SubMenuSingleplayer, $MenuCenter/SubMenuMultiplayer, $MenuCenter/SubMenuInternetGames, $MenuCenter/SubMenuHotSeat, $MenuCenter/SubMenuPlayByEmail, $MenuCenter/SubMenuAdvanced] 
	
	# Iterate through all known menus and hide them
	for single_menu in menus:
		single_menu.hide()

## Find currently visible menu Node
func find_visible_menu() -> CanvasItem:
	# Array of all hardcoded menus in the main menu
	var menus : Array[CanvasItem] = [$MenuCenter/DefaultMenu, $MenuCenter/SubMenuSingleplayer, $MenuCenter/SubMenuMultiplayer, $MenuCenter/SubMenuInternetGames, $MenuCenter/SubMenuHotSeat, $MenuCenter/SubMenuPlayByEmail, $MenuCenter/SubMenuAdvanced] 
	
	# Iterate through all known menus and try to find the one that is currently visible and return it, else return null
	for single_menu in menus:
		if single_menu.visible:
			return (single_menu)
	return null

## Function that controls the functionality of the main menu elements (buttons)
func _on_button_pressed(caller: Button) -> void:
		
	var last_visible_menu = find_visible_menu()	# Find the currently visible menu Node and save it, to jump back to it later via the Go Back button
	
	hide_all_menus()	# Hide all of the menus in preparation to open the next menu in accordance to what button was pressed
	
	# Whatever button is pressed in the main menu the Exit Button is replaced by the Go Back button
	$MenuBottomBar/HBoxContainer/ExitGameButton.hide()
	$MenuBottomBar/HBoxContainer/GoBackButton.show()
	
	# Switch case to catch which button was pressed and to perform the appropriate menu action
	match caller.name:
		"ExitGameButton":
			get_tree().quit()
		"GoBackButton":
			# Check if a valid menu was returned by find_visible_menu() earlier
			if last_visible_menu != null:
				# Check the returned menu against all known hardcoded menus
				match last_visible_menu.name:
					# DefaultMenu can never be its own last_visible_menu as you can't call the DefaultMenu from the DefaultMenu directly
					"DefaultMenu":
						pass
					# SubMenuSingleplayer has no further submenus at the moment, therefor it returns to the DefaultMenu
					"SubMenuSingleplayer":
						$MenuBottomBar/HBoxContainer/ExitGameButton.show()
						$MenuBottomBar/HBoxContainer/GoBackButton.hide()
						$MenuCenter/DefaultMenu.show()
					# SubMenuMultiplayer has no further submenus at the moment, therefor it returns to the DefaultMenu
					"SubMenuMultiplayer":
						$MenuBottomBar/HBoxContainer/ExitGameButton.show()
						$MenuBottomBar/HBoxContainer/GoBackButton.hide()
						$MenuCenter/DefaultMenu.show()
					# SubMenuInternetGames is a sub-menu of SubMenuMultiplayer, therefor it returns to SubMenuMultiplayer
					"SubMenuInternetGames":
						$MenuCenter/SubMenuMultiplayer.show()
					# SubMenuHotSeat is a sub-menu of SubMenuMultiplayer, therefor it returns to SubMenuMultiplayer
					"SubMenuHotSeat":
						$MenuCenter/SubMenuMultiplayer.show()
					# SubMenuPlayByEmail is a sub-menu of SubMenuMultiplayer, therefor it returns to SubMenuMultiplayer
					"SubMenuPlayByEmail":
						$MenuCenter/SubMenuMultiplayer.show()
					# SubMenuMultiplayer has no further submenus at the moment, therefor it returns to the DefaultMenu
					"SubMenuAdvanced":
						$MenuBottomBar/HBoxContainer/ExitGameButton.show()
						$MenuBottomBar/HBoxContainer/GoBackButton.hide()
						$MenuCenter/DefaultMenu.show()
					# Default case to catch errors, therefor it returns to the DefaultMenu
					_:
						$MenuBottomBar/HBoxContainer/ExitGameButton.show()
						$MenuBottomBar/HBoxContainer/GoBackButton.hide()
						$MenuCenter/DefaultMenu.show()
			else:
				# Error case to catch errors, therefor it returns to the DefaultMenu
				print("Error: visible_menu returned: ",last_visible_menu)
				print ("Returning to main menu...")
				$MenuBottomBar/HBoxContainer/ExitGameButton.show()
				$MenuBottomBar/HBoxContainer/GoBackButton.hide()
				$MenuCenter/DefaultMenu.show()
		"SinglePlayerButton":
			$MenuCenter/SubMenuSingleplayer.show()
		"MultiplayerButton":
			$MenuCenter/SubMenuMultiplayer.show()
		"HallOfFameButton":
			#$MenuCenter/SubMenuHallOfFame.show()
			pass
		"CivilopediaButton":
			#$MenuCenter/SubMenuCivilopedia.show()
			pass
		"TutorialButton":
			#$MenuCenter/SubMenuTutorial.show()
			pass
		"AdvancedButton":
			$MenuCenter/SubMenuAdvanced.show()
			pass
		"PlayNowButton":
			#$MenuCenter/SubMenuPlayNow.show()
			get_tree().change_scene_to_file("res://scenes/04_GameWorldScene.tscn")
			#pass
		"LoadGameButton":
			#$MenuCenter/SubMenuLoadGame.show()
			pass
		"PlayScenarioButton":
			#$MenuCenter/SubMenuPlayScenario.show()
			pass
		"CustomGameButton":
			#$MenuCenter/SubMenuCustomGame.show()
			pass
		"CustomScenarioButton":
			#$MenuCenter/SubMenuCustomScenario.show()
			pass
		"LANGamesButton":
			#$MenuCenter/SubMenuLANGames.show()
			pass
		"InternetGamesButton":
			$MenuCenter/SubMenuInternetGames.show()
			
		"DirectIPConnectionButton":
			#$MenuCenter/SubMenuDirectIPConnection.show()
			pass
		"HotSeatButton":
			$MenuCenter/SubMenuHotSeat.show()
			
		"PlaybyEmailButton":
			$MenuCenter/SubMenuPlayByEmail.show()
			
		"LoginButton":
			#$MenuCenter/SubMenuLogin.show()
			pass
		"CreateNewAccountButton":
			#$MenuCenter/SubMenuCreateNewAccount.show()
			pass
		"NewGameButton":
			#$MenuCenter/SubMenuNewGame.show()
			pass
		"NewScenarioButton":
			#$MenuCenter/SubMenuNewScenario.show()
			pass
		"LoadGameButton":
			#$MenuCenter/SubMenuLoadGame.show()
			pass
		"NewGameButton":
			#$MenuCenter/SubMenuNewGame.show()
			pass
		"NewScenarioButton":
			#$MenuCenter/SubMenuNewScenario.show()
			pass
		"LoadGameButton":
			#$MenuCenter/SubMenuLoadGame.show()
			pass
		"AboutBuildButton":
			#$MenuCenter/SubMenuAboutBuild.show()
			pass
		"LoadModButton":
			#$MenuCenter/SubMenuLoadMod.show()
			pass
		"OptionsButton":
			#$MenuCenter/SubMenuOptions.show()
			pass
		"CheckUpdatesButton":
			#$MenuCenter/SubMenuCheckUpdates.show()
			pass
		"CreditsButton":
			#$SubMenuCredits.show()
			pass
		_:
			print("No valid/recognized button was pressed")
