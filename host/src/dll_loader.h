// =============================================================================
// File:              dll_loader.h
// Author(s):         Chrischn89
// Description:
//   Loads CvGameCoreDLL.dll at runtime using LoadLibrary, resolves the three
//   critical exports (getInstance, setDLLIFace, init), and wires TesseraHost's
//   stub interfaces into the DLL's gDLL pointer.
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
    // Resolves the three required exports: getInstance, setDLLIFace, init.
    bool load(const std::string& dll_path);

    // Wire our CvDLLUtilityIFaceBase into the DLL's gDLL pointer.
    // Must be called after load() succeeds.
    bool wire_interfaces(CvDLLUtilityIFaceBase* utility_iface);

    // Call CvGlobals::init() to trigger XML loading.
    // Wrapped in SEH __try/__except for crash protection.
    bool initialize();

    // Get pointer to CvGlobals singleton (for data extraction later).
    void* get_globals() { return m_pGlobals; }

    // Unload the DLL and reset all state.
    void unload();

private:
    HMODULE m_hDll = nullptr;
    void* m_pGlobals = nullptr;  // CvGlobals* (opaque)

    // Function pointer types for the three exports.
    //
    // getInstance is a static method -> __cdecl (no this pointer).
    // The "reference return" from getInstance is effectively a pointer at ABI level.
    typedef void* (__cdecl *GetInstanceFn)();

    // setDLLIFace and init are member methods -> __thiscall (ECX = this on x86 MSVC).
    typedef void (__thiscall *SetDLLIFaceFn)(void* thisPtr, CvDLLUtilityIFaceBase* pDll);
    typedef void (__thiscall *InitFn)(void* thisPtr);

    GetInstanceFn  m_pfnGetInstance  = nullptr;
    SetDLLIFaceFn  m_pfnSetDLLIFace  = nullptr;
    InitFn         m_pfnInit         = nullptr;
};
