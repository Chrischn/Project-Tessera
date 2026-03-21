// =============================================================================
// File:              message_protocol.cpp
// Author(s):         Chrischn89
// Description:
//   JSON message protocol layer for the TesseraHost bridge. Uses yyjson for
//   parsing and serialization. Provides command dispatch: ping, shutdown, init,
//   and error responses for unknown commands.
//
// License:
//   Released under the terms of the GNU General Public License version 3.0
// =============================================================================
#include "message_protocol.h"
#include "tcp_server.h"
#include "dll_loader.h"

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
// Global base_path — set before DLL init so host callbacks can use it.
// The host_callbacks.cpp reads this for file enumeration.
// ---------------------------------------------------------------------------
std::string g_basePath;
std::string g_modName;

// ---------------------------------------------------------------------------
// resolve_dll_path — build path to CvGameCoreDLL.dll from base_path and mod
// ---------------------------------------------------------------------------
static std::string resolve_dll_path(const std::string& base_path,
                                    const std::string& mod) {
    // Normalize: ensure no trailing slash on base_path
    std::string base = base_path;
    while (!base.empty() && (base.back() == '/' || base.back() == '\\'))
        base.pop_back();

    if (!mod.empty()) {
        // Mod DLL: try mod Assets/ first
        std::string mod_path = base + "/Beyond the Sword/Mods/" + mod
                             + "/Assets/CvGameCoreDLL.dll";
        DWORD attr = GetFileAttributesA(mod_path.c_str());
        if (attr != INVALID_FILE_ATTRIBUTES) {
            fprintf(stderr, "[DLL] Using mod DLL: %s\n", mod_path.c_str());
            return mod_path;
        }
        fprintf(stderr, "[DLL] Mod DLL not found, falling back to base: %s\n",
                mod_path.c_str());
    }

    // Base BTS DLL
    return base + "/Beyond the Sword/Assets/CvGameCoreDLL.dll";
}

// ---------------------------------------------------------------------------
// handle_init — load DLL, wire interfaces, call init
// ---------------------------------------------------------------------------
static void handle_init(const char* json, uint32_t len,
                        TcpServer& server, DllLoader& dll_loader) {
    // Parse base_path and mod from the JSON
    yyjson_doc* doc = yyjson_read(json, static_cast<size_t>(len), 0);
    if (!doc) {
        auto err = build_error("init: invalid JSON");
        server.send_message(err.data(), static_cast<uint32_t>(err.size()));
        return;
    }

    yyjson_val* root = yyjson_doc_get_root(doc);
    yyjson_val* base_val = yyjson_obj_get(root, "base_path");
    yyjson_val* mod_val  = yyjson_obj_get(root, "mod");

    if (!base_val || !yyjson_is_str(base_val)) {
        yyjson_doc_free(doc);
        auto err = build_error("init: missing or invalid 'base_path' field");
        server.send_message(err.data(), static_cast<uint32_t>(err.size()));
        return;
    }

    std::string base_path = yyjson_get_str(base_val);
    std::string mod;
    if (mod_val && yyjson_is_str(mod_val))
        mod = yyjson_get_str(mod_val);

    yyjson_doc_free(doc);

    // Resolve DLL path
    std::string dll_path = resolve_dll_path(base_path, mod);
    fprintf(stderr, "[Protocol] init: dll_path = %s\n", dll_path.c_str());

    // Set global base_path and mod name BEFORE DLL init so the utility
    // interface can use them for file enumeration during XML loading.
    g_basePath = base_path;
    g_modName = mod;
    fprintf(stderr, "[Protocol] init: g_basePath = %s\n", g_basePath.c_str());
    fprintf(stderr, "[Protocol] init: g_modName = %s\n", g_modName.c_str());

    // Step 1a: Load TesseraRelay.dll (must happen before game DLL)
    // Resolve relay path: same directory as TesseraHost.exe
    char exe_path[MAX_PATH];
    GetModuleFileNameA(NULL, exe_path, MAX_PATH);
    std::string exe_dir(exe_path);
    exe_dir = exe_dir.substr(0, exe_dir.find_last_of("/\\"));
    std::string relay_path = exe_dir + "\\TesseraRelay.dll";

    if (!dll_loader.load_relay(relay_path)) {
        auto err = build_error("init: failed to load TesseraRelay.dll");
        server.send_message(err.data(), static_cast<uint32_t>(err.size()));
        return;
    }

    // Step 1b: Load the game DLL and resolve exports
    if (!dll_loader.load(dll_path)) {
        auto err = build_error("init: failed to load CvGameCoreDLL.dll");
        server.send_message(err.data(), static_cast<uint32_t>(err.size()));
        return;
    }

    // Step 2: Wire relay's utility interface into the DLL's gDLL pointer
    if (!dll_loader.wire_interfaces()) {
        auto err = build_error("init: failed to wire interfaces");
        server.send_message(err.data(), static_cast<uint32_t>(err.size()));
        dll_loader.unload();
        return;
    }

    // Step 3: Call CvGlobals::init() (may crash — SEH protected)
    bool init_ok = dll_loader.initialize();
    if (!init_ok) {
        // init() crashed but we still got past loading and wiring.
        // Report partial success so the caller knows the DLL loaded.
        auto err = build_error("init: CvGlobals::init() crashed (SEH caught)");
        server.send_message(err.data(), static_cast<uint32_t>(err.size()));
        return;
    }

    // Step 4: Orchestrate XML data loading via CvXMLLoadUtility.
    // This creates a CvXMLLoadUtility instance and calls all loading
    // methods in the correct sequence. Expected to fail/crash on first
    // attempt — we log everything for debugging in Tasks 8-9.
    fprintf(stderr, "[Protocol] init: starting XML data loading...\n");
    bool xml_ok = dll_loader.load_xml_data();
    if (!xml_ok) {
        fprintf(stderr, "[Protocol] init: XML loading failed (expected at this stage)\n");
        // Non-fatal: CvGlobals::init() succeeded, XML loading is incremental
    } else {
        fprintf(stderr, "[Protocol] init: XML loading completed successfully\n");
    }

    // Report success (even if XML loading partially failed — init() worked)
    auto resp = build_response("ok");
    server.send_message(resp.data(), static_cast<uint32_t>(resp.size()));
}

// ---------------------------------------------------------------------------
// dispatch_command
// ---------------------------------------------------------------------------
void dispatch_command(const char* json, uint32_t len,
                      TcpServer& server, bool* shutdown_flag,
                      DllLoader& dll_loader) {
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

    } else if (cmd == "init") {
        handle_init(json, len, server, dll_loader);

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
