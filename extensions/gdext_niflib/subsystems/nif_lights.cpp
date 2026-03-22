// =============================================================================
// File:              nif_lights.cpp
// Author(s):         Chrischn89
// Godot Version:     4.5
// Description:
//   NIF light types -> Godot light nodes.
//   NiPointLight -> OmniLight3D, NiDirectionalLight -> DirectionalLight3D,
//   NiSpotLight -> SpotLight3D, NiAmbientLight -> metadata only.
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
#include <godot_cpp/classes/omni_light3d.hpp>
#include <godot_cpp/classes/directional_light3d.hpp>
#include <godot_cpp/classes/spot_light3d.hpp>

#include <cmath>

using namespace godot;
using namespace Niflib;

// Approximates a Godot OmniLight3D range from NIF attenuation coefficients.
// NIF uses: intensity = 1 / (constant + linear*d + quadratic*d^2)
// We solve for d where intensity drops to 'threshold' (1% = 0.01).
static float compute_light_range(float constant, float linear, float quadratic) {
    const float threshold = 0.01f;  // 1% intensity cutoff

    // Try quadratic first (most common for point/spot lights)
    if (quadratic > 0.0001f) {
        // Solve: threshold = 1 / (C + L*d + Q*d^2) -> d = sqrt((1/threshold - C) / Q)
        // Simplified: ignore constant/linear for the quadratic-dominant case
        float range = std::sqrt(1.0f / (threshold * quadratic));
        return std::min(range, 200.0f);  // cap at 200 units
    }
    // Linear fallback
    if (linear > 0.0001f) {
        float range = (1.0f / threshold - constant) / linear;
        return std::clamp(range, 1.0f, 200.0f);
    }
    // No attenuation — use default range
    return 50.0f;
}

void GdextNiflib::process_ni_light(NiLightRef light, Node3D* parent_godot) {
    if (!light || !parent_godot) return;

    std::string name = nif_display_name(StaticCast<NiObject>(light));
    Color3 diffuse = light->GetDiffuseColor();
    float dimmer = light->GetDimmer();
    godot::Color light_color(diffuse.r, diffuse.g, diffuse.b, 1.0f);

    // --- NiPointLight -> OmniLight3D ---
    if (auto point = DynamicCast<NiPointLight>(light)) {
        OmniLight3D* omni = memnew(OmniLight3D);
        omni->set_name(String::utf8(name.c_str()));
        apply_nif_transform(StaticCast<NiAVObject>(light), omni);

        omni->set_color(light_color);
        omni->set_param(Light3D::PARAM_ENERGY, dimmer);
        omni->set_param(Light3D::PARAM_ATTENUATION, 1.0f);  // linear falloff

        float range = compute_light_range(
            point->GetConstantAttenuation(),
            point->GetLinearAttenuation(),
            point->GetQuadraticAttenuation());
        omni->set_param(Light3D::PARAM_RANGE, range);

        // Shadows off by default (performance — enable via GDScript if needed)
        omni->set_shadow(false);

        parent_godot->add_child(omni);
        UtilityFunctions::print("[LIGHT] NiPointLight '", String::utf8(name.c_str()),
            "' color=(", diffuse.r, ",", diffuse.g, ",", diffuse.b,
            ") dimmer=", dimmer, " range=", range);
    }
    // --- NiSpotLight -> SpotLight3D ---
    else if (auto spot = DynamicCast<NiSpotLight>(light)) {
        SpotLight3D* spot_light = memnew(SpotLight3D);
        spot_light->set_name(String::utf8(name.c_str()));
        apply_nif_transform(StaticCast<NiAVObject>(light), spot_light);

        spot_light->set_color(light_color);
        spot_light->set_param(Light3D::PARAM_ENERGY, dimmer);
        spot_light->set_param(Light3D::PARAM_ATTENUATION, 1.0f);

        float range = compute_light_range(
            spot->GetConstantAttenuation(),
            spot->GetLinearAttenuation(),
            spot->GetQuadraticAttenuation());
        spot_light->set_param(Light3D::PARAM_RANGE, range);
        spot_light->set_param(Light3D::PARAM_SPOT_ANGLE, spot->GetCutoffAngle());
        spot_light->set_param(Light3D::PARAM_SPOT_ATTENUATION, spot->GetExponent());

        spot_light->set_shadow(false);

        parent_godot->add_child(spot_light);
        UtilityFunctions::print("[LIGHT] NiSpotLight '", String::utf8(name.c_str()),
            "' color=(", diffuse.r, ",", diffuse.g, ",", diffuse.b,
            ") dimmer=", dimmer, " range=", range,
            " cutoff=", spot->GetCutoffAngle(), " exponent=", spot->GetExponent());
    }
    // --- NiDirectionalLight -> DirectionalLight3D ---
    else if (DynamicCast<NiDirectionalLight>(light) != NULL) {
        DirectionalLight3D* dir_light = memnew(DirectionalLight3D);
        dir_light->set_name(String::utf8(name.c_str()));
        apply_nif_transform(StaticCast<NiAVObject>(light), dir_light);

        dir_light->set_color(light_color);
        dir_light->set_param(Light3D::PARAM_ENERGY, dimmer);

        dir_light->set_shadow(false);

        parent_godot->add_child(dir_light);
        UtilityFunctions::print("[LIGHT] NiDirectionalLight '", String::utf8(name.c_str()),
            "' color=(", diffuse.r, ",", diffuse.g, ",", diffuse.b,
            ") dimmer=", dimmer);
    }
    // --- NiAmbientLight -> metadata only ---
    else if (DynamicCast<NiAmbientLight>(light) != NULL) {
        // Civ4 ambient lights are typically near-black placeholders.
        // Store as metadata for GDScript; no Godot light node created.
        parent_godot->set_meta("nif_ambient_color", light_color);
        UtilityFunctions::print("[LIGHT] NiAmbientLight '", String::utf8(name.c_str()),
            "' color=(", diffuse.r, ",", diffuse.g, ",", diffuse.b,
            ") stored as metadata (no node created)");
    }
    else {
        UtilityFunctions::print("[LIGHT] Unknown NiLight subclass '",
            String::utf8(name.c_str()), "'");
    }
}
