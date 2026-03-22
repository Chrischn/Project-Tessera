# =============================================================================
# Script Name:        HostBridge.gd
# Author(s):          Chrischn89
# Godot Version:      4.5
# Description:
#     Manages the TesseraHost.exe child process lifecycle, TCP connection, and
#     data queries. Spawns the host, connects via length-prefixed JSON over TCP,
#     and provides query methods for game info (XML data, art definitions, etc.).
#
# License:
#	Released under the terms of the GNU General Public License version 3.0
# =============================================================================

class_name HostBridge
extends Node

## Emitted when the TCP connection to TesseraHost is lost unexpectedly.
signal connection_lost()

# --- Configuration -----------------------------------------------------------

const HEALTH_PING_INTERVAL := 30.0   # seconds between keep-alive pings
const INIT_TIMEOUT := 60.0           # seconds to wait for init response
const QUERY_TIMEOUT := 5.0           # seconds to wait for query responses
const CONNECT_TIMEOUT := 10.0        # seconds to wait for TCP connection
const POLL_INTERVAL := 0.05          # seconds between poll ticks

# --- Internal state ----------------------------------------------------------

var _tcp: StreamPeerTCP = null
var _host_pid: int = -1
var _port: int = 0
var _connected: bool = false
var _health_timer: Timer = null


# =============================================================================
#  Public API
# =============================================================================

## Spawn TesseraHost.exe and establish a TCP connection.
## Returns true on success, false on failure.
func spawn_host() -> bool:
	# Find a free port
	_port = _find_free_port()
	if _port == 0:
		push_error("[HostBridge] Could not find a free TCP port")
		return false
	print("[HostBridge] Using port %d" % _port)

	# Resolve host exe path
	var host_path := _resolve_host_path()
	if host_path.is_empty():
		push_error("[HostBridge] Could not resolve TesseraHost.exe path")
		return false
	if not FileAccess.file_exists(host_path):
		push_error("[HostBridge] TesseraHost.exe not found at: " + host_path)
		return false
	print("[HostBridge] Host exe: " + host_path)

	# Spawn the process
	var args := ["--port", str(_port)]
	if OS.get_name() == "Linux":
		_host_pid = OS.create_process("wine", [host_path] + args)
	else:
		_host_pid = OS.create_process(host_path, args)

	if _host_pid <= 0:
		push_error("[HostBridge] Failed to create host process")
		return false
	print("[HostBridge] Spawned host process PID=%d" % _host_pid)

	# Connect via TCP
	_tcp = StreamPeerTCP.new()
	_tcp.big_endian = false  # CRITICAL: protocol uses little-endian length prefix

	var connect_elapsed := 0.0
	while connect_elapsed < CONNECT_TIMEOUT:
		var err := _tcp.connect_to_host("127.0.0.1", _port)
		if err == OK:
			# Poll until connected or error
			var poll_elapsed := 0.0
			while poll_elapsed < CONNECT_TIMEOUT - connect_elapsed:
				_tcp.poll()
				var status := _tcp.get_status()
				if status == StreamPeerTCP.STATUS_CONNECTED:
					_connected = true
					print("[HostBridge] TCP connected to 127.0.0.1:%d" % _port)
					_start_health_timer()
					return true
				elif status == StreamPeerTCP.STATUS_ERROR:
					break
				await get_tree().create_timer(POLL_INTERVAL).timeout
				poll_elapsed += POLL_INTERVAL
		# Connection attempt failed, retry after a short wait
		await get_tree().create_timer(0.25).timeout
		connect_elapsed += 0.25
		_tcp.disconnect_from_host()

	push_error("[HostBridge] TCP connection timed out after %.1fs" % CONNECT_TIMEOUT)
	return false


## Send the init command to load XML data from the given base path.
## Returns the response dictionary ({"status":"ok"} on success).
func init_game(base_path: String, mod = null) -> Dictionary:
	var cmd := {"cmd": "init", "base_path": base_path}
	if mod != null and mod is String and not (mod as String).is_empty():
		cmd["mod"] = mod
	return await _send_and_recv(cmd, INIT_TIMEOUT)


## Query a single info record by type and key.
## Example: get_info("tech", "TECH_AGRICULTURE")
func get_info(type: String, key: String) -> Dictionary:
	return await _send_and_recv({"cmd": "get_info", "type": type, "key": key}, QUERY_TIMEOUT)


## Query all info records for a given type.
## Example: get_all_infos("tech") → {"status":"ok","type":"tech","count":86,"items":[...]}
func get_all_infos(type: String) -> Dictionary:
	return await _send_and_recv({"cmd": "get_all_infos", "type": type}, QUERY_TIMEOUT)


## Query art info by type and key.
## Example: get_art_info("unit_art", "ART_DEF_UNIT_WARRIOR")
func get_art_info(type: String, key: String) -> Dictionary:
	return await _send_and_recv({"cmd": "get_art_info", "type": type, "key": key}, QUERY_TIMEOUT)


## Gracefully shut down the host process.
func shutdown() -> void:
	if _health_timer and _health_timer.is_inside_tree():
		_health_timer.stop()
	if _connected and _tcp != null:
		_send_raw({"cmd": "shutdown"})
		# Give it a moment to process
		await get_tree().create_timer(0.2).timeout
		_tcp.disconnect_from_host()
	_connected = false
	_tcp = null
	if _host_pid > 0:
		# OS.kill does not exist in Godot 4.5; the shutdown command tells the
		# host to exit. If it does not, the OS will clean up when Godot exits.
		_host_pid = -1
	print("[HostBridge] Shutdown complete")


