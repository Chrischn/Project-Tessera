# =============================================================================
# Script Name:        PlayerPivot.gd
# Author(s):          Chrischn89
# Godot Version:      4.5
# Description:        
#	PlayerPivot.gd provides basic camera control in the 3D world
#	
#
# Usage:              
#     WASD: move direction
#     Hold Shift: Increase Speed
#     Spacebar: Go up
#     Ctrl: Go down
#     Mouse: look around
#     ESC: enable mouse cursor
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

extends Node3D

@export var cam: Camera3D
@export var move_speed := 8.0
@export var sprint_multiplier := 2.0
@export var mouse_sens := 0.002

var _pitch := 0.0  # in radiant

func _ready() -> void:
	if cam == null:
		cam = $Camera3D
	Input.set_mouse_mode(Input.MOUSE_MODE_CAPTURED)

func _unhandled_input(event: InputEvent) -> void:
	if event is InputEventMouseMotion:
		# Yaw around the pivot (left/right)
		rotate_y(-event.relative.x * mouse_sens)
		# Camera pitch (up/down, clamped)
		_pitch = clamp(_pitch - event.relative.y * mouse_sens, deg_to_rad(-89), deg_to_rad(89))
		cam.rotation.x = _pitch
	elif event.is_action_pressed("ui_cancel"):
		Input.set_mouse_mode(Input.MOUSE_MODE_VISIBLE)

func _physics_process(delta: float) -> void:
	var dir := Vector3.ZERO

	# WASD movement aligned to pivot, ignoring Y-axis for flat navigation
	var fwd := -global_transform.basis.z; fwd.y = 0; fwd = fwd.normalized()
	var right := global_transform.basis.x; right.y = 0; right = right.normalized()

	if Input.is_action_pressed("move_forward"): dir += fwd
	if Input.is_action_pressed("move_back"):    dir -= fwd
	if Input.is_action_pressed("move_left"):    dir -= right
	if Input.is_action_pressed("move_right"):   dir += right
	if Input.is_action_pressed("move_up"):      dir += Vector3.UP
	if Input.is_action_pressed("move_down"):    dir -= Vector3.UP

	if dir != Vector3.ZERO:
		var spd := move_speed * (sprint_multiplier if Input.is_action_pressed("move_sprint") else 1.0)
		global_translate(dir.normalized() * spd * delta)
