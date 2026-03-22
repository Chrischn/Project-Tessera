// =============================================================================
// File:              data_extractor.cpp
// Author(s):         Chrischn89
// Description:
//   Extracts Civ IV game data via Python 2.4 scripts and converts TSV output
//   to JSON arrays using yyjson.
//   Supports: tech, building, unit, promotion, bonus, civilization, era.
//
// License:
//   Released under the terms of the GNU General Public License version 3.0
// =============================================================================
#include "data_extractor.h"
#include "python_bridge.h"

#include <yyjson.h>
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>
#include <sstream>

// ---------------------------------------------------------------------------
// get_all_infos — dispatch to the appropriate extractor by type
// ---------------------------------------------------------------------------
std::string DataExtractor::get_all_infos(const char* type) {
    if (!m_bridge || !m_bridge->is_initialized()) {
        fprintf(stderr, "[DataExtractor] get_all_infos: bridge not initialized\n");
        return {};
    }

    if (!type || type[0] == '\0') {
        fprintf(stderr, "[DataExtractor] get_all_infos: empty type\n");
        return {};
    }

    if (strcmp(type, "tech") == 0) {
        return extract_techs();
    } else if (strcmp(type, "building") == 0) {
        return extract_buildings();
    } else if (strcmp(type, "unit") == 0) {
        return extract_units();
    } else if (strcmp(type, "promotion") == 0) {
        return extract_simple_infos("getNumPromotionInfos", "getPromotionInfo");
    } else if (strcmp(type, "bonus") == 0) {
        return extract_simple_infos("getNumBonusInfos", "getBonusInfo");
    } else if (strcmp(type, "civilization") == 0) {
        return extract_simple_infos("getNumCivilizationInfos", "getCivilizationInfo");
    } else if (strcmp(type, "era") == 0) {
        return extract_simple_infos("getNumEraInfos", "getEraInfo");
    } else if (strcmp(type, "unit_art") == 0) {
        return extract_unit_art();
    } else if (strcmp(type, "building_art") == 0) {
        return extract_building_art();
    }

    fprintf(stderr, "[DataExtractor] get_all_infos: unknown type '%s'\n", type);
    return {};
}

