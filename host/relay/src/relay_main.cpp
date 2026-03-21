// =============================================================================
// File:              relay_main.cpp
// Author(s):         Chrischn89
// Description:
//   TesseraRelay DLL entry point. Implements relay_init, relay_get_utility_iface,
//   and relay_shutdown. Creates static instances of all sub-interface
//   implementations that the utility interface returns to the game DLL.
//
// License:
//   Released under the terms of the GNU General Public License version 3.0
// =============================================================================

#ifndef RELAY_EXPORTS
#define RELAY_EXPORTS
#endif
#include "relay_api.h"
#include "relay_types.h"

#include <cstdio>

// =============================================================================
// Global state
// =============================================================================

static HostCallbacks* g_callbacks = NULL;

// Forward-declare (full definition in relay_utility.cpp)
class CvDLLUtilityIFaceBase;

// Extern: sub-interface instances created in other relay source files
// Each is a static global in its respective file.
extern CvDLLXmlIFaceBase*  g_pRelayXmlIFace;      // relay_xml.cpp
extern CvDLLUtilityIFaceBase* g_pRelayUtilityIFace; // relay_utility.cpp

// Accessor for other relay files to reach the callbacks
HostCallbacks* relay_get_callbacks() {
    return g_callbacks;
}

// =============================================================================
// DLL Entry Point (Windows)
// NOTE: DllMain omitted — VS2003 Toolkit lacks Platform SDK (no windows.h).
// The default DllMain (provided by the CRT) returns TRUE, which is sufficient.
// =============================================================================

// =============================================================================
// Exported functions
// =============================================================================

extern "C" {

RELAY_API int relay_init(HostCallbacks* callbacks) {
    fprintf(stderr, "[TesseraRelay] relay_init called\n");

    if (!callbacks) {
        fprintf(stderr, "[TesseraRelay ERROR] callbacks is NULL\n");
        return -1;
    }

    if (callbacks->version != RELAY_API_VERSION) {
        fprintf(stderr, "[TesseraRelay ERROR] version mismatch: host=%d relay=%d\n",
            callbacks->version, RELAY_API_VERSION);
        return -2;
    }

    g_callbacks = callbacks;
    fprintf(stderr, "[TesseraRelay] Initialized OK (API version %d)\n",
        RELAY_API_VERSION);
    return 0;
}

RELAY_API void* relay_get_utility_iface() {
    fprintf(stderr, "[TesseraRelay] relay_get_utility_iface called\n");
    return static_cast<void*>(g_pRelayUtilityIFace);
}

RELAY_API void relay_shutdown() {
    fprintf(stderr, "[TesseraRelay] relay_shutdown called\n");
    g_callbacks = NULL;
}

} // extern "C"
