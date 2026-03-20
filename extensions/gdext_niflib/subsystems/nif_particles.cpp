// =============================================================================
// File:              nif_particles.cpp
// Author(s):         Chrischn89
// Godot Version:     4.5
// Description:
//   NIF particle system placeholder. Creates a billboard sprite at the particle
//   system's position using the first texture from its properties. Full particle
//   simulation (GPUParticles3D) is deferred to a future milestone.
//
// License:
//   Released under the terms of the GNU General Public License version 3.0
// =============================================================================
#include "gdext_niflib.hpp"
#include "nif_coordinate.hpp"

#include <obj/NiParticles.h>
#include <obj/NiParticleSystem.h>
#include <obj/NiTexturingProperty.h>
#include <obj/NiSourceTexture.h>
#include <obj/NiProperty.h>

#include <godot_cpp/variant/utility_functions.hpp>
#include <godot_cpp/classes/sprite3d.hpp>
#include <godot_cpp/classes/standard_material3d.hpp>

using namespace godot;
using namespace Niflib;

void GdextNiflib::process_ni_particle_system(NiParticlesRef particles,
    Node3D* parent, const String& base_path) {
    if (!particles || !parent) return;

    std::string name = nif_display_name(StaticCast<NiObject>(particles));
    std::string type = particles->GetType().GetTypeName();

    // Try to find a texture from the particle's NiTexturingProperty
    Ref<ImageTexture> tex;
    auto properties = particles->GetProperties();
    for (const auto& prop : properties) {
        if (prop == NULL) continue;
        auto tex_prop = DynamicCast<NiTexturingProperty>(prop);
        if (!tex_prop) continue;
        // Try BASE_MAP (slot 0) first, then DECAL_0 (slot 8)
        int slots[] = {0, 8};
        for (int slot : slots) {
            if (!tex_prop->HasTexture(slot)) continue;
            TexDesc td = tex_prop->GetTexture(slot);
            if (td.source != NULL && td.source->IsTextureExternal()) {
                tex = load_dds_texture(base_path, td.source->GetTextureFileName());
                if (tex.is_valid()) break;
            }
        }
        if (tex.is_valid()) break;
    }

    // Create a billboard Sprite3D as a placeholder for the particle effect
    Sprite3D* sprite = memnew(Sprite3D);
    sprite->set_name(String::utf8(name.c_str()));
    apply_nif_transform(StaticCast<NiAVObject>(particles), sprite);

    // Billboard mode: always face camera
    sprite->set_billboard_mode(StandardMaterial3D::BILLBOARD_ENABLED);

    if (tex.is_valid()) {
        sprite->set_texture(tex);
        // Scale based on texture size (assume ~1 pixel = 0.1 world units)
        float w = tex->get_width() * 0.1f;
        float h = tex->get_height() * 0.1f;
        sprite->set_pixel_size(0.1f);
    } else {
        // No texture — use a small default size
        sprite->set_pixel_size(0.5f);
    }

    // Transparent rendering for particle textures (most have alpha)
    sprite->set_transparency(0.5f);
    sprite->set_alpha_cut_mode(Sprite3D::ALPHA_CUT_DISCARD);

    // Store particle type as metadata for future full implementation
    sprite->set_meta("nif_particle_type", String::utf8(type.c_str()));
    sprite->set_meta("nif_particle_placeholder", true);

    parent->add_child(sprite);
    UtilityFunctions::print("[PARTICLE] ", String::utf8(type.c_str()),
        " placeholder: '", String::utf8(name.c_str()),
        "' tex=", tex.is_valid() ? "yes" : "none");
}