// ---------------------------------------------------------------------------
// get_info — single info lookup by type and key string
// Returns a JSON object (not array) with the same fields as get_all_infos.
// ---------------------------------------------------------------------------
std::string DataExtractor::get_info(const char* type, const char* key) {
    if (!m_bridge || !m_bridge->is_initialized()) {
        fprintf(stderr, "[DataExtractor] get_info: bridge not initialized\n");
        return {};
    }
    if (!type || !key || type[0] == '\0' || key[0] == '\0') {
        fprintf(stderr, "[DataExtractor] get_info: empty type or key\n");
        return {};
    }

    // Map type to Python count/get methods and determine field layout
    // Fields must match what get_all_infos returns for that type.
    struct InfoType {
        const char* countMethod;
        const char* getMethod;
        const char* writeFormat;  // Python format string for f.write()
        int fieldCount;           // number of tab-separated fields (including type)
    };

    InfoType info = {};
    bool found = false;

    if (strcmp(type, "tech") == 0) {
        info = {"getNumTechInfos", "getTechInfo",
                "'%s\\t%d\\t%d\\n' % (info.getType(), info.getResearchCost(), info.getEra())", 3};
        found = true;
    } else if (strcmp(type, "building") == 0) {
        info = {"getNumBuildingInfos", "getBuildingInfo",
                "'%s\\t%d\\t%s\\n' % (info.getType(), info.getProductionCost(), info.getArtDefineTag())", 3};
        found = true;
    } else if (strcmp(type, "unit") == 0) {
        info = {"getNumUnitInfos", "getUnitInfo",
                "'%s\\t%d\\t%d\\t%d\\n' % (info.getType(), info.getCombat(), info.getMoves(), info.getProductionCost())", 4};
        found = true;
    } else if (strcmp(type, "promotion") == 0) {
        info = {"getNumPromotionInfos", "getPromotionInfo", "'%s\\n' % info.getType()", 1};
        found = true;
    } else if (strcmp(type, "bonus") == 0) {
        info = {"getNumBonusInfos", "getBonusInfo", "'%s\\n' % info.getType()", 1};
        found = true;
    } else if (strcmp(type, "civilization") == 0) {
        info = {"getNumCivilizationInfos", "getCivilizationInfo", "'%s\\n' % info.getType()", 1};
        found = true;
    } else if (strcmp(type, "era") == 0) {
        info = {"getNumEraInfos", "getEraInfo", "'%s\\n' % info.getType()", 1};
        found = true;
    }

    if (!found) {
        fprintf(stderr, "[DataExtractor] get_info: unknown type '%s'\n", type);
        return {};
    }

    std::string tempPath = m_bridge->get_temp_file_path();
    std::string keyStr(key);

    // Build Python 2.4 script: iterate all infos of the given type,
    // write the matching one to the temp file, then break.
    std::string script =
        "try:\n"
        "    from CvPythonExtensions import *\n"
        "    gc = CyGlobalContext()\n"
        "    target = '" + keyStr + "'\n"
        "    f = open('" + tempPath + "', 'w')\n"
        "    found = 0\n"
        "    for i in range(gc." + std::string(info.countMethod) + "()):\n"
        "        info = gc." + std::string(info.getMethod) + "(i)\n"
        "        if info and info.getType() == target:\n"
        "            f.write(" + std::string(info.writeFormat) + ")\n"
        "            found = 1\n"
        "            break\n"
        "    if not found:\n"
        "        f.write('NOT_FOUND\\n')\n"
        "    f.close()\n"
        "except Exception, e:\n"
        "    f2 = open('" + tempPath + "', 'w')\n"
        "    f2.write('ERROR: %s\\n' % str(e))\n"
        "    f2.close()\n";

    fprintf(stderr, "[DataExtractor] get_info(%s, %s): running script...\n", type, key);
    std::string result = m_bridge->run_script_with_output(script.c_str());

    if (result.empty()) {
        fprintf(stderr, "[DataExtractor] get_info: empty result\n");
        return {};
    }

    if (result.rfind("ERROR:", 0) == 0) {
        fprintf(stderr, "[DataExtractor] get_info: script error: %s\n", result.c_str());
        return {};
    }

    if (result.rfind("NOT_FOUND", 0) == 0) {
        fprintf(stderr, "[DataExtractor] get_info: key not found: %s\n", key);
        return {};
    }

    // Strip trailing newline/CR
    std::string line = result;
    while (!line.empty() && (line.back() == '\n' || line.back() == '\r'))
        line.pop_back();

    // Build JSON object based on type
    yyjson_mut_doc* doc = yyjson_mut_doc_new(nullptr);
    yyjson_mut_val* obj = yyjson_mut_obj(doc);
    yyjson_mut_doc_set_root(doc, obj);

    if (strcmp(type, "tech") == 0) {
        // type \t cost \t era
        size_t tab1 = line.find('\t');
        size_t tab2 = (tab1 != std::string::npos) ? line.find('\t', tab1 + 1) : std::string::npos;
        if (tab1 == std::string::npos || tab2 == std::string::npos) {
            yyjson_mut_doc_free(doc);
            fprintf(stderr, "[DataExtractor] get_info(tech): parse error: %s\n", line.c_str());
            return {};
        }
        std::string type_str = line.substr(0, tab1);
        int cost = 0, era = 0;
        try {
            cost = std::stoi(line.substr(tab1 + 1, tab2 - tab1 - 1));
            era  = std::stoi(line.substr(tab2 + 1));
        } catch (...) {
            yyjson_mut_doc_free(doc);
            fprintf(stderr, "[DataExtractor] get_info(tech): int parse error: %s\n", line.c_str());
            return {};
        }
        yyjson_mut_obj_add_strcpy(doc, obj, "type", type_str.c_str());
        yyjson_mut_obj_add_int(doc, obj, "cost", cost);
        yyjson_mut_obj_add_int(doc, obj, "era", era);

    } else if (strcmp(type, "building") == 0) {
        // type \t productionCost \t artDefineTag
        size_t tab1 = line.find('\t');
        size_t tab2 = (tab1 != std::string::npos) ? line.find('\t', tab1 + 1) : std::string::npos;
        if (tab1 == std::string::npos || tab2 == std::string::npos) {
            yyjson_mut_doc_free(doc);
            fprintf(stderr, "[DataExtractor] get_info(building): parse error: %s\n", line.c_str());
            return {};
        }
        std::string type_str = line.substr(0, tab1);
        int cost = 0;
        try {
            cost = std::stoi(line.substr(tab1 + 1, tab2 - tab1 - 1));
        } catch (...) {
            yyjson_mut_doc_free(doc);
            fprintf(stderr, "[DataExtractor] get_info(building): int parse error: %s\n", line.c_str());
            return {};
        }
        std::string art_str = line.substr(tab2 + 1);
        yyjson_mut_obj_add_strcpy(doc, obj, "type", type_str.c_str());
        yyjson_mut_obj_add_int(doc, obj, "productionCost", cost);
        yyjson_mut_obj_add_strcpy(doc, obj, "artDefineTag", art_str.c_str());

    } else if (strcmp(type, "unit") == 0) {
        // type \t combat \t moves \t cost
        size_t tab1 = line.find('\t');
        size_t tab2 = (tab1 != std::string::npos) ? line.find('\t', tab1 + 1) : std::string::npos;
        size_t tab3 = (tab2 != std::string::npos) ? line.find('\t', tab2 + 1) : std::string::npos;
        if (tab1 == std::string::npos || tab2 == std::string::npos || tab3 == std::string::npos) {
            yyjson_mut_doc_free(doc);
            fprintf(stderr, "[DataExtractor] get_info(unit): parse error: %s\n", line.c_str());
            return {};
        }
        std::string type_str = line.substr(0, tab1);
        int combat = 0, moves = 0, cost = 0;
        try {
            combat = std::stoi(line.substr(tab1 + 1, tab2 - tab1 - 1));
            moves  = std::stoi(line.substr(tab2 + 1, tab3 - tab2 - 1));
            cost   = std::stoi(line.substr(tab3 + 1));
        } catch (...) {
            yyjson_mut_doc_free(doc);
            fprintf(stderr, "[DataExtractor] get_info(unit): int parse error: %s\n", line.c_str());
            return {};
        }
        yyjson_mut_obj_add_strcpy(doc, obj, "type", type_str.c_str());
        yyjson_mut_obj_add_int(doc, obj, "combat", combat);
        yyjson_mut_obj_add_int(doc, obj, "moves", moves);
        yyjson_mut_obj_add_int(doc, obj, "cost", cost);

    } else {
        // Simple types (promotion, bonus, civilization, era): just "type"
        yyjson_mut_obj_add_strcpy(doc, obj, "type", line.c_str());
    }

    size_t out_len = 0;
    char* out_str = yyjson_mut_write(doc, 0, &out_len);
    yyjson_mut_doc_free(doc);

    if (!out_str) {
        fprintf(stderr, "[DataExtractor] get_info: yyjson write failed\n");
        return {};
    }

    std::string result_json(out_str, out_len);
    free(out_str);

    fprintf(stderr, "[DataExtractor] get_info(%s, %s): %s\n", type, key, result_json.c_str());
    return result_json;
}

