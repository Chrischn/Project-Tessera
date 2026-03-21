// =============================================================================
// File:              message_protocol.cpp
// Author(s):         Chrischn89
// Description:
//   JSON message protocol layer for the TesseraHost bridge. Uses yyjson for
//   parsing and serialization. Provides command dispatch: ping, shutdown, and
//   error responses for unknown commands.
//
// License:
//   Released under the terms of the GNU General Public License version 3.0
// =============================================================================
#include "message_protocol.h"
#include "tcp_server.h"

#include <yyjson.h>
#include <cstdio>
#include <cstring>
#include <string>

// ---------------------------------------------------------------------------
// parse_command
// ---------------------------------------------------------------------------
std::string parse_command(const char* json, uint32_t len) {
    yyjson_doc* doc = yyjson_read(json, static_cast<size_t>(len), 0);
    if (!doc) return {};

    yyjson_val* root = yyjson_doc_get_root(doc);
    yyjson_val* cmd_val = yyjson_obj_get(root, "cmd");

    std::string cmd;
    if (cmd_val && yyjson_is_str(cmd_val))
        cmd = yyjson_get_str(cmd_val);

    yyjson_doc_free(doc);
    return cmd;
}

// ---------------------------------------------------------------------------
// build_response
// ---------------------------------------------------------------------------
std::vector<char> build_response(const char* status, const char* json_body) {
    yyjson_mut_doc* doc = yyjson_mut_doc_new(nullptr);
    yyjson_mut_val* root = yyjson_mut_obj(doc);
    yyjson_mut_doc_set_root(doc, root);

    yyjson_mut_obj_add_str(doc, root, "status", status);

    if (json_body && json_body[0] != '\0') {
        // Parse the caller-supplied body and embed it as "data".
        yyjson_doc* body_doc = yyjson_read(json_body, strlen(json_body), 0);
        if (body_doc) {
            yyjson_val* body_root = yyjson_doc_get_root(body_doc);
            // Deep-copy into the mutable doc so the body doc can be freed.
            yyjson_mut_val* body_copy = yyjson_val_mut_copy(doc, body_root);
            if (body_copy)
                yyjson_mut_obj_add_val(doc, root, "data", body_copy);
            yyjson_doc_free(body_doc);
        }
    }

    size_t out_len = 0;
    char*  out_str = yyjson_mut_write(doc, 0, &out_len);
    yyjson_mut_doc_free(doc);

    if (!out_str) return {};

    std::vector<char> result(out_str, out_str + out_len);
    free(out_str);
    return result;
}

// ---------------------------------------------------------------------------
// build_error
// ---------------------------------------------------------------------------
std::vector<char> build_error(const char* message) {
    yyjson_mut_doc* doc = yyjson_mut_doc_new(nullptr);
    yyjson_mut_val* root = yyjson_mut_obj(doc);
    yyjson_mut_doc_set_root(doc, root);

    yyjson_mut_obj_add_str(doc, root, "status",  "error");
    yyjson_mut_obj_add_str(doc, root, "message", message);

    size_t out_len = 0;
    char*  out_str = yyjson_mut_write(doc, 0, &out_len);
    yyjson_mut_doc_free(doc);

    if (!out_str) return {};

    std::vector<char> result(out_str, out_str + out_len);
    free(out_str);
    return result;
}

// ---------------------------------------------------------------------------
// dispatch_command
// ---------------------------------------------------------------------------
void dispatch_command(const char* json, uint32_t len,
                      TcpServer& server, bool* shutdown_flag) {
    std::string cmd = parse_command(json, len);

    if (cmd.empty()) {
        auto err = build_error("invalid JSON or missing 'cmd' field");
        server.send_message(err.data(), static_cast<uint32_t>(err.size()));
        return;
    }

    printf("[Protocol] cmd = \"%s\"\n", cmd.c_str());

    if (cmd == "ping") {
        auto resp = build_response("ok");
        server.send_message(resp.data(), static_cast<uint32_t>(resp.size()));

    } else if (cmd == "shutdown") {
        auto resp = build_response("ok");
        server.send_message(resp.data(), static_cast<uint32_t>(resp.size()));
        if (shutdown_flag) *shutdown_flag = true;

    } else {
        std::string msg = "unknown command: " + cmd;
        auto err = build_error(msg.c_str());
        server.send_message(err.data(), static_cast<uint32_t>(err.size()));
    }
}
