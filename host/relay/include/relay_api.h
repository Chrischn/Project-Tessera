// =============================================================================
// File:              relay_api.h
// Author(s):         Chrischn89
// Description:
//   Shared C header defining the HostCallbacks function pointer table and
//   relay DLL exports. Included by both the VS2003 relay and VS2022 host.
//   MUST remain valid C89/C99 — no C++ types allowed in the struct.
//
// License:
//   Released under the terms of the GNU General Public License version 3.0
// =============================================================================
#ifndef TESSERA_RELAY_API_H
#define TESSERA_RELAY_API_H

#include <stddef.h>  /* size_t */

#define RELAY_API_VERSION 1

// HostCallbacks — function pointer table passed from host to relay.
// Every pointer is a C function implemented by the VS2022 host.
// The relay calls these to access pugixml, file enumeration, etc.
typedef struct HostCallbacks {
    int version;  /* Must be RELAY_API_VERSION. Checked by relay_init(). */

    /* === XML interface callbacks === */
    void* (*xml_create)(void* schema_cache);
    void  (*xml_destroy)(void* xml);
    void* (*xml_create_schema_cache)(void);
    void  (*xml_destroy_schema_cache)(void* cache);
    int   (*xml_load)(void* xml, const char* path);
    int   (*xml_locate_node)(void* xml, const char* path);
    int   (*xml_set_to_child)(void* xml);
    int   (*xml_set_to_child_by_tag)(void* xml, const char* tag);
    int   (*xml_set_to_parent)(void* xml);
    int   (*xml_next_sibling)(void* xml);
    int   (*xml_prev_sibling)(void* xml);
    int   (*xml_locate_first_sibling_by_tag)(void* xml, const char* tag);
    int   (*xml_locate_next_sibling_by_tag)(void* xml, const char* tag);

    /* Value readers — write into caller-provided C buffers */
    int   (*xml_get_value_string)(void* xml, char* buf, int buf_size);
    int   (*xml_get_value_wstring)(void* xml, wchar_t* buf, int buf_size);
    int   (*xml_get_value_int)(void* xml, int* val);
    int   (*xml_get_value_float)(void* xml, float* val);
    int   (*xml_get_value_bool)(void* xml, int* val);
    int   (*xml_get_value_uint)(void* xml, unsigned int* val);
    int   (*xml_get_last_node_text)(void* xml, char* buf, int buf_size);
    int   (*xml_get_last_node_text_size)(void* xml);
    int   (*xml_get_tag_name)(void* xml, char* buf, int buf_size);

    /* Counting / query */
    int   (*xml_get_num_children)(void* xml);
    int   (*xml_get_num_siblings)(void* xml);
    int   (*xml_num_children_by_tag)(void* xml, const char* tag);
    int   (*xml_num_elements_by_tag)(void* xml, const char* tag);
    int   (*xml_is_comment_node)(void* xml);
    int   (*xml_is_allow_caching)(void);
    void  (*xml_map_children)(void* xml);

    /* === Utility interface callbacks === */
    void  (*log_msg)(const char* filename, const char* msg);

    /* Text / localization */
    int   (*get_text)(const wchar_t* tag, wchar_t* buf, int buf_size);

    /* File enumeration */
    int   (*enumerate_files)(const char* pattern, const char*** out_paths);
    void  (*free_file_list)(const char** paths, int count);
    int   (*enumerate_module_files)(const char* root_dir, const char* mod_dir,
              const char* extension, int search_subdirs, const char*** out_paths);

    /* Game state queries */
    const char* (*get_mod_name)(int full_path);
    int   (*get_current_language)(void);
    int   (*is_modular_xml_loading)(void);
    int   (*is_game_initializing)(void);
    int   (*get_cheat_level)(void);
    int   (*get_audio_tag_index)(const char* tag, int script_type);

    /* Cache */
    void* (*create_cache_object)(const char* cache_name);
    int   (*cache_read)(void* cache, const char* source_file);
    int   (*cache_write)(void* cache);
    void  (*destroy_cache)(void* cache);

    /* Host-side memory (for host-allocated buffers passed to relay) */
    void* (*host_malloc)(size_t size);
    void  (*host_free)(void* ptr);

} HostCallbacks;

/* Relay DLL exports — resolved by host via GetProcAddress */
#ifdef RELAY_EXPORTS
#define RELAY_API __declspec(dllexport)
#else
#define RELAY_API __declspec(dllimport)
#endif

#ifdef __cplusplus
extern "C" {
#endif

RELAY_API int   relay_init(HostCallbacks* callbacks);
RELAY_API void* relay_get_utility_iface(void);
RELAY_API void  relay_shutdown(void);

#ifdef __cplusplus
}
#endif

#endif /* TESSERA_RELAY_API_H */