// ---------------------------------------------------------------------------
// get_art_info — single art info lookup by type and key tag
// ---------------------------------------------------------------------------
std::string DataExtractor::get_art_info(const char* type, const char* key) {
    if (!m_bridge || !m_bridge->is_initialized()) {
        fprintf(stderr, "[DataExtractor] get_art_info: bridge not initialized\n");
        return {};
    }
    if (!type || !key || type[0] == '\0' || key[0] == '\0') {
        fprintf(stderr, "[DataExtractor] get_art_info: empty type or key\n");
        return {};
    }

    // Map type to CyArtFileMgr method name
    std::string method;
    if (strcmp(type, "unit") == 0) {
        method = "getUnitArtInfo";
    } else if (strcmp(type, "building") == 0) {
        method = "getBuildingArtInfo";
    } else {
        fprintf(stderr, "[DataExtractor] get_art_info: unknown type '%s'\n", type);
        return {};
    }

    return lookup_art_info(method.c_str(), key);
}

// ---------------------------------------------------------------------------
// extract_techs — run Python 2.4 script, parse TSV, build JSON array
// ---------------------------------------------------------------------------
std::string DataExtractor::extract_techs() {
    std::string tempPath = m_bridge->get_temp_file_path();

    // Python 2.4 script: iterate all tech infos, write TAB-separated values
    // Fields: type (str), research_cost (int), era (int)
    std::string script =
        "try:\n"
        "    from CvPythonExtensions import *\n"
        "    gc = CyGlobalContext()\n"
        "    f = open('" + tempPath + "', 'w')\n"
        "    for i in range(gc.getNumTechInfos()):\n"
        "        info = gc.getTechInfo(i)\n"
        "        if info:\n"
        "            f.write('%s\\t%d\\t%d\\n' % (info.getType(), info.getResearchCost(), info.getEra()))\n"
        "    f.close()\n"
        "except Exception, e:\n"
        "    f2 = open('" + tempPath + "', 'w')\n"
        "    f2.write('ERROR: %s\\n' % str(e))\n"
        "    f2.close()\n";

    fprintf(stderr, "[DataExtractor] extract_techs: running script...\n");
    std::string result = m_bridge->run_script_with_output(script.c_str());

    if (result.empty()) {
        fprintf(stderr, "[DataExtractor] extract_techs: empty result\n");
        return {};
    }

    // Check for script-level error
    if (result.rfind("ERROR:", 0) == 0) {
        fprintf(stderr, "[DataExtractor] extract_techs: script error: %s\n", result.c_str());
        return {};
    }

    // Parse TSV lines into JSON array using yyjson
    yyjson_mut_doc* doc = yyjson_mut_doc_new(nullptr);
    yyjson_mut_val* arr = yyjson_mut_arr(doc);
    yyjson_mut_doc_set_root(doc, arr);

    std::istringstream stream(result);
    std::string line;
    int count = 0;

    while (std::getline(stream, line)) {
        if (line.empty()) continue;

        // Split by tab: type \t cost \t era
        size_t tab1 = line.find('\t');
        if (tab1 == std::string::npos) continue;
        size_t tab2 = line.find('\t', tab1 + 1);
        if (tab2 == std::string::npos) continue;

        std::string type_str = line.substr(0, tab1);
        std::string cost_str = line.substr(tab1 + 1, tab2 - tab1 - 1);
        std::string era_str  = line.substr(tab2 + 1);

        // Trim trailing \r if present (Windows line endings)
        if (!era_str.empty() && era_str.back() == '\r')
            era_str.pop_back();

        int cost = 0;
        int era  = 0;
        try {
            cost = std::stoi(cost_str);
            era  = std::stoi(era_str);
        } catch (...) {
            fprintf(stderr, "[DataExtractor] extract_techs: parse error on line: %s\n", line.c_str());
            continue;
        }

        yyjson_mut_val* obj = yyjson_mut_obj(doc);
        yyjson_mut_obj_add_strcpy(doc, obj, "type", type_str.c_str());
        yyjson_mut_obj_add_int(doc, obj, "cost", cost);
        yyjson_mut_obj_add_int(doc, obj, "era", era);
        yyjson_mut_arr_append(arr, obj);
        count++;
    }

    fprintf(stderr, "[DataExtractor] extract_techs: parsed %d techs\n", count);

    // Serialize to JSON string
    size_t out_len = 0;
    char* out_str = yyjson_mut_write(doc, 0, &out_len);
    yyjson_mut_doc_free(doc);

    if (!out_str) {
        fprintf(stderr, "[DataExtractor] extract_techs: yyjson write failed\n");
        return {};
    }

    std::string json_result(out_str, out_len);
    free(out_str);
    return json_result;
}

