// =============================================================================
// File:              dll_loader.cpp
// Author(s):         Chrischn89
// Description:
//   Loads CvGameCoreDLL.dll at runtime using LoadLibrary, resolves the three
//   critical exports (getInstance, setDLLIFace, init), and wires TesseraHost's
//   stub interfaces into the DLL's gDLL pointer.
//
// License:
//   Released under the terms of the GNU General Public License version 3.0
// =============================================================================

#include "dll_loader.h"
#include <cstdio>

// ---------------------------------------------------------------------------
// load — LoadLibrary + resolve all three exports
// ---------------------------------------------------------------------------
bool DllLoader::load(const std::string& dll_path) {
    fprintf(stderr, "[DLL] Loading: %s\n", dll_path.c_str());

    m_hDll = LoadLibraryA(dll_path.c_str());
    if (!m_hDll) {
        fprintf(stderr, "[DLL ERROR] LoadLibrary failed: error %lu\n", GetLastError());
        return false;
    }

    // Resolve exports by mangled names from dumpbin /exports
    m_pfnGetInstance = (GetInstanceFn)GetProcAddress(m_hDll,
        "?getInstance@CvGlobals@@SAAAV1@XZ");

    m_pfnSetDLLIFace = (SetDLLIFaceFn)GetProcAddress(m_hDll,
        "?setDLLIFace@CvGlobals@@QAEXPAVCvDLLUtilityIFaceBase@@@Z");

    m_pfnInit = (InitFn)GetProcAddress(m_hDll,
        "?init@CvGlobals@@QAEXXZ");

    if (!m_pfnGetInstance || !m_pfnSetDLLIFace || !m_pfnInit) {
        fprintf(stderr, "[DLL ERROR] Failed to resolve exports:\n");
        fprintf(stderr, "  getInstance: %s\n", m_pfnGetInstance ? "OK" : "MISSING");
        fprintf(stderr, "  setDLLIFace: %s\n", m_pfnSetDLLIFace ? "OK" : "MISSING");
        fprintf(stderr, "  init:        %s\n", m_pfnInit ? "OK" : "MISSING");
        FreeLibrary(m_hDll);
        m_hDll = nullptr;
        return false;
    }

    fprintf(stderr, "[DLL] All exports resolved successfully\n");
    return true;
}

// ---------------------------------------------------------------------------
// wire_interfaces — get the CvGlobals singleton and inject our utility iface
// ---------------------------------------------------------------------------
bool DllLoader::wire_interfaces(CvDLLUtilityIFaceBase* utility_iface) {
    if (!m_hDll || !m_pfnGetInstance || !m_pfnSetDLLIFace) {
        fprintf(stderr, "[DLL ERROR] wire_interfaces called before successful load\n");
        return false;
    }

    // Get the CvGlobals singleton.
    // getInstance() returns CvGlobals& which at ABI level is a pointer.
    m_pGlobals = m_pfnGetInstance();
    if (!m_pGlobals) {
        fprintf(stderr, "[DLL ERROR] getInstance returned null\n");
        return false;
    }
    fprintf(stderr, "[DLL] CvGlobals instance at %p\n", m_pGlobals);

    // Wire our utility interface into the DLL's gDLL pointer.
    m_pfnSetDLLIFace(m_pGlobals, utility_iface);
    fprintf(stderr, "[DLL] setDLLIFace called successfully\n");
    return true;
}

// ---------------------------------------------------------------------------
// initialize — call CvGlobals::init() with SEH crash protection
// ---------------------------------------------------------------------------
bool DllLoader::initialize() {
    if (!m_hDll || !m_pfnInit || !m_pGlobals) {
        fprintf(stderr, "[DLL ERROR] initialize called before successful wiring\n");
        return false;
    }

    fprintf(stderr, "[DLL] Calling CvGlobals::init()...\n");

    __try {
        m_pfnInit(m_pGlobals);
        fprintf(stderr, "[DLL] init() completed successfully\n");
        return true;
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        DWORD code = GetExceptionCode();
        fprintf(stderr, "[DLL ERROR] init() crashed with exception 0x%08X\n", code);
        return false;
    }
}

// ---------------------------------------------------------------------------
// unload — free the DLL and reset all state
// ---------------------------------------------------------------------------
void DllLoader::unload() {
    if (m_hDll) {
        fprintf(stderr, "[DLL] Unloading DLL\n");
        FreeLibrary(m_hDll);
        m_hDll = nullptr;
    }
    m_pGlobals       = nullptr;
    m_pfnGetInstance  = nullptr;
    m_pfnSetDLLIFace  = nullptr;
    m_pfnInit         = nullptr;
}
