// =============================================================================
// File:              python_bridge.h
// Author(s):         Chrischn89
// Description:
//   Embeds Python 2.4 (already loaded as a CvGameCoreDLL dependency) and
//   registers the DLL's Boost.Python bindings via init_module. Provides
//   run_script / run_script_with_output for data extraction.
//
// License:
//   Released under the terms of the GNU General Public License version 3.0
// =============================================================================
#pragma once

#include <windows.h>
#include <string>

class PythonBridge {
public:
    bool init(HMODULE hGameDll, std::string& error_out);
    int run_script(const char* script);
    std::string run_script_with_output(const char* script);
    bool is_initialized() const { return m_initialized; }
    const std::string& get_temp_file_path() const { return m_tempFilePath; }

private:
    bool m_initialized = false;
    std::string m_tempFilePath;  // built in init() via GetTempPathA

    typedef void (*Py_InitializeFn)();
    typedef int  (*Py_IsInitializedFn)();
    typedef int  (*PyRun_SimpleStringFn)(const char*);

    Py_InitializeFn      m_pfnPyInit   = nullptr;
    Py_IsInitializedFn   m_pfnPyIsInit = nullptr;
    PyRun_SimpleStringFn m_pfnPyRun    = nullptr;
};