// ---------------------------------------------------------------------------
// extract_buildings — buildings have type, productionCost, artDefineTag
// ---------------------------------------------------------------------------
std::string DataExtractor::extract_buildings() {
    std::string tempPath = m_bridge->get_temp_file_path();

    std::string script =
        "try:\n"
        "    from CvPythonExtensions import *\n"
        "    gc = CyGlobalContext()\n"
        "    f = open('" + tempPath + "', 'w')\n"
        "    for i in range(gc.getNumBuildingInfos()):\n"
        "        info = gc.getBuildingInfo(i)\n"
        "        if info:\n"
        "            f.write('%s\\t%d\\t%s\\n' % (info.getType(), info.getProductionCost(), info.getArtDefineTag()))\n"
        "    f.close()\n"
        "except Exception, e:\n"
        "    f2 = open('" + tempPath + "', 'w')\n"
        "    f2.write('ERROR: %s\\n' % str(e))\n"
        "    f2.close()\n";

    fprintf(stderr, "[DataExtractor] extract_buildings: running script...\n");
    std::string result = m_bridge->run_script_with_output(script.c_str());

    if (result.empty()) {
        fprintf(stderr, "[DataExtractor] extract_buildings: empty result\n");
        return {};
    }

    if (result.rfind("ERROR:", 0) == 0) {
        fprintf(stderr, "[DataExtractor] extract_buildings: script error: %s\n", result.c_str());
        return {};
    }

    yyjson_mut_doc* doc = yyjson_mut_doc_new(nullptr);
    yyjson_mut_val* arr = yyjson_mut_arr(doc);
    yyjson_mut_doc_set_root(doc, arr);

    std::istringstream stream(result);
    std::string line;
    int count = 0;

    while (std::getline(stream, line)) {
        if (line.empty()) continue;

        // Split by tab: type \t productionCost \t artDefineTag
        size_t tab1 = line.find('\t');
        if (tab1 == std::string::npos) continue;
        size_t tab2 = line.find('\t', tab1 + 1);
        if (tab2 == std::string::npos) continue;

        std::string type_str = line.substr(0, tab1);
        std::string cost_str = line.substr(tab1 + 1, tab2 - tab1 - 1);
        std::string art_str  = line.substr(tab2 + 1);

        // Trim trailing \r if present (Windows line endings)
        if (!art_str.empty() && art_str.back() == '\r')
            art_str.pop_back();

        int cost = 0;
        try {
            cost = std::stoi(cost_str);
        } catch (...) {
            fprintf(stderr, "[DataExtractor] extract_buildings: parse error on line: %s\n", line.c_str());
            continue;
        }

        yyjson_mut_val* obj = yyjson_mut_obj(doc);
        yyjson_mut_obj_add_strcpy(doc, obj, "type", type_str.c_str());
        yyjson_mut_obj_add_int(doc, obj, "productionCost", cost);
        yyjson_mut_obj_add_strcpy(doc, obj, "artDefineTag", art_str.c_str());
        yyjson_mut_arr_append(arr, obj);
        count++;
    }

    fprintf(stderr, "[DataExtractor] extract_buildings: parsed %d buildings\n", count);

    size_t out_len = 0;
    char* out_str = yyjson_mut_write(doc, 0, &out_len);
    yyjson_mut_doc_free(doc);

    if (!out_str) {
        fprintf(stderr, "[DataExtractor] extract_buildings: yyjson write failed\n");
        return {};
    }

    std::string json_result(out_str, out_len);
    free(out_str);
    return json_result;
}

