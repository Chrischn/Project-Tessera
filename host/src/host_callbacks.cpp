// =============================================================================
// File:              host_callbacks.cpp
// Author(s):         Chrischn89
// Description:
//   C callback implementations for the HostCallbacks table. Each function
//   is called by the VS2003 relay via function pointer. All parameters
//   and return values are C-compatible types — no STL crosses this boundary.
//
// License:
//   Released under the terms of the GNU General Public License version 3.0
// =============================================================================

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include "host_callbacks.h"

#include <pugixml.hpp>

#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>

// ---------------------------------------------------------------------------
// Global base path and mod name — set by message_protocol.cpp before DLL init
// ---------------------------------------------------------------------------
extern std::string g_basePath;
extern std::string g_modName;

// =============================================================================
// FXml / FXmlSchemaCache — pugixml wrappers (moved from old iface_xml.cpp)
// =============================================================================

struct FXml {
    pugi::xml_document doc;
    pugi::xml_node current_node;
    bool loaded = false;
};

struct FXmlSchemaCache {
    int dummy = 0;
};

// =============================================================================
// Debug: global call counter to identify last callback before crash
// =============================================================================
int g_cbCallCount = 0;
const char* g_cbLastName = "";
const char* g_cbLastXmlFile = "";

static void cb_trace(const char* name) {
    g_cbCallCount++;
    g_cbLastName = name;
}

// =============================================================================
// XML Callback Implementations
// =============================================================================

static void* cb_xml_create(void* schema_cache) {
    cb_trace("cb_xml_create");
    return new FXml();
}

static void cb_xml_destroy(void* xml_ptr) {
    cb_trace("cb_xml_destroy");
    delete static_cast<FXml*>(xml_ptr);
}

static void* cb_xml_create_schema_cache() {
    cb_trace("cb_xml_create_schema_cache");
    return new FXmlSchemaCache();
}

static void cb_xml_destroy_schema_cache(void* cache) {
    cb_trace("cb_xml_destroy_schema_cache");
    delete static_cast<FXmlSchemaCache*>(cache);
}

// Strip leading "Assets//" or "Assets\" prefix from game DLL paths.
// The game DLL passes paths like "Assets//xml\Foo.xml" — we need just "xml\Foo.xml"
// to prepend our own directory roots.
static std::string strip_assets_prefix(const char* path) {
    std::string p(path);
    // Strip "Assets//" or "Assets\" or "Assets/"
    if (p.size() > 7 && (_strnicmp(p.c_str(), "Assets//", 8) == 0
                      || _strnicmp(p.c_str(), "Assets\\", 7) == 0
                      || _strnicmp(p.c_str(), "Assets/", 7) == 0)) {
        size_t start = p.find_first_not_of("/\\", 6); // skip "Assets" + slashes
        if (start != std::string::npos)
            return p.substr(start);
    }
    return p;
}

// Path resolution: try mod Assets, BTS Assets, Warlords Assets, then base.
// Mirrors the original game's XML search order: BtS overrides Warlords overrides vanilla.
static std::string resolve_xml_path(const char* path) {
    if (!path || !path[0]) return {};

    // Already absolute?
    if (path[1] == ':' || path[0] == '\\' || path[0] == '/') {
        DWORD attr = GetFileAttributesA(path);
        if (attr != INVALID_FILE_ATTRIBUTES) return path;
    }

    // Strip "Assets//" prefix — game DLL includes it, but we prepend our own root
    std::string relpath = strip_assets_prefix(path);

    // Try mod Assets first
    if (!g_modName.empty()) {
        std::string mod_path = g_basePath + "\\Beyond the Sword\\Mods\\"
            + g_modName + "\\Assets\\" + relpath;
        DWORD attr = GetFileAttributesA(mod_path.c_str());
        if (attr != INVALID_FILE_ATTRIBUTES) return mod_path;
    }

    // BTS Assets
    std::string bts_assets = g_basePath + "\\Beyond the Sword\\Assets\\" + relpath;
    DWORD attr = GetFileAttributesA(bts_assets.c_str());
    if (attr != INVALID_FILE_ATTRIBUTES) return bts_assets;

    // Warlords Assets (BtS inherits content from Warlords — e.g. trait definitions)
    std::string war_assets = g_basePath + "\\Warlords\\Assets\\" + relpath;
    attr = GetFileAttributesA(war_assets.c_str());
    if (attr != INVALID_FILE_ATTRIBUTES) return war_assets;

    // Base install (vanilla)
    std::string base = g_basePath + "\\Assets\\" + relpath;
    attr = GetFileAttributesA(base.c_str());
    if (attr != INVALID_FILE_ATTRIBUTES) return base;

    return bts_assets;
}