# =============================================================================
#  Internals
# =============================================================================

func _find_free_port() -> int:
	var server := TCPServer.new()
	# Port 0 tells the OS to assign a free ephemeral port
	var err := server.listen(0, "127.0.0.1")
	if err != OK:
		# Fallback: try a range of ports
		for port in range(49152, 49252):
			err = server.listen(port, "127.0.0.1")
			if err == OK:
				var p : int = port
				server.stop()
				return p
		return 0
	# TCPServer in Godot 4.5 does not expose the bound port when using port 0,
	# so we fall back to scanning.
	server.stop()
	for port in range(49152, 49252):
		err = server.listen(port, "127.0.0.1")
		if err == OK:
			server.stop()
			return port
	return 0


func _resolve_host_path() -> String:
	if OS.has_feature("editor"):
		return ProjectSettings.globalize_path("res://host/build/Debug/TesseraHost.exe")
	else:
		return OS.get_executable_path().get_base_dir().path_join("TesseraHost.exe")


## Send a command and receive the response, pausing health pings to prevent
## ping responses from interleaving with query responses in the TCP stream.
func _send_and_recv(cmd: Dictionary, timeout_sec: float) -> Dictionary:
	# Pause health pings so no ping response can appear in the buffer
	if _health_timer:
		_health_timer.stop()
	# Drain any stale ping responses sitting in the buffer
	_drain_stale_responses()
	_send_raw(cmd)
	var result = await _recv_raw(timeout_sec)
	# Resume health pings
	if _health_timer:
		_health_timer.start()
	return result


## Drain any buffered responses (e.g., from pings sent before we paused).
func _drain_stale_responses() -> void:
	if _tcp == null or not _connected:
		return
	_tcp.poll()
	while _tcp.get_available_bytes() >= 4:
		var length := _tcp.get_32()
		if length > 0 and length < 64 * 1024 * 1024:
			var result := _tcp.get_data(length)
			if result[0] == OK:
				print("[HostBridge] Drained stale response (%d bytes)" % length)
		_tcp.poll()


func _send_raw(cmd: Dictionary) -> void:
	if _tcp == null or not _connected:
		push_error("[HostBridge] Cannot send: not connected")
		return
	var json_str := JSON.stringify(cmd)
	var data := json_str.to_utf8_buffer()
	_tcp.put_32(data.size())   # big_endian=false → writes as LE uint32
	_tcp.put_data(data)


func _recv_raw(timeout_sec: float):
	if _tcp == null or not _connected:
		return {"status": "error", "message": "Not connected"}
	var elapsed := 0.0
	# Phase 1: wait for the 4-byte length prefix
	while elapsed < timeout_sec:
		_tcp.poll()
		if _tcp.get_status() != StreamPeerTCP.STATUS_CONNECTED:
			_on_connection_lost()
			return {"status": "error", "message": "Connection lost"}
		if _tcp.get_available_bytes() >= 4:
			break
		await get_tree().create_timer(POLL_INTERVAL).timeout
		elapsed += POLL_INTERVAL

	if _tcp.get_available_bytes() < 4:
		return {"status": "error", "message": "Timed out waiting for response header"}

	var length := _tcp.get_32()
	if length <= 0 or length > 64 * 1024 * 1024:  # sanity: max 64 MB
		return {"status": "error", "message": "Invalid response length: %d" % length}

	# Phase 2: wait for the full payload
	while elapsed < timeout_sec:
		_tcp.poll()
		if _tcp.get_status() != StreamPeerTCP.STATUS_CONNECTED:
			_on_connection_lost()
			return {"status": "error", "message": "Connection lost during payload"}
		if _tcp.get_available_bytes() >= length:
			break
		await get_tree().create_timer(POLL_INTERVAL).timeout
		elapsed += POLL_INTERVAL

	if _tcp.get_available_bytes() < length:
		return {"status": "error", "message": "Timed out waiting for response payload"}

	var result := _tcp.get_data(length)
	if result[0] != OK:
		return {"status": "error", "message": "TCP receive failed with code %d" % result[0]}

	var json_str : String = result[1].get_string_from_utf8()
	var parsed = JSON.parse_string(json_str)
	if parsed == null:
		return {"status": "error", "message": "Failed to parse JSON response"}
	return parsed


func _start_health_timer() -> void:
	_health_timer = Timer.new()
	_health_timer.wait_time = HEALTH_PING_INTERVAL
	_health_timer.autostart = true
	_health_timer.one_shot = false
	_health_timer.timeout.connect(_on_health_ping)
	add_child(_health_timer)


func _on_health_ping() -> void:
	if not _connected or _tcp == null:
		return
	_send_raw({"cmd": "ping"})
	# We don't await the response here — it will be consumed naturally.
	# If the connection is dead, the next poll in _process will detect it.


func _on_connection_lost() -> void:
	if not _connected:
		return
	_connected = false
	push_error("[HostBridge] Connection to TesseraHost lost")
	connection_lost.emit()


func _notification(what: int) -> void:
	# Clean up when the node is removed from the tree or the app is quitting
	if what == NOTIFICATION_PREDELETE or what == NOTIFICATION_WM_CLOSE_REQUEST:
		if _connected:
			# Best-effort shutdown; can't await in _notification
			if _tcp != null:
				var cmd_str := JSON.stringify({"cmd": "shutdown"})
				var data := cmd_str.to_utf8_buffer()
				_tcp.put_32(data.size())
				_tcp.put_data(data)
				_tcp.disconnect_from_host()
			_connected = false