// ---------------------------------------------------------------------------
// extract_units — units have type, combat, moves, productionCost
// ---------------------------------------------------------------------------
std::string DataExtractor::extract_units() {
    std::string tempPath = m_bridge->get_temp_file_path();

    std::string script =
        "try:\n"
        "    from CvPythonExtensions import *\n"
        "    gc = CyGlobalContext()\n"
        "    f = open('" + tempPath + "', 'w')\n"
        "    for i in range(gc.getNumUnitInfos()):\n"
        "        info = gc.getUnitInfo(i)\n"
        "        if info:\n"
        "            f.write('%s\\t%d\\t%d\\t%d\\n' % (info.getType(), info.getCombat(), info.getMoves(), info.getProductionCost()))\n"
        "    f.close()\n"
        "except Exception, e:\n"
        "    f2 = open('" + tempPath + "', 'w')\n"
        "    f2.write('ERROR: %s\\n' % str(e))\n"
        "    f2.close()\n";

    fprintf(stderr, "[DataExtractor] extract_units: running script...\n");
    std::string result = m_bridge->run_script_with_output(script.c_str());

    if (result.empty()) {
        fprintf(stderr, "[DataExtractor] extract_units: empty result\n");
        return {};
    }

    if (result.rfind("ERROR:", 0) == 0) {
        fprintf(stderr, "[DataExtractor] extract_units: script error: %s\n", result.c_str());
        return {};
    }

    yyjson_mut_doc* doc = yyjson_mut_doc_new(nullptr);
    yyjson_mut_val* arr = yyjson_mut_arr(doc);
    yyjson_mut_doc_set_root(doc, arr);

    std::istringstream stream(result);
    std::string line;
    int count = 0;

    while (std::getline(stream, line)) {
        if (line.empty()) continue;

        // Split by tab: type \t combat \t moves \t cost
        size_t tab1 = line.find('\t');
        if (tab1 == std::string::npos) continue;
        size_t tab2 = line.find('\t', tab1 + 1);
        if (tab2 == std::string::npos) continue;
        size_t tab3 = line.find('\t', tab2 + 1);
        if (tab3 == std::string::npos) continue;

        std::string type_str   = line.substr(0, tab1);
        std::string combat_str = line.substr(tab1 + 1, tab2 - tab1 - 1);
        std::string moves_str  = line.substr(tab2 + 1, tab3 - tab2 - 1);
        std::string cost_str   = line.substr(tab3 + 1);

        // Trim trailing \r if present (Windows line endings)
        if (!cost_str.empty() && cost_str.back() == '\r')
            cost_str.pop_back();

        int combat = 0, moves = 0, cost = 0;
        try {
            combat = std::stoi(combat_str);
            moves  = std::stoi(moves_str);
            cost   = std::stoi(cost_str);
        } catch (...) {
            fprintf(stderr, "[DataExtractor] extract_units: parse error on line: %s\n", line.c_str());
            continue;
        }

        yyjson_mut_val* obj = yyjson_mut_obj(doc);
        yyjson_mut_obj_add_strcpy(doc, obj, "type", type_str.c_str());
        yyjson_mut_obj_add_int(doc, obj, "combat", combat);
        yyjson_mut_obj_add_int(doc, obj, "moves", moves);
        yyjson_mut_obj_add_int(doc, obj, "cost", cost);
        yyjson_mut_arr_append(arr, obj);
        count++;
    }

    fprintf(stderr, "[DataExtractor] extract_units: parsed %d units\n", count);

    size_t out_len = 0;
    char* out_str = yyjson_mut_write(doc, 0, &out_len);
    yyjson_mut_doc_free(doc);

    if (!out_str) {
        fprintf(stderr, "[DataExtractor] extract_units: yyjson write failed\n");
        return {};
    }

    std::string json_result(out_str, out_len);
    free(out_str);
    return json_result;
}