static int cb_xml_load(void* xml_ptr, const char* path) {
    cb_trace("cb_xml_load");
    g_cbLastXmlFile = path ? path : "";
    FXml* xml = static_cast<FXml*>(xml_ptr);
    std::string resolved = resolve_xml_path(path);
    fprintf(stderr, "[XML] Loading: %s\n", resolved.c_str());

    pugi::xml_parse_result result = xml->doc.load_file(resolved.c_str());
    if (!result) {
        fprintf(stderr, "[XML ERROR] Failed to load %s: %s\n",
            resolved.c_str(), result.description());
        xml->loaded = false;
        return 0;
    }
    xml->current_node = xml->doc.document_element();
    xml->loaded = true;
    return 1;
}

static int cb_xml_locate_node(void* xml_ptr, const char* path) {
    cb_trace("cb_xml_locate_node");
    FXml* xml = static_cast<FXml*>(xml_ptr);
    if (!xml->loaded || !path) return 0;

    pugi::xml_node node = xml->doc;
    std::string spath(path);
    size_t start = 0;
    while (start < spath.size()) {
        size_t sep = spath.find('/', start);
        if (sep == std::string::npos) sep = spath.size();
        std::string segment = spath.substr(start, sep - start);
        if (!segment.empty()) {
            node = node.child(segment.c_str());
            if (!node) return 0;
        }
        start = sep + 1;
    }
    xml->current_node = node;
    return 1;
}

static int cb_xml_set_to_child(void* xml_ptr) {
    cb_trace("cb_xml_set_to_child");
    FXml* xml = static_cast<FXml*>(xml_ptr);
    pugi::xml_node child = xml->current_node.first_child();
    while (child && child.type() != pugi::node_element)
        child = child.next_sibling();
    if (!child) return 0;
    xml->current_node = child;
    return 1;
}

static int cb_xml_set_to_child_by_tag(void* xml_ptr, const char* tag) {
    cb_trace("cb_xml_set_to_child_by_tag");
    if (g_cbCallCount == 285872) {
        fprintf(stderr, "[CRASH_POINT] SetToChildByTagName xml=%p tag='%s' node='%s'\n",
            xml_ptr, tag ? tag : "(null)",
            xml_ptr ? static_cast<FXml*>(xml_ptr)->current_node.name() : "BAD_PTR");
        fflush(stderr);
    }
    FXml* xml = static_cast<FXml*>(xml_ptr);
    pugi::xml_node child = xml->current_node.child(tag);
    if (!child) return 0;
    xml->current_node = child;
    return 1;
}

static int cb_xml_set_to_parent(void* xml_ptr) {
    cb_trace("cb_xml_set_to_parent");
    FXml* xml = static_cast<FXml*>(xml_ptr);
    pugi::xml_node parent = xml->current_node.parent();
    if (!parent) return 0;
    xml->current_node = parent;
    return 1;
}

static int cb_xml_next_sibling(void* xml_ptr) {
    cb_trace("cb_xml_next_sibling");
    FXml* xml = static_cast<FXml*>(xml_ptr);
    pugi::xml_node sib = xml->current_node.next_sibling();
    while (sib && sib.type() != pugi::node_element)
        sib = sib.next_sibling();
    if (!sib) return 0;
    xml->current_node = sib;
    return 1;
}

static int cb_xml_prev_sibling(void* xml_ptr) {
    cb_trace("cb_xml_prev_sibling");
    FXml* xml = static_cast<FXml*>(xml_ptr);
    pugi::xml_node sib = xml->current_node.previous_sibling();
    while (sib && sib.type() != pugi::node_element)
        sib = sib.previous_sibling();
    if (!sib) return 0;
    xml->current_node = sib;
    return 1;
}

static int cb_xml_locate_first_sibling_by_tag(void* xml_ptr, const char* tag) {
    cb_trace("cb_xml_locate_first_sibling_by_tag");
    FXml* xml = static_cast<FXml*>(xml_ptr);
    pugi::xml_node parent = xml->current_node.parent();
    if (!parent) return 0;
    pugi::xml_node sib = parent.child(tag);
    if (!sib) return 0;
    xml->current_node = sib;
    return 1;
}

