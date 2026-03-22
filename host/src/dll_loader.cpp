// =============================================================================
// File:              dll_loader.cpp
// Author(s):         Chrischn89
// Description:
//   Loads CvGameCoreDLL.dll at runtime using LoadLibrary, resolves exports
//   (getInstance, setDLLIFace, init, CvXMLLoadUtility methods), wires
//   TesseraHost's stub interfaces into the DLL's gDLL pointer, and
//   orchestrates XML data loading via CvXMLLoadUtility.
//
// License:
//   Released under the terms of the GNU General Public License version 3.0
// =============================================================================

#include "dll_loader.h"
#include "host_callbacks.h"
#include <cstdio>
#include <cstring>

// Static HostCallbacks instance — must outlive the relay DLL.
static HostCallbacks s_hostCallbacks;

// ---------------------------------------------------------------------------
// load — LoadLibrary + resolve all exports (CvGlobals + CvXMLLoadUtility)
// ---------------------------------------------------------------------------
bool DllLoader::load(const std::string& dll_path) {
    fprintf(stderr, "[DLL] Loading: %s\n", dll_path.c_str());

    // Set DLL search directory to the BTS root (not the mod directory).
    // CvGameCoreDLL.dll depends on boost_python-vc71-mt-1_32.dll and python24.dll
    // which always live in the BTS root, even when loading a mod's DLL.
    extern std::string g_basePath;
    std::string bts_root = g_basePath + "\\Beyond the Sword";
    fprintf(stderr, "[DLL] Adding DLL search path: %s\n", bts_root.c_str());
    SetDllDirectoryA(bts_root.c_str());

    // Set working directory to BTS root. Mods with built-in XML parsers
    // (e.g., RapidXML in Rise of Mankind) open files relative to CWD.
    SetCurrentDirectoryA(bts_root.c_str());
    fprintf(stderr, "[DLL] Set working directory: %s\n", bts_root.c_str());

    m_hDll = LoadLibraryA(dll_path.c_str());
    if (!m_hDll) {
        fprintf(stderr, "[DLL ERROR] LoadLibrary failed: error %lu\n", GetLastError());
        return false;
    }

    // -----------------------------------------------------------------------
    // Resolve CvGlobals exports (mangled names from dumpbin /exports)
    // -----------------------------------------------------------------------
    m_pfnGetInstance = (GetInstanceFn)GetProcAddress(m_hDll,
        "?getInstance@CvGlobals@@SAAAV1@XZ");

    m_pfnSetDLLIFace = (SetDLLIFaceFn)GetProcAddress(m_hDll,
        "?setDLLIFace@CvGlobals@@QAEXPAVCvDLLUtilityIFaceBase@@@Z");

    m_pfnInit = (InitFn)GetProcAddress(m_hDll,
        "?init@CvGlobals@@QAEXXZ");

    if (!m_pfnGetInstance || !m_pfnSetDLLIFace || !m_pfnInit) {
        fprintf(stderr, "[DLL ERROR] Failed to resolve CvGlobals exports:\n");
        fprintf(stderr, "  getInstance: %s\n", m_pfnGetInstance ? "OK" : "MISSING");
        fprintf(stderr, "  setDLLIFace: %s\n", m_pfnSetDLLIFace ? "OK" : "MISSING");
        fprintf(stderr, "  init:        %s\n", m_pfnInit ? "OK" : "MISSING");
        FreeLibrary(m_hDll);
        m_hDll = nullptr;
        return false;
    }

    fprintf(stderr, "[DLL] CvGlobals exports resolved OK\n");

    // -----------------------------------------------------------------------
    // Resolve CvXMLLoadUtility exports
    // -----------------------------------------------------------------------
    m_pfnXmlCtor = (XmlCtorFn)GetProcAddress(m_hDll,
        "??0CvXMLLoadUtility@@QAE@XZ");
    m_pfnXmlDtor = (XmlDtorFn)GetProcAddress(m_hDll,
        "??1CvXMLLoadUtility@@QAE@XZ");
    m_pfnXmlCreateFXml = (XmlBoolFn)GetProcAddress(m_hDll,
        "?CreateFXml@CvXMLLoadUtility@@QAE_NXZ");
    m_pfnXmlDestroyFXml = (XmlVoidFn)GetProcAddress(m_hDll,
        "?DestroyFXml@CvXMLLoadUtility@@QAEXXZ");
    m_pfnSetGlobalDefines = (XmlBoolFn)GetProcAddress(m_hDll,
        "?SetGlobalDefines@CvXMLLoadUtility@@QAE_NXZ");
    m_pfnSetGlobalTypes = (XmlBoolFn)GetProcAddress(m_hDll,
        "?SetGlobalTypes@CvXMLLoadUtility@@QAE_NXZ");
    m_pfnSetGlobalArtDefines = (XmlBoolFn)GetProcAddress(m_hDll,
        "?SetGlobalArtDefines@CvXMLLoadUtility@@QAE_NXZ");
    m_pfnLoadBasicInfos = (XmlBoolFn)GetProcAddress(m_hDll,
        "?LoadBasicInfos@CvXMLLoadUtility@@QAE_NXZ");
    m_pfnLoadPreMenuGlobals = (XmlBoolFn)GetProcAddress(m_hDll,
        "?LoadPreMenuGlobals@CvXMLLoadUtility@@QAE_NXZ");
    m_pfnSetPostGlobalsGlobalDefines = (XmlBoolFn)GetProcAddress(m_hDll,
        "?SetPostGlobalsGlobalDefines@CvXMLLoadUtility@@QAE_NXZ");
    m_pfnSetupGlobalLandscapeInfo = (XmlBoolFn)GetProcAddress(m_hDll,
        "?SetupGlobalLandscapeInfo@CvXMLLoadUtility@@QAE_NXZ");
    m_pfnLoadPostMenuGlobals = (XmlBoolFn)GetProcAddress(m_hDll,
        "?LoadPostMenuGlobals@CvXMLLoadUtility@@QAE_NXZ");
    m_pfnLoadGlobalText = (XmlBoolFn)GetProcAddress(m_hDll,
        "?LoadGlobalText@CvXMLLoadUtility@@QAE_NXZ");

    // Check all XML exports resolved
    bool xml_ok = m_pfnXmlCtor && m_pfnXmlDtor && m_pfnXmlCreateFXml
        && m_pfnXmlDestroyFXml && m_pfnSetGlobalDefines && m_pfnSetGlobalTypes
        && m_pfnSetGlobalArtDefines && m_pfnLoadBasicInfos
        && m_pfnLoadPreMenuGlobals && m_pfnSetPostGlobalsGlobalDefines
        && m_pfnSetupGlobalLandscapeInfo && m_pfnLoadPostMenuGlobals
        && m_pfnLoadGlobalText;

    if (!xml_ok) {
        fprintf(stderr, "[DLL WARNING] Some CvXMLLoadUtility exports missing:\n");
        fprintf(stderr, "  Ctor:                     %s\n", m_pfnXmlCtor ? "OK" : "MISSING");
        fprintf(stderr, "  Dtor:                     %s\n", m_pfnXmlDtor ? "OK" : "MISSING");
        fprintf(stderr, "  CreateFXml:               %s\n", m_pfnXmlCreateFXml ? "OK" : "MISSING");
        fprintf(stderr, "  DestroyFXml:              %s\n", m_pfnXmlDestroyFXml ? "OK" : "MISSING");
        fprintf(stderr, "  SetGlobalDefines:         %s\n", m_pfnSetGlobalDefines ? "OK" : "MISSING");
        fprintf(stderr, "  SetGlobalTypes:           %s\n", m_pfnSetGlobalTypes ? "OK" : "MISSING");
        fprintf(stderr, "  SetGlobalArtDefines:      %s\n", m_pfnSetGlobalArtDefines ? "OK" : "MISSING");
        fprintf(stderr, "  LoadBasicInfos:           %s\n", m_pfnLoadBasicInfos ? "OK" : "MISSING");
        fprintf(stderr, "  LoadPreMenuGlobals:       %s\n", m_pfnLoadPreMenuGlobals ? "OK" : "MISSING");
        fprintf(stderr, "  SetPostGlobalsGlobalDef:  %s\n", m_pfnSetPostGlobalsGlobalDefines ? "OK" : "MISSING");
        fprintf(stderr, "  SetupGlobalLandscapeInfo: %s\n", m_pfnSetupGlobalLandscapeInfo ? "OK" : "MISSING");
        fprintf(stderr, "  LoadPostMenuGlobals:      %s\n", m_pfnLoadPostMenuGlobals ? "OK" : "MISSING");
        fprintf(stderr, "  LoadGlobalText:           %s\n", m_pfnLoadGlobalText ? "OK" : "MISSING");
        // Non-fatal: we can still use CvGlobals, just can't load XML
    } else {
        fprintf(stderr, "[DLL] CvXMLLoadUtility exports resolved OK\n");
    }

    // -----------------------------------------------------------------------
    // Resolve CvArtFileMgr exports — needed to build art info lookup maps.
    // Without these, updateArtDefineButton() in CvUnitInfo::read() crashes
    // because the art info maps are never allocated/populated.
    // -----------------------------------------------------------------------
    m_pfnArtMgrGetInstance = (ArtMgrGetInstanceFn)GetProcAddress(m_hDll,
        "?GetInstance@CvArtFileMgr@@SAAAV1@XZ");
    m_pfnArtMgrInit = (ArtMgrVoidFn)GetProcAddress(m_hDll,
        "?Init@CvArtFileMgr@@QAEXXZ");
    m_pfnArtMgrBuildMaps = (ArtMgrVoidFn)GetProcAddress(m_hDll,
        "?buildArtFileInfoMaps@CvArtFileMgr@@QAEXXZ");

    if (m_pfnArtMgrGetInstance && m_pfnArtMgrInit && m_pfnArtMgrBuildMaps) {
        fprintf(stderr, "[DLL] CvArtFileMgr exports resolved OK\n");
    } else {
        fprintf(stderr, "[DLL WARNING] CvArtFileMgr exports missing:\n");
        fprintf(stderr, "  GetInstance:          %s\n", m_pfnArtMgrGetInstance ? "OK" : "MISSING");
        fprintf(stderr, "  Init:                %s\n", m_pfnArtMgrInit ? "OK" : "MISSING");
        fprintf(stderr, "  buildArtFileInfoMaps:%s\n", m_pfnArtMgrBuildMaps ? "OK" : "MISSING");
    }

    fprintf(stderr, "[DLL] All exports resolved successfully\n");
    return true;
}