// ---------------------------------------------------------------------------
// extract_simple_infos — shared helper for types with only a "type" field
// Used for: promotion, bonus, civilization, era
// ---------------------------------------------------------------------------
std::string DataExtractor::extract_simple_infos(const char* pyCountMethod, const char* pyGetMethod) {
    std::string tempPath = m_bridge->get_temp_file_path();

    std::string script =
        "try:\n"
        "    from CvPythonExtensions import *\n"
        "    gc = CyGlobalContext()\n"
        "    f = open('" + tempPath + "', 'w')\n"
        "    for i in range(gc." + std::string(pyCountMethod) + "()):\n"
        "        info = gc." + std::string(pyGetMethod) + "(i)\n"
        "        if info:\n"
        "            f.write('%s\\n' % info.getType())\n"
        "    f.close()\n"
        "except Exception, e:\n"
        "    f2 = open('" + tempPath + "', 'w')\n"
        "    f2.write('ERROR: %s\\n' % str(e))\n"
        "    f2.close()\n";

    fprintf(stderr, "[DataExtractor] extract_simple_infos(%s): running script...\n", pyGetMethod);
    std::string result = m_bridge->run_script_with_output(script.c_str());

    if (result.empty()) {
        fprintf(stderr, "[DataExtractor] extract_simple_infos(%s): empty result\n", pyGetMethod);
        return {};
    }

    if (result.rfind("ERROR:", 0) == 0) {
        fprintf(stderr, "[DataExtractor] extract_simple_infos(%s): script error: %s\n", pyGetMethod, result.c_str());
        return {};
    }

    yyjson_mut_doc* doc = yyjson_mut_doc_new(nullptr);
    yyjson_mut_val* arr = yyjson_mut_arr(doc);
    yyjson_mut_doc_set_root(doc, arr);

    std::istringstream stream(result);
    std::string line;
    int count = 0;

    while (std::getline(stream, line)) {
        if (line.empty()) continue;

        // Trim trailing \r if present (Windows line endings)
        if (!line.empty() && line.back() == '\r')
            line.pop_back();
        if (line.empty()) continue;

        yyjson_mut_val* obj = yyjson_mut_obj(doc);
        yyjson_mut_obj_add_strcpy(doc, obj, "type", line.c_str());
        yyjson_mut_arr_append(arr, obj);
        count++;
    }

    fprintf(stderr, "[DataExtractor] extract_simple_infos(%s): parsed %d items\n", pyGetMethod, count);

    size_t out_len = 0;
    char* out_str = yyjson_mut_write(doc, 0, &out_len);
    yyjson_mut_doc_free(doc);

    if (!out_str) {
        fprintf(stderr, "[DataExtractor] extract_simple_infos(%s): yyjson write failed\n", pyGetMethod);
        return {};
    }

    std::string json_result(out_str, out_len);
    free(out_str);
    return json_result;
}

// ---------------------------------------------------------------------------
// extract_unit_art — iterate all unit art infos via CyGlobalContext
// TSV: type \t nif \t kfm \t scale
// ---------------------------------------------------------------------------
std::string DataExtractor::extract_unit_art() {
    std::string tempPath = m_bridge->get_temp_file_path();

    std::string script =
        "try:\n"
        "    from CvPythonExtensions import *\n"
        "    gc = CyGlobalContext()\n"
        "    f = open('" + tempPath + "', 'w')\n"
        "    for i in range(gc.getNumUnitArtInfos()):\n"
        "        info = gc.getUnitArtInfo(i)\n"
        "        if info:\n"
        "            f.write('%s\\t%s\\t%s\\t%s\\n' % (info.getTag(), info.getNIF(), info.getKFM(), str(info.getScale())))\n"
        "    f.close()\n"
        "except Exception, e:\n"
        "    f2 = open('" + tempPath + "', 'w')\n"
        "    f2.write('ERROR: %s\\n' % str(e))\n"
        "    f2.close()\n";

    fprintf(stderr, "[DataExtractor] extract_unit_art: running script...\n");
    std::string result = m_bridge->run_script_with_output(script.c_str());

    if (result.empty()) {
        fprintf(stderr, "[DataExtractor] extract_unit_art: empty result\n");
        return {};
    }

    if (result.rfind("ERROR:", 0) == 0) {
        fprintf(stderr, "[DataExtractor] extract_unit_art: script error: %s\n", result.c_str());
        return {};
    }

    yyjson_mut_doc* doc = yyjson_mut_doc_new(nullptr);
    yyjson_mut_val* arr = yyjson_mut_arr(doc);
    yyjson_mut_doc_set_root(doc, arr);

    std::istringstream stream(result);
    std::string line;
    int count = 0;

    while (std::getline(stream, line)) {
        if (line.empty()) continue;

        // Split by tab: type \t nif \t kfm \t scale
        size_t tab1 = line.find('\t');
        if (tab1 == std::string::npos) continue;
        size_t tab2 = line.find('\t', tab1 + 1);
        if (tab2 == std::string::npos) continue;
        size_t tab3 = line.find('\t', tab2 + 1);
        if (tab3 == std::string::npos) continue;

        std::string type_str  = line.substr(0, tab1);
        std::string nif_str   = line.substr(tab1 + 1, tab2 - tab1 - 1);
        std::string kfm_str   = line.substr(tab2 + 1, tab3 - tab2 - 1);
        std::string scale_str = line.substr(tab3 + 1);

        if (!scale_str.empty() && scale_str.back() == '\r')
            scale_str.pop_back();

        double scale = 1.0;
        try {
            scale = std::stod(scale_str);
        } catch (...) {
            fprintf(stderr, "[DataExtractor] extract_unit_art: scale parse error on line: %s\n", line.c_str());
        }

        yyjson_mut_val* obj = yyjson_mut_obj(doc);
        yyjson_mut_obj_add_strcpy(doc, obj, "type", type_str.c_str());
        yyjson_mut_obj_add_strcpy(doc, obj, "nif", nif_str.c_str());
        yyjson_mut_obj_add_strcpy(doc, obj, "kfm", kfm_str.c_str());
        yyjson_mut_obj_add_real(doc, obj, "scale", scale);
        yyjson_mut_arr_append(arr, obj);
        count++;
    }

    fprintf(stderr, "[DataExtractor] extract_unit_art: parsed %d items\n", count);

    size_t out_len = 0;
    char* out_str = yyjson_mut_write(doc, 0, &out_len);
    yyjson_mut_doc_free(doc);

    if (!out_str) {
        fprintf(stderr, "[DataExtractor] extract_unit_art: yyjson write failed\n");
        return {};
    }

    std::string result_json(out_str, out_len);
    free(out_str);
    return result_json;
}