static int cb_xml_locate_next_sibling_by_tag(void* xml_ptr, const char* tag) {
    cb_trace("cb_xml_locate_next_sibling_by_tag");
    FXml* xml = static_cast<FXml*>(xml_ptr);
    pugi::xml_node sib = xml->current_node.next_sibling(tag);
    if (!sib) return 0;
    xml->current_node = sib;
    return 1;
}

// Helper: get text content from current node
static std::string get_node_text(FXml* xml) {
    for (pugi::xml_node child = xml->current_node.first_child(); child;
         child = child.next_sibling()) {
        if (child.type() == pugi::node_pcdata || child.type() == pugi::node_cdata)
            return child.value();
    }
    const char* txt = xml->current_node.text().get();
    return txt ? txt : "";
}

static int cb_xml_get_value_string(void* xml_ptr, char* buf, int buf_size) {
    cb_trace("cb_xml_get_value_string");
    FXml* xml = static_cast<FXml*>(xml_ptr);
    std::string text = get_node_text(xml);
    if (text.empty() && !xml->current_node.first_child()) return 0;
    strncpy(buf, text.c_str(), buf_size - 1);
    buf[buf_size - 1] = '\0';
    return 1;
}

static int cb_xml_get_value_wstring(void* xml_ptr, wchar_t* buf, int buf_size) {
    cb_trace("cb_xml_get_value_wstring");
    FXml* xml = static_cast<FXml*>(xml_ptr);
    std::string text = get_node_text(xml);
    int needed = MultiByteToWideChar(CP_UTF8, 0, text.c_str(), -1, NULL, 0);
    if (needed <= 0 || needed > buf_size) {
        if (buf_size > 0) buf[0] = L'\0';
        return 0;
    }
    MultiByteToWideChar(CP_UTF8, 0, text.c_str(), -1, buf, buf_size);
    return 1;
}

static int cb_xml_get_value_int(void* xml_ptr, int* val) {
    cb_trace("cb_xml_get_value_int");
    FXml* xml = static_cast<FXml*>(xml_ptr);
    std::string text = get_node_text(xml);
    if (text.empty()) return 0;
    *val = atoi(text.c_str());
    return 1;
}

static int cb_xml_get_value_float(void* xml_ptr, float* val) {
    cb_trace("cb_xml_get_value_float");
    FXml* xml = static_cast<FXml*>(xml_ptr);
    std::string text = get_node_text(xml);
    if (text.empty()) return 0;
    *val = static_cast<float>(atof(text.c_str()));
    return 1;
}

static int cb_xml_get_value_bool(void* xml_ptr, int* val) {
    cb_trace("cb_xml_get_value_bool");
    // Log details near crash point
    if (g_cbCallCount >= 107640 && g_cbCallCount <= 107650) {
        fprintf(stderr, "[CB #%d] get_value_bool xml=%p val=%p node='%s'\n",
            g_cbCallCount, xml_ptr, (void*)val,
            xml_ptr ? static_cast<FXml*>(xml_ptr)->current_node.name() : "NULL");
        fflush(stderr);
    }
    FXml* xml = static_cast<FXml*>(xml_ptr);
    std::string text = get_node_text(xml);
    if (text.empty()) return 0;
    *val = (text == "1" || text == "true" || text == "True") ? 1 : 0;
    return 1;
}

static int cb_xml_get_value_uint(void* xml_ptr, unsigned int* val) {
    cb_trace("cb_xml_get_value_uint");
    FXml* xml = static_cast<FXml*>(xml_ptr);
    std::string text = get_node_text(xml);
    if (text.empty()) return 0;
    *val = static_cast<unsigned int>(strtoul(text.c_str(), NULL, 10));
    return 1;
}

static int cb_xml_get_last_node_text(void* xml_ptr, char* buf, int buf_size) {
    cb_trace("cb_xml_get_last_node_text");
    return cb_xml_get_value_string(xml_ptr, buf, buf_size);
}

static int cb_xml_get_last_node_text_size(void* xml_ptr) {
    cb_trace("cb_xml_get_last_node_text_size");
    FXml* xml = static_cast<FXml*>(xml_ptr);
    std::string text = get_node_text(xml);
    return static_cast<int>(text.size());
}

