// =============================================================================
// File:              data_extractor.h
// Author(s):         Chrischn89
// Description:
//   Extracts Civ IV game data (infos, art definitions) via Python 2.4 scripts
//   running against CvPythonExtensions. Outputs structured JSON using yyjson.
//
// License:
//   Released under the terms of the GNU General Public License version 3.0
// =============================================================================
#pragma once

#include <string>

class PythonBridge;

class DataExtractor {
public:
    void set_bridge(PythonBridge* bridge) { m_bridge = bridge; }

    // Get all infos of a given type as a JSON array string.
    // Returns JSON string: [{"type":"...","field":value,...}, ...]
    // Returns empty string on error.
    std::string get_all_infos(const char* type);

    // Get a single info by type and key. Returns JSON object or empty.
    std::string get_info(const char* type, const char* key);

    // Get art info by type and key. Returns JSON with nif, kfm, scale.
    std::string get_art_info(const char* type, const char* key);

private:
    PythonBridge* m_bridge = nullptr;

    // Build and run a Python 2.4 script for extracting all infos of a type.
    std::string extract_techs();
};
