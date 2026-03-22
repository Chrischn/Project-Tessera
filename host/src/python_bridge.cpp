// =============================================================================
// File:              python_bridge.cpp
// Author(s):         Chrischn89
// Description:
//   Implements PythonBridge: embeds Python 2.4, registers CvGameCoreDLL's
//   Boost.Python bindings, and provides script execution with output capture.
//
// License:
//   Released under the terms of the GNU General Public License version 3.0
// =============================================================================
#include "python_bridge.h"

#include <cstdio>
#include <fstream>
#include <sstream>

// ---------------------------------------------------------------------------
// SEH wrappers — plain C-style functions (no C++ objects on stack)
// MSVC forbids __try in functions that need object unwinding.
// ---------------------------------------------------------------------------
typedef void (*VoidFn)();
typedef int  (*IntStringFn)(const char*);
typedef void (*BoostInitModuleFn)(const char*, void(*)());

static bool seh_call_void(VoidFn fn, const char* name, DWORD* out_code) {
    __try {
        fn();
        return true;
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        *out_code = GetExceptionCode();
        fprintf(stderr, "[PyBridge] %s crashed: 0x%08X\n", name, *out_code);
        return false;
    }
}

static bool seh_call_init_module(BoostInitModuleFn fn, const char* name,
                                  VoidFn callback, DWORD* out_code) {
    __try {
        fn(name, callback);
        return true;
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        *out_code = GetExceptionCode();
        fprintf(stderr, "[PyBridge] init_module crashed: 0x%08X\n", *out_code);
        return false;
    }
}

static bool seh_call_pyrun(IntStringFn fn, const char* script,
                           int* out_rc, DWORD* out_code) {
    __try {
        *out_rc = fn(script);
        return true;
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        *out_code = GetExceptionCode();
        fprintf(stderr, "[PyBridge] PyRun_SimpleString crashed: 0x%08X\n", *out_code);
        return false;
    }
}

// ---------------------------------------------------------------------------
// PythonBridge::init
// ---------------------------------------------------------------------------
bool PythonBridge::init(HMODULE hGameDll, std::string& error_out) {
    if (m_initialized) {
        error_out = "already initialized";
        return true;  // not an error, just a no-op
    }

    if (!hGameDll) {
        error_out = "game DLL handle is null";
        return false;
    }

    // Step 1: python24.dll is already loaded as a dependency of CvGameCoreDLL.dll.
    HMODULE hPython = GetModuleHandleA("python24.dll");
    if (!hPython) {
        error_out = "python24.dll not found in process "
                    "(should be a CvGameCoreDLL dependency)";
        return false;
    }
    fprintf(stderr, "[PyBridge] python24.dll found at %p\n", (void*)hPython);

    // Step 2: Resolve Python C API functions
    m_pfnPyInit   = (Py_InitializeFn)GetProcAddress(hPython, "Py_Initialize");
    m_pfnPyIsInit = (Py_IsInitializedFn)GetProcAddress(hPython, "Py_IsInitialized");
    m_pfnPyRun    = (PyRun_SimpleStringFn)GetProcAddress(hPython, "PyRun_SimpleString");

    if (!m_pfnPyInit || !m_pfnPyIsInit || !m_pfnPyRun) {
        error_out = "failed to resolve Python C API (Py_Initialize/IsInitialized/Run)";
        return false;
    }
    fprintf(stderr, "[PyBridge] Python C API resolved: Init=%p IsInit=%p Run=%p\n",
            (void*)m_pfnPyInit, (void*)m_pfnPyIsInit, (void*)m_pfnPyRun);

    // Step 3: Resolve Boost.Python init_module
    HMODULE hBoost = GetModuleHandleA("boost_python-vc71-mt-1_32.dll");
    if (!hBoost) {
        error_out = "boost_python-vc71-mt-1_32.dll not found in process";
        return false;
    }
    fprintf(stderr, "[PyBridge] boost_python DLL at %p\n", (void*)hBoost);

    // Step 4: Resolve init_module and DLLPublishToPython
    auto pfnBoostInitModule = (BoostInitModuleFn)GetProcAddress(
        hBoost, "?init_module@detail@python@boost@@YAXPBDP6AXXZ@Z");
    if (!pfnBoostInitModule) {
        error_out = "boost::python::detail::init_module not found";
        return false;
    }

    // Step 5: Resolve DLLPublishToPython from the game DLL
    auto pfnPublish = (VoidFn)GetProcAddress(
        hGameDll, "?DLLPublishToPython@@YAXXZ");
    if (!pfnPublish) {
        error_out = "DLLPublishToPython not found in game DLL";
        return false;
    }

    // Step 6: Initialize Python interpreter (if not already)
    DWORD crash_code = 0;
    if (!m_pfnPyIsInit()) {
        fprintf(stderr, "[PyBridge] Calling Py_Initialize()...\n");
        if (!seh_call_void((VoidFn)m_pfnPyInit, "Py_Initialize", &crash_code)) {
            error_out = "Py_Initialize() crashed";
            return false;
        }
        fprintf(stderr, "[PyBridge] Py_Initialize() succeeded\n");
    } else {
        fprintf(stderr, "[PyBridge] Python already initialized\n");
    }

    // Step 7: Register CvPythonExtensions via Boost.Python
    fprintf(stderr, "[PyBridge] Calling boost::python::detail::init_module"
                    "(\"CvPythonExtensions\", DLLPublishToPython)...\n");
    if (!seh_call_init_module(pfnBoostInitModule, "CvPythonExtensions",
                              pfnPublish, &crash_code)) {
        error_out = "init_module(CvPythonExtensions) crashed";
        return false;
    }
    fprintf(stderr, "[PyBridge] init_module succeeded — Boost.Python bindings registered\n");

    // Step 8: Build temp file path for script output capture
    char tmpDir[MAX_PATH];
    DWORD tmpLen = GetTempPathA(MAX_PATH, tmpDir);
    if (tmpLen == 0 || tmpLen > MAX_PATH) {
        error_out = "GetTempPathA failed";
        return false;
    }
    m_tempFilePath = std::string(tmpDir) + "tessera_py_result.txt";
    fprintf(stderr, "[PyBridge] Temp file path: %s\n", m_tempFilePath.c_str());

    // Step 9: Done
    m_initialized = true;
    fprintf(stderr, "[PyBridge] Initialization complete\n");
    return true;
}

// ---------------------------------------------------------------------------
// PythonBridge::run_script
// ---------------------------------------------------------------------------
int PythonBridge::run_script(const char* script) {
    if (!m_initialized || !m_pfnPyRun)
        return -2;

    DWORD crash_code = 0;
    int rc = -999;
    if (!seh_call_pyrun((IntStringFn)m_pfnPyRun, script, &rc, &crash_code)) {
        return -2;  // SEH crash
    }

    // PyRun_SimpleString returns 0 on success, -1 on Python exception
    return rc;
}

// ---------------------------------------------------------------------------
// PythonBridge::run_script_with_output
// ---------------------------------------------------------------------------
std::string PythonBridge::run_script_with_output(const char* script) {
    int rc = run_script(script);
    if (rc == -2)
        return "[PyBridge] Script execution crashed (SEH)";
    if (rc != 0)
        return "[PyBridge] Script raised a Python exception (rc=" + std::to_string(rc) + ")";

    // Read the temp file contents
    std::ifstream ifs(m_tempFilePath);
    if (!ifs.is_open())
        return "[PyBridge] Could not open temp results file: " + m_tempFilePath;

    std::ostringstream oss;
    oss << ifs.rdbuf();
    return oss.str();
}