// ---------------------------------------------------------------------------
// load_relay — LoadLibrary TesseraRelay.dll, resolve exports, call relay_init
// ---------------------------------------------------------------------------
bool DllLoader::load_relay(const std::string& relay_path) {
    fprintf(stderr, "[RELAY] Loading: %s\n", relay_path.c_str());

    m_hRelay = LoadLibraryA(relay_path.c_str());
    if (!m_hRelay) {
        fprintf(stderr, "[RELAY ERROR] LoadLibrary failed: error %lu\n", GetLastError());
        return false;
    }

    // Resolve relay exports (plain C names — no mangling)
    m_pfnRelayInit = (RelayInitFn)GetProcAddress(m_hRelay, "relay_init");
    m_pfnRelayGetUtility = (RelayGetUtilityFn)GetProcAddress(m_hRelay,
        "relay_get_utility_iface");
    m_pfnRelayShutdown = (RelayShutdownFn)GetProcAddress(m_hRelay,
        "relay_shutdown");

    if (!m_pfnRelayInit || !m_pfnRelayGetUtility || !m_pfnRelayShutdown) {
        fprintf(stderr, "[RELAY ERROR] Failed to resolve relay exports:\n");
        fprintf(stderr, "  relay_init:              %s\n",
                m_pfnRelayInit ? "OK" : "MISSING");
        fprintf(stderr, "  relay_get_utility_iface: %s\n",
                m_pfnRelayGetUtility ? "OK" : "MISSING");
        fprintf(stderr, "  relay_shutdown:           %s\n",
                m_pfnRelayShutdown ? "OK" : "MISSING");
        FreeLibrary(m_hRelay);
        m_hRelay = nullptr;
        m_pfnRelayInit = nullptr;
        m_pfnRelayGetUtility = nullptr;
        m_pfnRelayShutdown = nullptr;
        return false;
    }

    fprintf(stderr, "[RELAY] Exports resolved OK\n");

    // Create host callbacks and pass to relay
    s_hostCallbacks = create_host_callbacks();
    int rc = m_pfnRelayInit(&s_hostCallbacks);
    if (rc != 0) {
        fprintf(stderr, "[RELAY ERROR] relay_init returned %d\n", rc);
        FreeLibrary(m_hRelay);
        m_hRelay = nullptr;
        m_pfnRelayInit = nullptr;
        m_pfnRelayGetUtility = nullptr;
        m_pfnRelayShutdown = nullptr;
        return false;
    }

    fprintf(stderr, "[RELAY] relay_init succeeded\n");
    return true;
}

