// =============================================================================
// File:              nif_extra_data.cpp
// Author(s):         Chrischn89
// Godot Version:     4.5
// Description:
//   Extra data processing: text keys, string data, metadata extraction
//
// License:
//   Released under the terms of the GNU General Public License version 3.0
// =============================================================================
#include "gdext_niflib.hpp"

#include <obj/NiObjectNET.h>
#include <obj/NiExtraData.h>
#include <obj/NiTextKeyExtraData.h>
#include <obj/NiStringExtraData.h>
#include <obj/NiIntegerExtraData.h>
#include <obj/NiFloatExtraData.h>
#include <obj/NiBinaryExtraData.h>
#include <obj/NiBooleanExtraData.h>

#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;
using namespace Niflib;

void GdextNiflib::process_extra_data(NiObjectNETRef obj, Node3D* godot_node) {
    if (!obj || !godot_node) return;

    auto extra_list = obj->GetExtraData();
    for (const auto& ed : extra_list) {
        if (ed == NULL) continue;

        if (auto tk = DynamicCast<NiTextKeyExtraData>(ed)) {
            // REAL IMPL: Store animation event markers as node metadata.
            // GDScript can query node.get_meta("nif_text_keys") for gameplay events.
            auto keys = tk->GetKeys();
            if (!keys.empty()) {
                Dictionary dict;
                for (const auto& k : keys) {
                    dict[k.time] = String::utf8(k.data.c_str());
                }
                godot_node->set_meta("nif_text_keys", dict);
                UtilityFunctions::print("[EXTRA] NiTextKeyExtraData: ",
                    (int)keys.size(), " text keys stored as metadata");
            }
        }
        else if (auto se = DynamicCast<NiStringExtraData>(ed)) {
            // REAL IMPL: Store "Prn" (parent bone name for weapon attachment) as metadata.
            std::string name = se->GetName();
            std::string value = se->GetData();
            if (name == "Prn") {
                godot_node->set_meta("nif_prn", String::utf8(value.c_str()));
                UtilityFunctions::print("[EXTRA] NiStringExtraData 'Prn' = '",
                    String::utf8(value.c_str()), "'");
            } else {
                UtilityFunctions::print("[EXTRA] NiStringExtraData '",
                    String::utf8(name.c_str()), "' = '",
                    String::utf8(value.c_str()), "'");
            }
        }
        else if (DynamicCast<NiIntegerExtraData>(ed) != NULL) {
            UtilityFunctions::print("[STUB] NiIntegerExtraData: '",
                String::utf8(ed->GetName().c_str()), "' — integer metadata, skipped");
        }
        else if (DynamicCast<NiFloatExtraData>(ed) != NULL) {
            UtilityFunctions::print("[STUB] NiFloatExtraData: '",
                String::utf8(ed->GetName().c_str()), "' — float metadata, skipped");
        }
        else if (DynamicCast<NiBinaryExtraData>(ed) != NULL) {
            UtilityFunctions::print("[STUB] NiBinaryExtraData: '",
                String::utf8(ed->GetName().c_str()), "' — binary blob, skipped");
        }
        else if (DynamicCast<NiBooleanExtraData>(ed) != NULL) {
            UtilityFunctions::print("[STUB] NiBooleanExtraData: '",
                String::utf8(ed->GetName().c_str()), "' — boolean flag, skipped");
        }
        else {
            // Catch-all for unknown extra data types (includes BSXFlags and others)
            UtilityFunctions::print("[STUB] ", String::utf8(ed->GetType().GetTypeName().c_str()),
                ": '", String::utf8(ed->GetName().c_str()), "' — unhandled extra data, skipped");
        }
    }
}