static int cb_xml_get_tag_name(void* xml_ptr, char* buf, int buf_size) {
    cb_trace("cb_xml_get_tag_name");
    FXml* xml = static_cast<FXml*>(xml_ptr);
    const char* name = xml->current_node.name();
    if (!name || !name[0]) return 0;
    strncpy(buf, name, buf_size - 1);
    buf[buf_size - 1] = '\0';
    return 1;
}

static int cb_xml_get_num_children(void* xml_ptr) {
    cb_trace("cb_xml_get_num_children");
    FXml* xml = static_cast<FXml*>(xml_ptr);
    int count = 0;
    for (pugi::xml_node child = xml->current_node.first_child(); child;
         child = child.next_sibling()) {
        if (child.type() == pugi::node_element) count++;
    }
    return count;
}

static int cb_xml_get_num_siblings(void* xml_ptr) {
    cb_trace("cb_xml_get_num_siblings");
    FXml* xml = static_cast<FXml*>(xml_ptr);
    pugi::xml_node parent = xml->current_node.parent();
    if (!parent) return 0;
    int count = 0;
    for (pugi::xml_node sib = parent.first_child(); sib;
         sib = sib.next_sibling()) {
        if (sib.type() == pugi::node_element) count++;
    }
    return count;
}

static int cb_xml_num_children_by_tag(void* xml_ptr, const char* tag) {
    cb_trace("cb_xml_num_children_by_tag");
    FXml* xml = static_cast<FXml*>(xml_ptr);
    int count = 0;
    for (pugi::xml_node child = xml->current_node.first_child(); child;
         child = child.next_sibling()) {
        if (child.type() == pugi::node_element && strcmp(child.name(), tag) == 0)
            count++;
    }
    return count;
}

static int cb_xml_num_elements_by_tag(void* xml_ptr, const char* tag) {
    cb_trace("cb_xml_num_elements_by_tag");
    FXml* xml = static_cast<FXml*>(xml_ptr);

    // The game DLL passes full paths like "Civ4Types/FlavorTypes/FlavorType".
    // After LocateNode navigated to the first matching element, we need to
    // count all sibling elements with the same tag name (last path component).
    const char* simple_tag = tag;
    const char* last_slash = strrchr(tag, '/');
    if (last_slash) simple_tag = last_slash + 1;

    // Count siblings (including current) in parent with matching tag
    pugi::xml_node parent = xml->current_node.parent();
    if (!parent) return 0;
    int count = 0;
    for (pugi::xml_node sib = parent.first_child(); sib;
         sib = sib.next_sibling()) {
        if (sib.type() == pugi::node_element && strcmp(sib.name(), simple_tag) == 0)
            count++;
    }
    return count;
}

static int cb_xml_is_comment_node(void* xml_ptr) {
    cb_trace("cb_xml_is_comment_node");
    FXml* xml = static_cast<FXml*>(xml_ptr);
    return (xml->current_node.type() == pugi::node_comment) ? 1 : 0;
}

static int cb_xml_is_allow_caching() {
    cb_trace("cb_xml_is_allow_caching");
    return 0;
}

static void cb_xml_map_children(void* xml_ptr) {
    cb_trace("cb_xml_map_children");
    // No-op for MVP
}

// =============================================================================
// Utility Callback Implementations
// =============================================================================

static void cb_log_msg(const char* filename, const char* msg) {
    fprintf(stderr, "[DLL LOG] %s: %s\n", filename ? filename : "", msg ? msg : "");
}

static int cb_get_text(const wchar_t* tag, wchar_t* buf, int buf_size) {
    cb_trace("cb_get_text");
    if (!tag || buf_size <= 0) return 0;
    int i = 0;
    while (tag[i] && i < buf_size - 1) { buf[i] = tag[i]; i++; }
    buf[i] = L'\0';
    return 1;
}