// ---------------------------------------------------------------------------
// extract_building_art — iterate all building art infos via CyGlobalContext
// TSV: type \t nif \t kfm \t scale
// ---------------------------------------------------------------------------
std::string DataExtractor::extract_building_art() {
    std::string tempPath = m_bridge->get_temp_file_path();

    std::string script =
        "try:\n"
        "    from CvPythonExtensions import *\n"
        "    gc = CyGlobalContext()\n"
        "    f = open('" + tempPath + "', 'w')\n"
        "    for i in range(gc.getNumBuildingArtInfos()):\n"
        "        info = gc.getBuildingArtInfo(i)\n"
        "        if info:\n"
        "            f.write('%s\\t%s\\t%s\\t%s\\n' % (info.getTag(), info.getNIF(), info.getKFM(), str(info.getScale())))\n"
        "    f.close()\n"
        "except Exception, e:\n"
        "    f2 = open('" + tempPath + "', 'w')\n"
        "    f2.write('ERROR: %s\\n' % str(e))\n"
        "    f2.close()\n";

    fprintf(stderr, "[DataExtractor] extract_building_art: running script...\n");
    std::string result = m_bridge->run_script_with_output(script.c_str());

    if (result.empty()) {
        fprintf(stderr, "[DataExtractor] extract_building_art: empty result\n");
        return {};
    }

    if (result.rfind("ERROR:", 0) == 0) {
        fprintf(stderr, "[DataExtractor] extract_building_art: script error: %s\n", result.c_str());
        return {};
    }

    yyjson_mut_doc* doc = yyjson_mut_doc_new(nullptr);
    yyjson_mut_val* arr = yyjson_mut_arr(doc);
    yyjson_mut_doc_set_root(doc, arr);

    std::istringstream stream(result);
    std::string line;
    int count = 0;

    while (std::getline(stream, line)) {
        if (line.empty()) continue;

        // Split by tab: type \t nif \t kfm \t scale
        size_t tab1 = line.find('\t');
        if (tab1 == std::string::npos) continue;
        size_t tab2 = line.find('\t', tab1 + 1);
        if (tab2 == std::string::npos) continue;
        size_t tab3 = line.find('\t', tab2 + 1);
        if (tab3 == std::string::npos) continue;

        std::string type_str  = line.substr(0, tab1);
        std::string nif_str   = line.substr(tab1 + 1, tab2 - tab1 - 1);
        std::string kfm_str   = line.substr(tab2 + 1, tab3 - tab2 - 1);
        std::string scale_str = line.substr(tab3 + 1);

        if (!scale_str.empty() && scale_str.back() == '\r')
            scale_str.pop_back();

        double scale = 1.0;
        try {
            scale = std::stod(scale_str);
        } catch (...) {
            fprintf(stderr, "[DataExtractor] extract_building_art: scale parse error on line: %s\n", line.c_str());
        }

        yyjson_mut_val* obj = yyjson_mut_obj(doc);
        yyjson_mut_obj_add_strcpy(doc, obj, "type", type_str.c_str());
        yyjson_mut_obj_add_strcpy(doc, obj, "nif", nif_str.c_str());
        yyjson_mut_obj_add_strcpy(doc, obj, "kfm", kfm_str.c_str());
        yyjson_mut_obj_add_real(doc, obj, "scale", scale);
        yyjson_mut_arr_append(arr, obj);
        count++;
    }

    fprintf(stderr, "[DataExtractor] extract_building_art: parsed %d items\n", count);

    size_t out_len = 0;
    char* out_str = yyjson_mut_write(doc, 0, &out_len);
    yyjson_mut_doc_free(doc);

    if (!out_str) {
        fprintf(stderr, "[DataExtractor] extract_building_art: yyjson write failed\n");
        return {};
    }

    std::string result_json(out_str, out_len);
    free(out_str);
    return result_json;
}

