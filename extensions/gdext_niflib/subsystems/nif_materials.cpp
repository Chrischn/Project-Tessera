// =============================================================================
// File:              nif_materials.cpp
// Author(s):         Chrischn89
// Godot Version:     4.5
// Description:
//   Material creation: NIF properties -> Godot StandardMaterial3D/ShaderMaterial
//
// License:
//   Released under the terms of the GNU General Public License version 3.0
// =============================================================================
#include "gdext_niflib.hpp"
#include "nif_coordinate.hpp"

#include <obj/NiProperty.h>
#include <obj/NiTexturingProperty.h>
#include <obj/NiMaterialProperty.h>
#include <obj/NiAlphaProperty.h>
#include <obj/NiVertexColorProperty.h>
#include <obj/NiSpecularProperty.h>
#include <obj/NiStencilProperty.h>
#include <obj/NiZBufferProperty.h>
#include <obj/NiDitherProperty.h>
#include <obj/NiShadeProperty.h>
#include <obj/NiWireframeProperty.h>
#include <obj/NiFogProperty.h>
#include <obj/NiSourceTexture.h>

#include <godot_cpp/variant/utility_functions.hpp>
#include <godot_cpp/classes/standard_material3d.hpp>
#include <godot_cpp/classes/shader_material.hpp>
#include <godot_cpp/classes/shader.hpp>
#include <godot_cpp/classes/resource_loader.hpp>
#include <godot_cpp/classes/image.hpp>
#include <godot_cpp/classes/image_texture.hpp>
#include <godot_cpp/classes/file_access.hpp>
#include <godot_cpp/classes/engine.hpp>

using namespace godot;
using namespace Niflib;

// Loads a DDS texture via VFS (FPK archives + loose files), with caching.
// Falls back to direct disk search if VFS is unavailable.
godot::Ref<ImageTexture> GdextNiflib::load_dds_texture(
    const String& base_path, const std::string& nif_tex_path)
{
    // Check cache first
    auto it = texture_cache.find(nif_tex_path);
    if (it != texture_cache.end()) return it->second;

    // Convert backslashes to forward slashes
    std::string tex_path = nif_tex_path;
    std::replace(tex_path.begin(), tex_path.end(), '\\', '/');
    String rel_path = String::utf8(tex_path.c_str());

    godot::Ref<Image> img;
    img.instantiate();
    Error err = ERR_FILE_NOT_FOUND;

    // --- Try VFS first (resolves both loose files and FPK archives) ---
    // Uses VFS.load_image_texture() which goes through ResourceLoader -- required for Civ IV DDS.
    Object* vfs = Engine::get_singleton()->get_singleton("VFS");
    if (vfs != nullptr) {
        Variant tex_variant = vfs->call("load_image_texture", rel_path);
        if (tex_variant.get_type() == Variant::OBJECT) {
            Object* obj = tex_variant;
            if (obj != nullptr) {
                Ref<ImageTexture> tex(Object::cast_to<ImageTexture>(obj));
                if (tex.is_valid()) {
                    texture_cache[nif_tex_path] = tex;
                    UtilityFunctions::print("[TEX] VFS loaded: ", rel_path);
                    return tex;
                }
            }
        }
    }

    // --- Fallback: direct disk search (if VFS unavailable or file not found) ---
    if (err != OK) {
        String rel_filename = rel_path.get_file();
        std::vector<String> search_paths;
        if (!current_nif_dir.is_empty()) {
            search_paths.push_back(current_nif_dir.path_join(rel_filename));
        }
        search_paths.push_back(base_path.path_join("Assets").path_join(rel_path));
        search_paths.push_back(base_path.path_join(rel_path));

        for (const auto& path : search_paths) {
            if (FileAccess::file_exists(path)) {
                err = img->load(path);
                if (err == OK) break;
            }
        }
    }

    if (err != OK) {
        texture_cache[nif_tex_path] = godot::Ref<ImageTexture>();
        return godot::Ref<ImageTexture>();
    }

    auto tex = ImageTexture::create_from_image(img);
    texture_cache[nif_tex_path] = tex;
    return tex;
}

