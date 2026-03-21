// =============================================================================
// File:              message_protocol.h
// Author(s):         Chrischn89
// Description:
//   JSON message protocol layer for the TesseraHost bridge. Uses yyjson for
//   parsing and serialization. Provides command dispatch: ping, shutdown, and
//   error responses for unknown commands.
//
// License:
//   Released under the terms of the GNU General Public License version 3.0
// =============================================================================
#pragma once

#include <vector>
#include <string>
#include <cstdint>

class TcpServer;

// Parse the "cmd" field from a JSON payload.
// Returns an empty string if the field is absent or the JSON is invalid.
std::string parse_command(const char* json, uint32_t len);

// Build a response payload.
//   status    — value for the "status" field (e.g., "ok")
//   json_body — optional raw JSON object string merged in as "data" (may be nullptr)
// Returns the serialized JSON as a vector of chars (not null-terminated).
std::vector<char> build_response(const char* status, const char* json_body = nullptr);

// Build an error response: {"status":"error","message":"<message>"}.
std::vector<char> build_error(const char* message);

// Parse cmd from json, route to handler, send response via server.
// Sets *shutdown_flag = true when a "shutdown" command is received.
void dispatch_command(const char* json, uint32_t len,
                      TcpServer& server, bool* shutdown_flag);