// ---------------------------------------------------------------------------
// lookup_art_info — single art info lookup via CyArtFileMgr by string tag
// Returns JSON object: {"type":"...","nif":"...","kfm":"...","scale":0.61}
// ---------------------------------------------------------------------------
std::string DataExtractor::lookup_art_info(const char* artMgrMethod, const char* key) {
    std::string tempPath = m_bridge->get_temp_file_path();
    std::string keyStr(key);

    std::string script =
        "try:\n"
        "    from CvPythonExtensions import *\n"
        "    mgr = CyArtFileMgr()\n"
        "    info = mgr." + std::string(artMgrMethod) + "('" + keyStr + "')\n"
        "    f = open('" + tempPath + "', 'w')\n"
        "    if info:\n"
        "        f.write('%s\\t%s\\t%s\\t%s\\n' % (info.getTag(), info.getNIF(), info.getKFM(), str(info.getScale())))\n"
        "    else:\n"
        "        f.write('NOT_FOUND\\n')\n"
        "    f.close()\n"
        "except Exception, e:\n"
        "    f2 = open('" + tempPath + "', 'w')\n"
        "    f2.write('ERROR: %s\\n' % str(e))\n"
        "    f2.close()\n";

    fprintf(stderr, "[DataExtractor] lookup_art_info(%s, %s): running script...\n", artMgrMethod, key);
    std::string result = m_bridge->run_script_with_output(script.c_str());

    if (result.empty()) {
        fprintf(stderr, "[DataExtractor] lookup_art_info: empty result\n");
        return {};
    }

    if (result.rfind("ERROR:", 0) == 0) {
        fprintf(stderr, "[DataExtractor] lookup_art_info: script error: %s\n", result.c_str());
        return {};
    }

    if (result.rfind("NOT_FOUND", 0) == 0) {
        fprintf(stderr, "[DataExtractor] lookup_art_info: key not found: %s\n", key);
        return {};
    }

    // Parse single TSV line: type \t nif \t kfm \t scale
    std::string line = result;
    while (!line.empty() && (line.back() == '\n' || line.back() == '\r'))
        line.pop_back();

    size_t tab1 = line.find('\t');
    if (tab1 == std::string::npos) return {};
    size_t tab2 = line.find('\t', tab1 + 1);
    if (tab2 == std::string::npos) return {};
    size_t tab3 = line.find('\t', tab2 + 1);
    if (tab3 == std::string::npos) return {};

    std::string type_str  = line.substr(0, tab1);
    std::string nif_str   = line.substr(tab1 + 1, tab2 - tab1 - 1);
    std::string kfm_str   = line.substr(tab2 + 1, tab3 - tab2 - 1);
    std::string scale_str = line.substr(tab3 + 1);

    double scale = 1.0;
    try {
        scale = std::stod(scale_str);
    } catch (...) {
        fprintf(stderr, "[DataExtractor] lookup_art_info: scale parse error: %s\n", scale_str.c_str());
    }

    yyjson_mut_doc* doc = yyjson_mut_doc_new(nullptr);
    yyjson_mut_val* obj = yyjson_mut_obj(doc);
    yyjson_mut_doc_set_root(doc, obj);

    yyjson_mut_obj_add_strcpy(doc, obj, "type", type_str.c_str());
    yyjson_mut_obj_add_strcpy(doc, obj, "nif", nif_str.c_str());
    yyjson_mut_obj_add_strcpy(doc, obj, "kfm", kfm_str.c_str());
    yyjson_mut_obj_add_real(doc, obj, "scale", scale);

    size_t out_len = 0;
    char* out_str = yyjson_mut_write(doc, 0, &out_len);
    yyjson_mut_doc_free(doc);

    if (!out_str) return {};

    std::string result_json(out_str, out_len);
    free(out_str);
    return result_json;
}