godot::Ref<Material> GdextNiflib::create_material_from_properties(
    const std::vector<Niflib::Ref<NiProperty>>& properties,
    bool has_vertex_colors,
    const String& base_path,
    const std::string& shape_name)
{
    godot::Ref<StandardMaterial3D> mat;
    mat.instantiate();

    NiMaterialPropertyRef mat_prop;
    NiAlphaPropertyRef alpha_prop;
    NiTexturingPropertyRef tex_prop;
    NiSpecularPropertyRef spec_prop;
    NiVertexColorPropertyRef vc_prop;
    NiStencilPropertyRef stencil_prop;
    // Captured for ShaderMaterial passthrough (populated in the blocks below)
    float alpha_scissor_threshold_val = 0.0f;
    godot::Ref<godot::ImageTexture> dark_map_tex;   // populated from TeamColor.bmp (BASE_MAP slot 0) when detected
    godot::Ref<godot::ImageTexture> albedo_tex_ref;  // stored for direct ShaderMaterial transfer (avoids get_texture)
    godot::Ref<godot::ImageTexture> normal_tex_ref;  // stored for direct ShaderMaterial transfer

    for (const auto& prop : properties) {
        if (prop == NULL) continue;
        if (auto m = DynamicCast<NiMaterialProperty>(prop))     mat_prop = m;
        if (auto a = DynamicCast<NiAlphaProperty>(prop))        alpha_prop = a;
        if (auto t = DynamicCast<NiTexturingProperty>(prop))    tex_prop = t;
        if (auto s = DynamicCast<NiSpecularProperty>(prop))     spec_prop = s;
        if (auto v = DynamicCast<NiVertexColorProperty>(prop))  vc_prop = v;
        if (auto st = DynamicCast<NiStencilProperty>(prop)) stencil_prop = st;
        // Stub-acknowledged properties (log and skip)
        if (auto zb = DynamicCast<NiZBufferProperty>(prop)) {
            UtilityFunctions::print("[STUB] NiZBufferProperty: depth test/write control not yet implemented");
        }
        if (auto di = DynamicCast<NiDitherProperty>(prop)) {
            UtilityFunctions::print("[STUB] NiDitherProperty: irrelevant for modern GPUs, skipped");
        }
        if (auto sh = DynamicCast<NiShadeProperty>(prop)) {
            UtilityFunctions::print("[STUB] NiShadeProperty: flat/smooth shading, skipped (always smooth)");
        }
        if (auto wf = DynamicCast<NiWireframeProperty>(prop)) {
            UtilityFunctions::print("[STUB] NiWireframeProperty: debug wireframe, skipped");
        }
        if (auto fg = DynamicCast<NiFogProperty>(prop)) {
            UtilityFunctions::print("[STUB] NiFogProperty: per-object fog not yet implemented");
        }
    }

    // ---- Section 1: NiMaterialProperty colors ----
    if (mat_prop) {
        Niflib::Color3 diff = mat_prop->GetDiffuseColor();
        float alpha = mat_prop->GetTransparency();  // 1.0 = opaque
        UtilityFunctions::print("[MAT] diffuse=(", diff.r, ",", diff.g, ",", diff.b,
            ") alpha=", alpha);
        mat->set_albedo(godot::Color(diff.r, diff.g, diff.b, alpha));

        // Emissive
        Niflib::Color3 emissive = mat_prop->GetEmissiveColor();
        if (emissive.r > 0.001f || emissive.g > 0.001f || emissive.b > 0.001f) {
            mat->set_feature(StandardMaterial3D::FEATURE_EMISSION, true);
            mat->set_emission(godot::Color(emissive.r, emissive.g, emissive.b));
            mat->set_emission_energy_multiplier(1.0f);
        }

        // Specular / roughness
        float gloss = mat_prop->GetGlossiness();
        float roughness = 1.0f - (gloss / 128.0f);
        if (roughness < 0.0f) roughness = 0.0f;
        if (roughness > 1.0f) roughness = 1.0f;
        mat->set_roughness(roughness);

        Niflib::Color3 spec = mat_prop->GetSpecularColor();
        float spec_intensity = (spec.r + spec.g + spec.b) / 3.0f;
        mat->set_specular(spec_intensity);
    }

    // ---- Section 1b: NiSpecularProperty ----
    // If the property exists and its enable flag (bit 0) is off, kill specular entirely.
    if (spec_prop && (spec_prop->GetFlags() & 1) == 0) {
        mat->set_specular(0.0f);
        mat->set_metallic(0.0f);
    }

    // ---- Section 1c: NiStencilProperty (face draw mode / double-sided) ----
    // After winding swap (CW→CCW), NIF DRAW_CCW = Godot standard front-face.
    if (stencil_prop) {
        FaceDrawMode fdm = stencil_prop->GetFaceDrawMode();
        if (fdm == DRAW_BOTH || fdm == DRAW_CCW_OR_BOTH) {
            mat->set_cull_mode(StandardMaterial3D::CULL_DISABLED);
            UtilityFunctions::print("[MAT] NiStencilProperty: double-sided (CULL_DISABLED)");
        } else if (fdm == DRAW_CW) {
            mat->set_cull_mode(StandardMaterial3D::CULL_FRONT);
            UtilityFunctions::print("[MAT] NiStencilProperty: CW only (CULL_FRONT)");
        }
        // DRAW_CCW (1) = default CULL_BACK, no change needed
    }

    // ---- Section 2: Vertex colors ----
    // NiVertexColorProperty.vertexMode: 0=ignore, 1=emissive (approx as amb+dif), 2=amb+dif
    if (has_vertex_colors) {
        VertMode vmode = vc_prop ? vc_prop->GetVertexMode() : VERT_MODE_SRC_AMB_DIF;
        if (vmode != VERT_MODE_SRC_IGNORE) {
            mat->set_flag(StandardMaterial3D::FLAG_SRGB_VERTEX_COLOR, true);
            if (vmode == VERT_MODE_SRC_EMISSIVE)
                UtilityFunctions::print("[MAT] NiVertexColorProperty: EMISSIVE mode "
                                        "approximated as ambient+diffuse");
        }
    }

    // ---- Section 3: NiTexturingProperty textures ----
    if (tex_prop != NULL) {
        // Log only slots that have textures (not all 12)
        const char* slot_names[] = {"BASE_MAP","DARK_MAP","DETAIL_MAP","GLOSS_MAP",
            "GLOW_MAP","BUMP_MAP","NORMAL_MAP","UNKNOWN2","DECAL_0","DECAL_1","DECAL_2","DECAL_3"};
        for (int s = 0; s < 12; ++s) {
            if (!tex_prop->HasTexture(s)) continue;
            TexDesc td = tex_prop->GetTexture(s);
            String fname = (td.source != NULL && td.source->IsTextureExternal())
                ? String::utf8(td.source->GetTextureFileName().c_str()) : String("(internal)");
            UtilityFunctions::print("[TEX] slot ", s, " (", slot_names[s], "): ", fname);
        }

        // Albedo texture priority: DECAL_0 (8) if BASE_MAP is TeamColor, else BASE_MAP (0) > DETAIL_MAP (2) > DECAL_0 (8)
        // Civ IV character models use TeamColor.bmp in BASE_MAP (team color mask) + actual texture in DECAL_0.
        // When TeamColor is detected in BASE_MAP and DECAL_0 exists, skip BASE_MAP.
        bool skip_base_map = false;
        if (tex_prop->HasTexture(0) && tex_prop->HasTexture(8)) {
            TexDesc base_desc = tex_prop->GetTexture(0);
            if (base_desc.source != NULL && base_desc.source->IsTextureExternal()) {
                std::string base_name = base_desc.source->GetTextureFileName();
                std::string base_lower = base_name;
                std::transform(base_lower.begin(), base_lower.end(), base_lower.begin(), ::tolower);
                if (base_lower.find("teamcolor") != std::string::npos) {
                    skip_base_map = true;
                    dark_map_tex = load_dds_texture(base_path, base_name);  // TeamColor.bmp is the mask
                    UtilityFunctions::print("[TEX] TeamColor mask loaded: ",
                        String::utf8(base_name.c_str()));
                }
            }
        }
        int albedo_candidates[] = {0, 2, 8};
        bool albedo_loaded = false;
        for (int slot : albedo_candidates) {
            if (albedo_loaded) break;
            if (slot == 0 && skip_base_map) continue;
            if (!tex_prop->HasTexture(slot)) continue;
            TexDesc albedo_desc = tex_prop->GetTexture(slot);
            if (albedo_desc.source == NULL || !albedo_desc.source->IsTextureExternal()) continue;
            auto tex = load_dds_texture(base_path, albedo_desc.source->GetTextureFileName());
            if (tex.is_valid()) {
                albedo_tex_ref = tex;  // keep local ref for ShaderMaterial transfer
                mat->set_texture(StandardMaterial3D::TEXTURE_ALBEDO, tex);
                // Reset albedo color to white, preserving alpha.
                // Godot multiplies albedo_color * albedo_texture. If the NIF material has
                // a grey diffuse like (0.8,0.8,0.8), the texture would appear darkened.
                // White ensures the texture displays at full intensity.
                godot::Color current = mat->get_albedo();
                mat->set_albedo(godot::Color(1.0f, 1.0f, 1.0f, current.a));
                UtilityFunctions::print("[TEX] Albedo from slot ", slot, ": ",
                    String::utf8(albedo_desc.source->GetTextureFileName().c_str()));
                // UV set: warn if non-zero (multi-UV geometry not yet supported)
                if (albedo_desc.uvSet != 0)
                    UtilityFunctions::print("[TEX] Slot ", slot, " requests UV set ",
                        albedo_desc.uvSet, " — multi-UV not yet supported, using UV set 0");
                // UV transform: apply offset and scale from TexDesc
                if (albedo_desc.hasTextureTransform) {
                    mat->set_uv1_offset(godot::Vector3(
                        albedo_desc.translation.u, albedo_desc.translation.v, 0.0f));
                    mat->set_uv1_scale(godot::Vector3(
                        albedo_desc.tiling.u, albedo_desc.tiling.v, 1.0f));
                    if (albedo_desc.wRotation != 0.0f)
                        UtilityFunctions::print("[TEX] UV rotation wRotation=",
                            albedo_desc.wRotation, " not yet supported in StandardMaterial3D");
                }
                albedo_loaded = true;
            }
        }
        // GLOSS_MAP (slot 3) -> roughness texture (inverted)
        // Civ IV GLOSS_MAP is glossiness (bright = glossy = low roughness).
        // Godot's roughness texture is the inverse (bright = rough).
        // Invert by duplicating the image and negating each pixel's RGB.
        if (tex_prop->HasTexture(3)) {
            TexDesc gloss_desc = tex_prop->GetTexture(3);
            if (gloss_desc.source != NULL && gloss_desc.source->IsTextureExternal()) {
                auto gloss_tex = load_dds_texture(base_path, gloss_desc.source->GetTextureFileName());
                if (gloss_tex.is_valid()) {
                    godot::Ref<godot::Image> rough_img = gloss_tex->get_image()->duplicate();
                    if (rough_img->is_compressed()) rough_img->decompress();
                    for (int iy = 0; iy < rough_img->get_height(); iy++)
                        for (int ix = 0; ix < rough_img->get_width(); ix++) {
                            godot::Color c = rough_img->get_pixel(ix, iy);
                            rough_img->set_pixel(ix, iy,
                                godot::Color(1.0f - c.r, 1.0f - c.g, 1.0f - c.b, c.a));
                        }
                    mat->set_texture(StandardMaterial3D::TEXTURE_ROUGHNESS,
                                     godot::ImageTexture::create_from_image(rough_img));
                    mat->set_roughness(1.0f); // texture is the sole roughness driver; scalar from NiMaterialProperty no longer needed
                    UtilityFunctions::print("[TEX] Roughness (GLOSS_MAP inverted) from slot 3: ",
                        String::utf8(gloss_desc.source->GetTextureFileName().c_str()));
                }
            }
        }
        // GLOW_MAP (slot 4) — environment / reflection map.
        // Despite the name, Civ IV uses this slot for environment reflections (e.g.
        // Environment_FX_GreyMetal.dds), NOT for self-illumination. Correct rendering
        // requires cubemap sampling with reflected view direction (as NifSkope does).
        // Skipped until a reflection shader is implemented.
        if (tex_prop->HasTexture(4)) {
            TexDesc glow_desc = tex_prop->GetTexture(4);
            std::string glow_name = (glow_desc.source != NULL && glow_desc.source->IsTextureExternal())
                ? glow_desc.source->GetTextureFileName() : "(internal)";
            UtilityFunctions::print("[TEX] GLOW_MAP (slot 4) skipped (environment map): ",
                String::utf8(glow_name.c_str()));
        }
        // BUMP_MAP (slot 5) or NORMAL_MAP (slot 6) -> normal map
        int normal_slot = tex_prop->HasTexture(6) ? 6 : (tex_prop->HasTexture(5) ? 5 : -1);
        if (normal_slot >= 0) {
            TexDesc norm_desc = tex_prop->GetTexture(normal_slot);
            if (norm_desc.source != NULL && norm_desc.source->IsTextureExternal()) {
                auto tex = load_dds_texture(base_path, norm_desc.source->GetTextureFileName());
                if (tex.is_valid()) {
                    normal_tex_ref = tex;  // keep local ref for ShaderMaterial transfer
                    mat->set_feature(StandardMaterial3D::FEATURE_NORMAL_MAPPING, true);
                    mat->set_texture(StandardMaterial3D::TEXTURE_NORMAL, tex);
                }
            }
        }
    } else {
        UtilityFunctions::print("[TEX] No NiTexturingProperty on this shape");
    }

    // ---- Section 4: NiAlphaProperty transparency ----
    if (alpha_prop != NULL) {
        bool blend = alpha_prop->GetBlendState();
        bool test = alpha_prop->GetTestState();
        unsigned short src_blend = alpha_prop->GetSourceBlendFunc();
        unsigned short dst_blend = alpha_prop->GetDestBlendFunc();
        unsigned int threshold = alpha_prop->GetTestThreshold();

        if (test && threshold > 0) {
            // Alpha test with meaningful threshold → hard cutout.
            // Works whether blend is also set or not (blend is a Civ IV default on many models).
            alpha_scissor_threshold_val = threshold / 255.0f;
            mat->set_transparency(StandardMaterial3D::TRANSPARENCY_ALPHA_SCISSOR);
            mat->set_alpha_scissor_threshold(alpha_scissor_threshold_val);
        } else if (blend && !test && !dark_map_tex.is_valid()) {
            // Pure alpha blend without test → real semi-transparency (glass, water, particles).
            // Exclude team-color meshes — those use ShaderMaterial.
            mat->set_transparency(StandardMaterial3D::TRANSPARENCY_ALPHA);
            mat->set_depth_draw_mode(StandardMaterial3D::DEPTH_DRAW_ALWAYS);
        }
        // blend=1, test=1, threshold=0 → effectively opaque. Alpha test passes all pixels,
        // and if textures have alpha=1.0, blending produces opaque result. No transparency mode.
        // blend=1, test=0, dark_map → team-color path (ShaderMaterial), handled in Section 5.
    }

    // Log final transparency decision so broken shapes can be identified by name
    {
        String tmode = "DISABLED";
        if (mat->get_transparency() == StandardMaterial3D::TRANSPARENCY_ALPHA_SCISSOR)
            tmode = String("ALPHA_SCISSOR(") + String::num(mat->get_alpha_scissor_threshold(), 3) + ")";
        else if (mat->get_transparency() == StandardMaterial3D::TRANSPARENCY_ALPHA)
            tmode = "ALPHA";
        String alpha_info = alpha_prop != NULL
            ? (String(" blend=") + (alpha_prop->GetBlendState() ? "1" : "0")
               + " test=" + (alpha_prop->GetTestState() ? "1" : "0")
               + " threshold=" + String::num((int)alpha_prop->GetTestThreshold()))
            : String(" (no NiAlphaProperty)");
        UtilityFunctions::print("[MAT] '", String::utf8(shape_name.c_str()),
            "' -> ", tmode, " albedo.a=", mat->get_albedo().a, alpha_info);
    }

    // Default cull mode (back-face culling enabled)
    mat->set_cull_mode(StandardMaterial3D::CULL_BACK);

    // ---- Section 5: Team-color shader detection ----
    // If a DARK_MAP mask (TeamColor.bmp) was loaded, return a ShaderMaterial
    // that performs the team-color blend.  Otherwise return StandardMaterial3D.
    if (dark_map_tex.is_valid()) {
        // Load shader once; ResourceLoader caches the resource.
        static godot::Ref<godot::Shader> team_shader;
        if (!team_shader.is_valid()) {
            team_shader = ResourceLoader::get_singleton()->load(
                "res://assets/shaders/unit_team_color.gdshader");
        }
        UtilityFunctions::print("[MAT] ShaderMaterial: shader_valid=", team_shader.is_valid(),
            " albedo_valid=", albedo_tex_ref.is_valid(),
            " dark_map_valid=", dark_map_tex.is_valid());
        if (team_shader.is_valid()) {
            godot::Ref<ShaderMaterial> smat;
            smat.instantiate();
            smat->set_shader(team_shader);
            // Use locally-stored refs — avoids mat->get_texture() Variant conversion issues
            if (albedo_tex_ref.is_valid())
                smat->set_shader_parameter("albedo_texture", albedo_tex_ref);
            smat->set_shader_parameter("dark_map", dark_map_tex);
            smat->set_shader_parameter("team_color", team_color);
            smat->set_shader_parameter("roughness",  (float)mat->get_roughness());
            smat->set_shader_parameter("specular_intens", (float)mat->get_specular());
            smat->set_shader_parameter("alpha_scissor_threshold", alpha_scissor_threshold_val);
            smat->set_shader_parameter("has_normal_map", normal_tex_ref.is_valid());
            if (normal_tex_ref.is_valid())
                smat->set_shader_parameter("normal_texture", normal_tex_ref);
            return smat;
        }
    }

    return mat;
}