static int cb_enumerate_files(const char* pattern, const char*** out_paths) {
    cb_trace("cb_enumerate_files");
    *out_paths = NULL;
    if (!pattern || !pattern[0]) return 0;

    std::vector<std::string> found;

    auto enumerate_in_dir = [&](const std::string& base_dir) {
        std::string full_pattern = base_dir;
        if (!full_pattern.empty() && full_pattern.back() != '\\')
            full_pattern += '\\';
        full_pattern += pattern;

        WIN32_FIND_DATAA fd;
        HANDLE hFind = FindFirstFileA(full_pattern.c_str(), &fd);
        if (hFind == INVALID_HANDLE_VALUE) return;

        std::string dir_prefix(pattern);
        size_t last_sep = dir_prefix.find_last_of("\\/");
        if (last_sep != std::string::npos)
            dir_prefix = dir_prefix.substr(0, last_sep + 1);
        else
            dir_prefix.clear();

        do {
            if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
                found.push_back(dir_prefix + fd.cFileName);
        } while (FindNextFileA(hFind, &fd));
        FindClose(hFind);
    };

    if (!g_modName.empty()) {
        std::string mod_dir = g_basePath + "\\Beyond the Sword\\Mods\\"
            + g_modName + "\\Assets";
        enumerate_in_dir(mod_dir);
    }
    std::string bts_dir = g_basePath + "\\Beyond the Sword\\Assets";
    enumerate_in_dir(bts_dir);

    if (found.empty()) return 0;

    int count = static_cast<int>(found.size());
    const char** paths = static_cast<const char**>(
        malloc(count * sizeof(const char*)));
    for (int i = 0; i < count; i++) {
        char* s = static_cast<char*>(malloc(found[i].size() + 1));
        strcpy(s, found[i].c_str());
        paths[i] = s;
    }

    *out_paths = paths;
    return count;
}

static void cb_free_file_list(const char** paths, int count) {
    cb_trace("cb_free_file_list");
    if (!paths) return;
    for (int i = 0; i < count; i++)
        free(const_cast<char*>(paths[i]));
    free(const_cast<char**>(paths));
}

static void enumerate_recursive(std::vector<std::string>& files,
                                 const std::string& dir,
                                 const std::string& ext,
                                 bool subdirs) {
    std::string search = dir + "\\*";
    WIN32_FIND_DATAA fd;
    HANDLE hFind = FindFirstFileA(search.c_str(), &fd);
    if (hFind == INVALID_HANDLE_VALUE) return;

    do {
        if (fd.cFileName[0] == '.') continue;
        std::string full = dir + "\\" + fd.cFileName;

        if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            if (subdirs) enumerate_recursive(files, full, ext, subdirs);
        } else {
            std::string fname(fd.cFileName);
            if (fname.size() >= ext.size()) {
                std::string fext = fname.substr(fname.size() - ext.size());
                std::string ext_l = ext, fext_l = fext;
                std::transform(ext_l.begin(), ext_l.end(), ext_l.begin(), ::tolower);
                std::transform(fext_l.begin(), fext_l.end(), fext_l.begin(), ::tolower);
                if (ext_l == fext_l) files.push_back(full);
            }
        }
    } while (FindNextFileA(hFind, &fd));
    FindClose(hFind);
}

static int cb_enumerate_module_files(const char* root_dir, const char* mod_dir,
                                      const char* extension, int search_subdirs,
                                      const char*** out_paths) {
    cb_trace("cb_enumerate_module_files");
    *out_paths = NULL;
    std::vector<std::string> found;

    std::string full_dir;
    if (!g_modName.empty()) {
        full_dir = g_basePath + "\\Beyond the Sword\\Mods\\" + g_modName
            + "\\Assets\\" + mod_dir;
    } else {
        full_dir = g_basePath + "\\Beyond the Sword\\Assets\\" + mod_dir;
    }

    enumerate_recursive(found, full_dir, extension ? extension : "",
                        search_subdirs != 0);

    if (found.empty()) return 0;

    int count = static_cast<int>(found.size());
    const char** paths = static_cast<const char**>(
        malloc(count * sizeof(const char*)));
    for (int i = 0; i < count; i++) {
        char* s = static_cast<char*>(malloc(found[i].size() + 1));
        strcpy(s, found[i].c_str());
        paths[i] = s;
    }

    *out_paths = paths;
    return count;
}

static const char* cb_get_mod_name(int full_path) {
    if (g_modName.empty()) return "";
    if (full_path) {
        static std::string s_fullModPath;
        s_fullModPath = "Mods\\" + g_modName + "\\";
        return s_fullModPath.c_str();
    }
    return g_modName.c_str();
}

