// =============================================================================
// File:              dll_loader.h
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
#pragma once

#include <windows.h>
#include <string>

// Forward declare our interface
class CvDLLUtilityIFaceBase;

class DllLoader {
public:
    // Load CvGameCoreDLL.dll from the given path.
    // Resolves all required exports: CvGlobals + CvXMLLoadUtility.
    bool load(const std::string& dll_path);

    // Wire our CvDLLUtilityIFaceBase into the DLL's gDLL pointer.
    // Must be called after load() succeeds.
    bool wire_interfaces(CvDLLUtilityIFaceBase* utility_iface);

    // Call CvGlobals::init() to trigger initGlobals callback.
    // Wrapped in SEH __try/__except for crash protection.
    bool initialize();

    // Orchestrate XML data loading via CvXMLLoadUtility.
    // Creates an instance, calls CreateFXml, then all loading methods in order.
    // Must be called after initialize() succeeds.
    bool load_xml_data();

    // Get pointer to CvGlobals singleton (for data extraction later).
    void* get_globals() { return m_pGlobals; }

    // Unload the DLL and reset all state.
    void unload();

private:
    HMODULE m_hDll = nullptr;
    void* m_pGlobals = nullptr;  // CvGlobals* (opaque)

    // =========================================================================
    // CvGlobals function pointers
    // =========================================================================

    // getInstance is a static method -> __cdecl (no this pointer).
    // The "reference return" from getInstance is effectively a pointer at ABI level.
    typedef void* (__cdecl *GetInstanceFn)();

    // setDLLIFace and init are member methods -> __thiscall (ECX = this on x86 MSVC).
    typedef void (__thiscall *SetDLLIFaceFn)(void* thisPtr, CvDLLUtilityIFaceBase* pDll);
    typedef void (__thiscall *InitFn)(void* thisPtr);

    GetInstanceFn  m_pfnGetInstance  = nullptr;
    SetDLLIFaceFn  m_pfnSetDLLIFace  = nullptr;
    InitFn         m_pfnInit         = nullptr;

    // =========================================================================
    // CvXMLLoadUtility function pointers
    // =========================================================================

    // All CvXMLLoadUtility methods are __thiscall (ECX = this).
    // Constructor/Destructor take no extra args.
    // Loading methods return bool, DestroyFXml returns void.
    typedef void (__thiscall *XmlCtorFn)(void* thisPtr);
    typedef void (__thiscall *XmlDtorFn)(void* thisPtr);
    typedef bool (__thiscall *XmlBoolFn)(void* thisPtr);
    typedef void (__thiscall *XmlVoidFn)(void* thisPtr);

    XmlCtorFn m_pfnXmlCtor                     = nullptr;
    XmlDtorFn m_pfnXmlDtor                     = nullptr;
    XmlBoolFn m_pfnXmlCreateFXml               = nullptr;
    XmlVoidFn m_pfnXmlDestroyFXml              = nullptr;
    XmlBoolFn m_pfnSetGlobalDefines            = nullptr;
    XmlBoolFn m_pfnSetGlobalTypes              = nullptr;
    XmlBoolFn m_pfnSetGlobalArtDefines         = nullptr;
    XmlBoolFn m_pfnLoadBasicInfos              = nullptr;
    XmlBoolFn m_pfnLoadPreMenuGlobals          = nullptr;
    XmlBoolFn m_pfnSetPostGlobalsGlobalDefines = nullptr;
    XmlBoolFn m_pfnSetupGlobalLandscapeInfo    = nullptr;
    XmlBoolFn m_pfnLoadPostMenuGlobals         = nullptr;
    XmlBoolFn m_pfnLoadGlobalText              = nullptr;
};