// ---------------------------------------------------------------------------
// wire_interfaces — get the CvGlobals singleton and inject relay utility iface
// ---------------------------------------------------------------------------
bool DllLoader::wire_interfaces() {
    if (!m_hDll || !m_pfnGetInstance || !m_pfnSetDLLIFace) {
        fprintf(stderr, "[DLL ERROR] wire_interfaces called before successful load\n");
        return false;
    }

    if (!m_hRelay || !m_pfnRelayGetUtility) {
        fprintf(stderr, "[DLL ERROR] wire_interfaces called before successful load_relay\n");
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

    // Get the relay's VS2003-compiled utility interface
    void* utility_ptr = m_pfnRelayGetUtility();
    if (!utility_ptr) {
        fprintf(stderr, "[DLL ERROR] relay_get_utility_iface returned null\n");
        return false;
    }
    fprintf(stderr, "[DLL] Relay utility interface at %p\n", utility_ptr);

    // Wire relay's utility interface into the DLL's gDLL pointer.
    m_pfnSetDLLIFace(m_pGlobals, static_cast<CvDLLUtilityIFaceBase*>(utility_ptr));
    fprintf(stderr, "[DLL] setDLLIFace called successfully (relay interface)\n");
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
// load_xml_data — orchestrate CvXMLLoadUtility XML loading sequence
// ---------------------------------------------------------------------------
bool DllLoader::load_xml_data() {
    if (!m_hDll) {
        fprintf(stderr, "[DLL ERROR] load_xml_data called before DLL loaded\n");
        return false;
    }

    // Verify all XML function pointers are resolved
    if (!m_pfnXmlCtor || !m_pfnXmlDtor || !m_pfnXmlCreateFXml
        || !m_pfnXmlDestroyFXml) {
        fprintf(stderr, "[DLL ERROR] CvXMLLoadUtility exports not resolved\n");
        return false;
    }

    fprintf(stderr, "[DLL] === XML Loading Orchestration Begin ===\n");

    // CvXMLLoadUtility class layout from SDK header:
    //   FXml*            m_pFXml           (4 bytes)
    //   FXmlSchemaCache* m_pSchemaCache    (4 bytes)
    //   int              m_iCurProgressStep(4 bytes)
    //   ProgressCB       m_pCBFxn          (4 bytes)
    // Total: 16 bytes of data members. No vtable (no virtual methods).
    // Use 256 bytes for generous safety margin.
    static const size_t XML_UTIL_SIZE = 256;
    char xmlUtilBuf[XML_UTIL_SIZE];
    memset(xmlUtilBuf, 0, XML_UTIL_SIZE);

    // --- Construct CvXMLLoadUtility ---
    fprintf(stderr, "[DLL] Constructing CvXMLLoadUtility (%zu byte buffer)...\n", XML_UTIL_SIZE);
    __try {
        m_pfnXmlCtor(xmlUtilBuf);
        fprintf(stderr, "[DLL] CvXMLLoadUtility constructed OK\n");
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        fprintf(stderr, "[DLL ERROR] CvXMLLoadUtility constructor crashed: 0x%08X\n",
                GetExceptionCode());
        return false;
    }

    // --- Create internal FXml parser ---
    fprintf(stderr, "[DLL] Calling CreateFXml()...\n");
    bool ok = false;
    __try {
        ok = m_pfnXmlCreateFXml(xmlUtilBuf);
        fprintf(stderr, "[DLL] CreateFXml() returned %s\n", ok ? "true" : "false");
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        fprintf(stderr, "[DLL ERROR] CreateFXml() crashed: 0x%08X\n",
                GetExceptionCode());
        // Try to destruct before returning
        __try { m_pfnXmlDtor(xmlUtilBuf); } __except(EXCEPTION_EXECUTE_HANDLER) {}
        return false;
    }

    if (!ok) {
        fprintf(stderr, "[DLL ERROR] CreateFXml() returned false — aborting XML loading\n");
        __try { m_pfnXmlDtor(xmlUtilBuf); } __except(EXCEPTION_EXECUTE_HANDLER) {}
        return false;
    }

    // --- Loading sequence: each step with SEH protection ---
    // Debug: extern callback diagnostics from host_callbacks.cpp
    extern int g_cbCallCount;
    extern const char* g_cbLastName;
    extern const char* g_cbLastXmlFile;

    // Helper macro for repetitive SEH-wrapped calls
    #define XML_LOAD_STEP(name, fnPtr) \
        do { \
            fprintf(stderr, "[DLL] Loading %s...\n", name); \
            __try { \
                bool step_ok = (fnPtr)(xmlUtilBuf); \
                fprintf(stderr, "[DLL] %s returned %s\n", name, step_ok ? "true" : "false"); \
            } \
            __except (EXCEPTION_EXECUTE_HANDLER) { \
                fprintf(stderr, "[DLL ERROR] %s crashed: 0x%08X\n", name, GetExceptionCode()); \
                fprintf(stderr, "[DLL ERROR] Last callback #%d: %s\n", g_cbCallCount, g_cbLastName); \
                fprintf(stderr, "[DLL ERROR] Last XML file: %s\n", g_cbLastXmlFile); \
                fflush(stderr); \
                /* Continue to next step instead of aborting */ \
            } \
        } while(0)

    if (m_pfnSetGlobalDefines)
        XML_LOAD_STEP("SetGlobalDefines", m_pfnSetGlobalDefines);

    if (m_pfnSetGlobalTypes)
        XML_LOAD_STEP("SetGlobalTypes", m_pfnSetGlobalTypes);

    if (m_pfnSetGlobalArtDefines)
        XML_LOAD_STEP("SetGlobalArtDefines", m_pfnSetGlobalArtDefines);

    // --- Initialize CvArtFileMgr maps after art defines are loaded ---
    // The game EXE normally calls CvArtFileMgr::Init() + buildArtFileInfoMaps()
    // after SetGlobalArtDefines. Without this, updateArtDefineButton() in
    // CvUnitInfo::read() crashes (null pointer dereference on unallocated maps).
    if (m_pfnArtMgrGetInstance && m_pfnArtMgrInit && m_pfnArtMgrBuildMaps) {
        fprintf(stderr, "[DLL] Initializing CvArtFileMgr...\n");
        __try {
            void* artMgr = m_pfnArtMgrGetInstance();
            if (artMgr) {
                m_pfnArtMgrInit(artMgr);
                fprintf(stderr, "[DLL] CvArtFileMgr::Init() OK\n");
                m_pfnArtMgrBuildMaps(artMgr);
                fprintf(stderr, "[DLL] CvArtFileMgr::buildArtFileInfoMaps() OK\n");
            } else {
                fprintf(stderr, "[DLL ERROR] CvArtFileMgr::GetInstance() returned null\n");
            }
        }
        __except (EXCEPTION_EXECUTE_HANDLER) {
            fprintf(stderr, "[DLL ERROR] CvArtFileMgr init crashed: 0x%08X\n",
                    GetExceptionCode());
        }
    }

    if (m_pfnLoadBasicInfos)
        XML_LOAD_STEP("LoadBasicInfos", m_pfnLoadBasicInfos);

    if (m_pfnLoadPreMenuGlobals)
        XML_LOAD_STEP("LoadPreMenuGlobals", m_pfnLoadPreMenuGlobals);

    if (m_pfnSetPostGlobalsGlobalDefines)
        XML_LOAD_STEP("SetPostGlobalsGlobalDefines", m_pfnSetPostGlobalsGlobalDefines);

    if (m_pfnSetupGlobalLandscapeInfo)
        XML_LOAD_STEP("SetupGlobalLandscapeInfo", m_pfnSetupGlobalLandscapeInfo);

    if (m_pfnLoadPostMenuGlobals)
        XML_LOAD_STEP("LoadPostMenuGlobals", m_pfnLoadPostMenuGlobals);

    if (m_pfnLoadGlobalText)
        XML_LOAD_STEP("LoadGlobalText", m_pfnLoadGlobalText);

    #undef XML_LOAD_STEP

    fprintf(stderr, "[DLL] === XML Loading Sequence Complete ===\n");

    // Skip DestroyFXml and CvXMLLoadUtility destructor cleanup.
    // Mod DLLs with custom allocators (global operator new override) can cause
    // STATUS_HEAP_CORRUPTION during cleanup which bypasses SEH via __fastfail.
    // The XML data is fully loaded — cleanup is unnecessary for a one-shot process.
    fprintf(stderr, "[DLL] Skipping FXml/XMLUtility cleanup (heap corruption workaround)\n");

    fprintf(stderr, "[DLL] === XML Loading Orchestration End ===\n");
    return true;
}

// ---------------------------------------------------------------------------
// unload — free the DLL and reset all state
// ---------------------------------------------------------------------------
void DllLoader::unload() {
    // Skip FreeLibrary for CvGameCoreDLL — its CvGlobals destructor triggers
    // STATUS_HEAP_CORRUPTION due to small buffer overflows in the game DLL's
    // internal allocations during XML loading. The overflows write past heap
    // block boundaries; the debug heap absorbs them but the normal heap doesn't.
    // Since we're a one-shot process, let the OS reclaim memory on exit.
    if (m_hDll) {
        fprintf(stderr, "[DLL] Skipping CvGameCoreDLL FreeLibrary (heap corruption workaround)\n");
        m_hDll = nullptr;
    }
    m_pGlobals       = nullptr;
    m_pfnGetInstance  = nullptr;
    m_pfnSetDLLIFace  = nullptr;
    m_pfnInit         = nullptr;

    // Reset XML function pointers
    m_pfnXmlCtor                     = nullptr;
    m_pfnXmlDtor                     = nullptr;
    m_pfnXmlCreateFXml               = nullptr;
    m_pfnXmlDestroyFXml              = nullptr;
    m_pfnSetGlobalDefines            = nullptr;
    m_pfnSetGlobalTypes              = nullptr;
    m_pfnSetGlobalArtDefines         = nullptr;
    m_pfnLoadBasicInfos              = nullptr;
    m_pfnLoadPreMenuGlobals          = nullptr;
    m_pfnSetPostGlobalsGlobalDefines = nullptr;
    m_pfnSetupGlobalLandscapeInfo    = nullptr;
    m_pfnLoadPostMenuGlobals         = nullptr;
    m_pfnLoadGlobalText              = nullptr;

    // Reset CvArtFileMgr pointers
    m_pfnArtMgrGetInstance           = nullptr;
    m_pfnArtMgrInit                  = nullptr;
    m_pfnArtMgrBuildMaps             = nullptr;

    // Unload relay DLL (after game DLL is freed)
    if (m_hRelay) {
        fprintf(stderr, "[RELAY] Shutting down relay\n");
        if (m_pfnRelayShutdown)
            m_pfnRelayShutdown();
        FreeLibrary(m_hRelay);
        m_hRelay = nullptr;
    }
    m_pfnRelayInit       = nullptr;
    m_pfnRelayGetUtility = nullptr;
    m_pfnRelayShutdown   = nullptr;
}