static int cb_get_current_language() { cb_trace("cb_get_current_language"); return 0; }
static int cb_is_modular_xml_loading() { cb_trace("cb_is_modular_xml_loading"); return 0; }
static int cb_is_game_initializing() { cb_trace("cb_is_game_initializing"); return 1; }
static int cb_get_cheat_level() { cb_trace("cb_get_cheat_level"); return 0; }
static int cb_get_audio_tag_index(const char* tag, int script_type) { cb_trace("cb_get_audio_tag_index"); return -1; }

static void* cb_create_cache_object(const char* name) { cb_trace("cb_create_cache_object"); return NULL; }
static int cb_cache_read(void* cache, const char* file) { cb_trace("cb_cache_read"); return 0; }
static int cb_cache_write(void* cache) { cb_trace("cb_cache_write"); return 0; }
static void cb_destroy_cache(void* cache) { cb_trace("cb_destroy_cache"); }

static void* cb_host_malloc(size_t size) { cb_trace("cb_host_malloc"); return malloc(size); }
static void cb_host_free(void* ptr) { cb_trace("cb_host_free"); free(ptr); }

// =============================================================================
// Factory function
// =============================================================================

HostCallbacks create_host_callbacks() {
    HostCallbacks cb;
    memset(&cb, 0, sizeof(cb));
    cb.version = RELAY_API_VERSION;

    // XML
    cb.xml_create = cb_xml_create;
    cb.xml_destroy = cb_xml_destroy;
    cb.xml_create_schema_cache = cb_xml_create_schema_cache;
    cb.xml_destroy_schema_cache = cb_xml_destroy_schema_cache;
    cb.xml_load = cb_xml_load;
    cb.xml_locate_node = cb_xml_locate_node;
    cb.xml_set_to_child = cb_xml_set_to_child;
    cb.xml_set_to_child_by_tag = cb_xml_set_to_child_by_tag;
    cb.xml_set_to_parent = cb_xml_set_to_parent;
    cb.xml_next_sibling = cb_xml_next_sibling;
    cb.xml_prev_sibling = cb_xml_prev_sibling;
    cb.xml_locate_first_sibling_by_tag = cb_xml_locate_first_sibling_by_tag;
    cb.xml_locate_next_sibling_by_tag = cb_xml_locate_next_sibling_by_tag;
    cb.xml_get_value_string = cb_xml_get_value_string;
    cb.xml_get_value_wstring = cb_xml_get_value_wstring;
    cb.xml_get_value_int = cb_xml_get_value_int;
    cb.xml_get_value_float = cb_xml_get_value_float;
    cb.xml_get_value_bool = cb_xml_get_value_bool;
    cb.xml_get_value_uint = cb_xml_get_value_uint;
    cb.xml_get_last_node_text = cb_xml_get_last_node_text;
    cb.xml_get_last_node_text_size = cb_xml_get_last_node_text_size;
    cb.xml_get_tag_name = cb_xml_get_tag_name;
    cb.xml_get_num_children = cb_xml_get_num_children;
    cb.xml_get_num_siblings = cb_xml_get_num_siblings;
    cb.xml_num_children_by_tag = cb_xml_num_children_by_tag;
    cb.xml_num_elements_by_tag = cb_xml_num_elements_by_tag;
    cb.xml_is_comment_node = cb_xml_is_comment_node;
    cb.xml_is_allow_caching = cb_xml_is_allow_caching;
    cb.xml_map_children = cb_xml_map_children;

    // Utility
    cb.get_text = cb_get_text;
    cb.log_msg = cb_log_msg;
    cb.enumerate_files = cb_enumerate_files;
    cb.free_file_list = cb_free_file_list;
    cb.enumerate_module_files = cb_enumerate_module_files;
    cb.get_mod_name = cb_get_mod_name;
    cb.get_current_language = cb_get_current_language;
    cb.is_modular_xml_loading = cb_is_modular_xml_loading;
    cb.is_game_initializing = cb_is_game_initializing;
    cb.get_cheat_level = cb_get_cheat_level;
    cb.get_audio_tag_index = cb_get_audio_tag_index;
    cb.create_cache_object = cb_create_cache_object;
    cb.cache_read = cb_cache_read;
    cb.cache_write = cb_cache_write;
    cb.destroy_cache = cb_destroy_cache;
    cb.host_malloc = cb_host_malloc;
    cb.host_free = cb_host_free;

    return cb;
}
