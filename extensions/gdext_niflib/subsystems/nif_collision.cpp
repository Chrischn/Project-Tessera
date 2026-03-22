// =============================================================================
// File:              nif_collision.cpp
// Author(s):         Chrischn89
// Godot Version:     4.5
// Description:
//   Collision object stub for NiCollisionObject/NiCollisionData
//
// License:
//   Released under the terms of the GNU General Public License version 3.0
// =============================================================================
#include "gdext_niflib.hpp"

#include <obj/NiAVObject.h>
#include <obj/NiCollisionObject.h>
#include <obj/NiCollisionData.h>

#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;
using namespace Niflib;

void GdextNiflib::process_ni_collision(NiAVObjectRef obj, Node3D* godot_node) {
    if (!obj || !godot_node) return;

    // Use explicit Niflib::Ref to avoid ambiguity with godot::Ref
    Niflib::Ref<NiCollisionObject> col = obj->GetCollisionObject();
    if (col == NULL) return;

    std::string type = col->GetType().GetTypeName();

    UtilityFunctions::print("[STUB] ", String::utf8(type.c_str()), " on '",
        String::utf8(nif_display_name(StaticCast<NiObject>(obj)).c_str()),
        "' — collision object, skipped");
}
