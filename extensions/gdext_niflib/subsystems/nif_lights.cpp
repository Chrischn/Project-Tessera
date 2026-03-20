// =============================================================================
// File:              nif_lights.cpp
// Author(s):         Chrischn89
// Godot Version:     4.5
// Description:
//   Light type stubs: NiPointLight, NiDirectionalLight, etc.
//
// License:
//   Released under the terms of the GNU General Public License version 3.0
// =============================================================================
#include "gdext_niflib.hpp"
#include "nif_coordinate.hpp"

#include <obj/NiLight.h>
#include <obj/NiPointLight.h>
#include <obj/NiDirectionalLight.h>
#include <obj/NiAmbientLight.h>
#include <obj/NiSpotLight.h>

#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;
using namespace Niflib;

void GdextNiflib::process_ni_light(NiLightRef light, Node3D* parent_godot) {
    if (!light || !parent_godot) return;

    std::string name = nif_display_name(StaticCast<NiObject>(light));
    Color3 diffuse = light->GetDiffuseColor();
    float dimmer = light->GetDimmer();

    if (DynamicCast<NiPointLight>(light) != NULL) {
        UtilityFunctions::print("[STUB] NiPointLight '", String::utf8(name.c_str()),
            "' diffuse=(", diffuse.r, ",", diffuse.g, ",", diffuse.b,
            ") dimmer=", dimmer);
    }
    else if (DynamicCast<NiDirectionalLight>(light) != NULL) {
        UtilityFunctions::print("[STUB] NiDirectionalLight '", String::utf8(name.c_str()),
            "' diffuse=(", diffuse.r, ",", diffuse.g, ",", diffuse.b, ")");
    }
    else if (DynamicCast<NiAmbientLight>(light) != NULL) {
        UtilityFunctions::print("[STUB] NiAmbientLight '", String::utf8(name.c_str()),
            "' diffuse=(", diffuse.r, ",", diffuse.g, ",", diffuse.b, ")");
    }
    else if (DynamicCast<NiSpotLight>(light) != NULL) {
        UtilityFunctions::print("[STUB] NiSpotLight '", String::utf8(name.c_str()),
            "' diffuse=(", diffuse.r, ",", diffuse.g, ",", diffuse.b, ")");
    }
    else {
        UtilityFunctions::print("[STUB] Unknown NiLight subclass '", String::utf8(name.c_str()), "'");
    }
}
