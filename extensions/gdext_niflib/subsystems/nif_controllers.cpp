// =============================================================================
// File:              nif_controllers.cpp
// Author(s):         Chrischn89
// Godot Version:     4.5
// Description:
//   Scene-graph controller animations: UV scroll, flip, alpha, color, visibility
//
// License:
//   Released under the terms of the GNU General Public License version 3.0
// =============================================================================
#include "gdext_niflib.hpp"

#include <obj/NiObjectNET.h>
#include <obj/NiTimeController.h>
#include <obj/NiControllerManager.h>
#include <obj/NiMultiTargetTransformController.h>
#include <obj/NiTransformController.h>
#include <obj/NiTextureTransformController.h>
#include <obj/NiFlipController.h>
#include <obj/NiAlphaController.h>
#include <obj/NiMaterialColorController.h>
#include <obj/NiVisController.h>
#include <obj/NiUVController.h>
#include <obj/NiGeomMorpherController.h>
#include <obj/NiPathController.h>
#include <obj/NiLookAtController.h>
#include <obj/NiBoneLODController.h>

#include <obj/NiFloatData.h>
#include <obj/NiPosData.h>
#include <obj/NiVisData.h>
#include <obj/NiFloatInterpolator.h>
#include <obj/NiBoolInterpolator.h>
#include <obj/NiBoolData.h>
#include <obj/NiSourceTexture.h>
#include <obj/NiUVData.h>
#include <gen/enums.h>

#include <godot_cpp/classes/animation.hpp>
#include <godot_cpp/classes/animation_player.hpp>
#include <godot_cpp/classes/animation_library.hpp>
#include <godot_cpp/classes/mesh_instance3d.hpp>
#include <godot_cpp/classes/standard_material3d.hpp>
#include <godot_cpp/classes/shader_material.hpp>

#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;
using namespace Niflib;

void GdextNiflib::process_scene_controllers(NiObjectNETRef obj, Node3D* godot_node) {
    if (!obj || !godot_node) return;

    auto controllers = obj->GetControllers();
    for (const auto& ctrl : controllers) {
        if (ctrl == NULL) continue;

        std::string type_name = ctrl->GetType().GetTypeName();

        // Controllers bypassed by design — .kfm/.kf reading handles transform animation
        if (DynamicCast<NiControllerManager>(ctrl) != NULL) {
            continue;
        }
        if (DynamicCast<NiMultiTargetTransformController>(ctrl) != NULL) {
            continue;
        }
        if (DynamicCast<NiTransformController>(ctrl) != NULL) {
            continue;
        }

        // --- Collect controllers for deferred animation building ---
        // These 6 types are handled by build_scene_animations() after the
        // full scene tree is built. Remaining types stay as stubs.
        if (DynamicCast<NiTextureTransformController>(ctrl) != NULL ||
            DynamicCast<NiFlipController>(ctrl) != NULL ||
            DynamicCast<NiAlphaController>(ctrl) != NULL ||
            DynamicCast<NiMaterialColorController>(ctrl) != NULL ||
            DynamicCast<NiVisController>(ctrl) != NULL ||
            DynamicCast<NiUVController>(ctrl) != NULL) {
            pending_scene_controllers.push_back({ctrl, obj, godot_node});
            UtilityFunctions::print("[CTRL] Collected ", String::utf8(type_name.c_str()),
                " on '", String::utf8(nif_display_name(StaticCast<NiObject>(obj)).c_str()), "'");
        }
        else if (DynamicCast<NiGeomMorpherController>(ctrl) != NULL) {
            UtilityFunctions::print("[STUB] NiGeomMorpherController on '",
                String::utf8(nif_display_name(StaticCast<NiObject>(obj)).c_str()), "'");
        }
        else if (DynamicCast<NiPathController>(ctrl) != NULL) {
            UtilityFunctions::print("[STUB] NiPathController on '",
                String::utf8(nif_display_name(StaticCast<NiObject>(obj)).c_str()), "'");
        }
        else if (DynamicCast<NiLookAtController>(ctrl) != NULL) {
            UtilityFunctions::print("[STUB] NiLookAtController on '",
                String::utf8(nif_display_name(StaticCast<NiObject>(obj)).c_str()), "'");
        }
        else if (DynamicCast<NiBoneLODController>(ctrl) != NULL) {
            UtilityFunctions::print("[STUB] NiBoneLODController on '",
                String::utf8(nif_display_name(StaticCast<NiObject>(obj)).c_str()), "'");
        }
        else {
            UtilityFunctions::print("[CTRL] Unknown controller type: ", String::utf8(type_name.c_str()),
                " on '", String::utf8(nif_display_name(StaticCast<NiObject>(obj)).c_str()), "'");
        }
    }
}

String GdextNiflib::get_relative_node_path(Node3D* root, Node3D* target) {
    if (!root || !target) return String();
    std::vector<std::string> parts;
    Node* cur = target;
    while (cur != nullptr && cur != root) {
        parts.push_back(std::string(String(cur->get_name()).utf8().get_data()));
        cur = cur->get_parent();
    }
    if (cur == nullptr) return String(); // not under root
    std::string result;
    for (int i = (int)parts.size() - 1; i >= 0; --i) {
        if (!result.empty()) result += "/";
        result += parts[i];
    }
    return String::utf8(result.c_str());
}

void GdextNiflib::build_scene_animations(const String& base_path, Node3D* root_godot_node) {
    if (!root_godot_node || pending_scene_controllers.empty()) return;

    UtilityFunctions::print("[CTRL] Building scene animations from ",
        (int)pending_scene_controllers.size(), " pending controllers");

    // Create dedicated AnimationPlayer for scene controllers
    AnimationPlayer* anim_player = memnew(AnimationPlayer);
    anim_player->set_name("_SceneAnimPlayer");
    root_godot_node->add_child(anim_player);

    Ref<Animation> anim;
    anim.instantiate();
    float max_duration = 0.0f;
    bool any_loop = false;

    for (auto& pc : pending_scene_controllers) {
        NiTimeControllerRef ctrl = pc.controller;
        Node3D* godot_node = pc.godot_node;
        if (!ctrl || !godot_node) continue;

        float start_time = ctrl->GetStartTime();
        float stop_time = ctrl->GetStopTime();
        float duration = stop_time - start_time;
        if (duration > max_duration) max_duration = duration;

        // Cycle type from flags bits 1-2
        CycleType cycle = (CycleType)((ctrl->GetFlags() >> 1) & 0x3);
        if (cycle == CYCLE_LOOP || cycle == CYCLE_REVERSE) any_loop = true;

        // Resolve path from root to target node
        String node_path = get_relative_node_path(root_godot_node, godot_node);
        if (node_path.is_empty()) {
            UtilityFunctions::push_warning("[CTRL] Cannot resolve path for controller on '",
                String::utf8(nif_display_name(StaticCast<NiObject>(pc.target_object)).c_str()), "'");
            continue;
        }

        // Find MeshInstance3D for material property controllers
        MeshInstance3D* mesh_instance = Object::cast_to<MeshInstance3D>(godot_node);
        if (!mesh_instance) {
            // Search immediate children
            for (int i = 0; i < godot_node->get_child_count(); ++i) {
                mesh_instance = Object::cast_to<MeshInstance3D>(godot_node->get_child(i));
                if (mesh_instance) break;
            }
        }
        if (!mesh_instance) {
            // Recursive fallback for skinned meshes under Skeleton3D
            TypedArray<Node> found = godot_node->find_children("*", "MeshInstance3D", true, false);
            if (found.size() > 0) mesh_instance = Object::cast_to<MeshInstance3D>(Object::cast_to<Node>(found[0]));
        }

        String mesh_path;
        if (mesh_instance) {
            mesh_path = get_relative_node_path(root_godot_node, mesh_instance);
        }

        // ShaderMaterial guard for material property controllers
        bool is_shader_material = false;
        if (mesh_instance) {
            Ref<Material> mat = mesh_instance->get_surface_override_material(0);
            if (mat.is_valid() && Object::cast_to<ShaderMaterial>(mat.ptr())) {
                is_shader_material = true;
            }
        }

        // --- Dispatch by controller type ---
        std::string ctrl_type = ctrl->GetType().GetTypeName();

        // --- NiTextureTransformController ---
        // Animates UV offset/scale on materials (scrolling water, animated flags).
        // Maps to StandardMaterial3D uv1_offset or uv1_scale properties.
        if (auto ttc = DynamicCast<NiTextureTransformController>(ctrl)) {
            if (!mesh_instance || mesh_path.is_empty()) {
                UtilityFunctions::push_warning("[CTRL] NiTextureTransformController: no MeshInstance3D found");
                continue;
            }
            if (is_shader_material) {
                UtilityFunctions::push_warning("[CTRL] NiTextureTransformController: ShaderMaterial not supported, skipping");
                continue;
            }
            // NormalizeKeys may crash if interpolator is NULL — guard it
            if (ttc->GetInterpolator() != NULL) ttc->NormalizeKeys();
            NiFloatDataRef float_data = ttc->GetData();
            if (!float_data) {
                // Fallback: try interpolator
                auto interp = DynamicCast<NiFloatInterpolator>(ttc->GetInterpolator());
                if (interp) float_data = interp->GetData();
            }
            if (!float_data) {
                UtilityFunctions::push_warning("[CTRL] NiTextureTransformController: no float data");
                continue;
            }

            TexTransform tt_type = ttc->GetTextureTransformType();
            String prop_suffix;
            switch (tt_type) {
                case TT_TRANSLATE_U: prop_suffix = ":uv1_offset:x"; break;
                case TT_TRANSLATE_V: prop_suffix = ":uv1_offset:y"; break;
                case TT_SCALE_U:    prop_suffix = ":uv1_scale:x"; break;
                case TT_SCALE_V:    prop_suffix = ":uv1_scale:y"; break;
                case TT_ROTATE:
                    UtilityFunctions::push_warning("[CTRL] NiTextureTransformController: UV rotation not supported (needs ShaderMaterial)");
                    continue;
                default:
                    UtilityFunctions::push_warning("[CTRL] NiTextureTransformController: unknown transform type ", (int)tt_type);
                    continue;
            }

            String track_path = mesh_path + ":surface_material_override/0" + prop_suffix;
            int track = anim->add_track(Animation::TYPE_VALUE);
            anim->track_set_path(track, NodePath(track_path));
            anim->value_track_set_update_mode(track, Animation::UPDATE_CONTINUOUS);

            auto keys = float_data->GetKeys();
            for (const auto& k : keys) {
                anim->track_insert_key(track, k.time - start_time, k.data);
            }

            UtilityFunctions::print("[CTRL] NiTextureTransformController: ", (int)keys.size(),
                " keys -> ", track_path);
        }
        // --- NiFlipController ---
        // Cycles through multiple source textures over time (animated fire, smoke).
        // Maps to StandardMaterial3D albedo_texture, swapped per frame via discrete keys.
        else if (auto fc = DynamicCast<NiFlipController>(ctrl)) {
            if (!mesh_instance || mesh_path.is_empty()) {
                UtilityFunctions::push_warning("[CTRL] NiFlipController: no MeshInstance3D found");
                continue;
            }
            if (is_shader_material) {
                UtilityFunctions::push_warning("[CTRL] NiFlipController: ShaderMaterial not supported, skipping");
                continue;
            }
            if (fc->GetInterpolator() != NULL) fc->NormalizeKeys();
            auto& sources = fc->GetSources();
            if (sources.empty()) {
                UtilityFunctions::push_warning("[CTRL] NiFlipController: no source textures (NiImage fallback not supported)");
                continue;
            }

            // Load all source textures
            std::vector<Ref<ImageTexture>> textures;
            for (const auto& src : sources) {
                if (src == NULL || !src->IsTextureExternal()) continue;
                auto tex = load_dds_texture(base_path, src->GetTextureFileName());
                textures.push_back(tex);
            }
            if (textures.empty()) {
                UtilityFunctions::push_warning("[CTRL] NiFlipController: failed to load any textures");
                continue;
            }

            String track_path = mesh_path + ":surface_material_override/0:albedo_texture";
            int track = anim->add_track(Animation::TYPE_VALUE);
            anim->track_set_path(track, NodePath(track_path));
            anim->value_track_set_update_mode(track, Animation::UPDATE_DISCRETE);

            float flip_delta = fc->GetDelta();
            if (flip_delta <= 0.0f && duration > 0.0f) {
                flip_delta = duration / (float)textures.size();
            }

            for (size_t i = 0; i < textures.size(); ++i) {
                float t = (float)i * flip_delta;
                if (textures[i].is_valid()) {
                    anim->track_insert_key(track, t, textures[i]);
                }
            }

            UtilityFunctions::print("[CTRL] NiFlipController: ", (int)textures.size(),
                " frames, delta=", flip_delta, "s -> ", track_path);
        }
        // --- NiAlphaController ---
        // Animates material transparency over time (fade-in/out effects on units).
        // Maps to StandardMaterial3D albedo_color alpha channel.
        else if (auto ac = DynamicCast<NiAlphaController>(ctrl)) {
            if (!mesh_instance || mesh_path.is_empty()) {
                UtilityFunctions::push_warning("[CTRL] NiAlphaController: no MeshInstance3D found");
                continue;
            }
            if (is_shader_material) {
                UtilityFunctions::push_warning("[CTRL] NiAlphaController: ShaderMaterial not supported, skipping");
                continue;
            }
            // NormalizeKeys may crash if interpolator is NULL — guard it
            if (ac->GetInterpolator() != NULL) ac->NormalizeKeys();
            NiFloatDataRef float_data = ac->GetData();
            if (!float_data) {
                auto interp = DynamicCast<NiFloatInterpolator>(ac->GetInterpolator());
                if (interp) float_data = interp->GetData();
            }
            if (!float_data) {
                UtilityFunctions::push_warning("[CTRL] NiAlphaController: no float data");
                continue;
            }

            // Ensure material has transparency enabled
            Ref<StandardMaterial3D> std_mat = Object::cast_to<StandardMaterial3D>(
                mesh_instance->get_surface_override_material(0).ptr());
            if (std_mat.is_valid() && std_mat->get_transparency() == StandardMaterial3D::TRANSPARENCY_DISABLED) {
                std_mat->set_transparency(StandardMaterial3D::TRANSPARENCY_ALPHA);
                std_mat->set_depth_draw_mode(StandardMaterial3D::DEPTH_DRAW_ALWAYS);
            }

            // Animate the full albedo_color as a Color value
            godot::Color base_color(1.0f, 1.0f, 1.0f, 1.0f);
            if (std_mat.is_valid()) base_color = std_mat->get_albedo();

            String track_path = mesh_path + ":surface_material_override/0:albedo_color";
            int track = anim->add_track(Animation::TYPE_VALUE);
            anim->track_set_path(track, NodePath(track_path));
            anim->value_track_set_update_mode(track, Animation::UPDATE_CONTINUOUS);

            auto keys = float_data->GetKeys();
            for (const auto& k : keys) {
                godot::Color c(base_color.r, base_color.g, base_color.b, k.data);
                anim->track_insert_key(track, k.time - start_time, c);
            }

            UtilityFunctions::print("[CTRL] NiAlphaController: ", (int)keys.size(),
                " keys -> ", track_path);
        }
        // --- NiMaterialColorController ---
        // Animates material color channels: diffuse, ambient, emissive, or specular.
        // Maps to StandardMaterial3D albedo_color, emission, or metallic_specular.
        else if (auto mcc = DynamicCast<NiMaterialColorController>(ctrl)) {
            if (!mesh_instance || mesh_path.is_empty()) {
                UtilityFunctions::push_warning("[CTRL] NiMaterialColorController: no MeshInstance3D found");
                continue;
            }
            if (is_shader_material) {
                UtilityFunctions::push_warning("[CTRL] NiMaterialColorController: ShaderMaterial not supported, skipping");
                continue;
            }
            mcc->NormalizeKeys();
            NiPosDataRef pos_data = mcc->GetData();
            if (!pos_data) {
                UtilityFunctions::push_warning("[CTRL] NiMaterialColorController: no pos data");
                continue;
            }

            TargetColor target = mcc->GetTargetColor();
            String prop_suffix;
            float existing_alpha = 1.0f;
            Ref<StandardMaterial3D> std_mat = Object::cast_to<StandardMaterial3D>(
                mesh_instance->get_surface_override_material(0).ptr());
            if (std_mat.is_valid()) existing_alpha = std_mat->get_albedo().a;

            switch (target) {
                case TC_DIFFUSE:
                case TC_AMBIENT:
                    prop_suffix = ":albedo_color";
                    break;
                case TC_SELF_ILLUM: {
                    if (std_mat.is_valid()) {
                        std_mat->set_feature(StandardMaterial3D::FEATURE_EMISSION, true);
                    }
                    prop_suffix = ":emission";
                    break;
                }
                case TC_SPECULAR:
                    prop_suffix = ":metallic_specular";
                    break;
                default:
                    UtilityFunctions::push_warning("[CTRL] NiMaterialColorController: unknown target ", (int)target);
                    continue;
            }

            String track_path = mesh_path + ":surface_material_override/0" + prop_suffix;
            int track = anim->add_track(Animation::TYPE_VALUE);
            anim->track_set_path(track, NodePath(track_path));
            anim->value_track_set_update_mode(track, Animation::UPDATE_CONTINUOUS);

            auto keys = pos_data->GetKeys();
            for (const auto& k : keys) {
                if (target == TC_SPECULAR) {
                    float spec = (k.data.x + k.data.y + k.data.z) / 3.0f;
                    anim->track_insert_key(track, k.time - start_time, spec);
                } else {
                    float a = (target == TC_DIFFUSE || target == TC_AMBIENT) ? existing_alpha : 1.0f;
                    godot::Color color(k.data.x, k.data.y, k.data.z, a);
                    anim->track_insert_key(track, k.time - start_time, color);
                }
            }

            UtilityFunctions::print("[CTRL] NiMaterialColorController: target=", (int)target,
                " ", (int)keys.size(), " keys -> ", track_path);
        }
        // --- NiUVController ---
        // Legacy UV animation with 4 key groups: U offset, V offset, U tiling, V tiling.
        // Maps to StandardMaterial3D uv1_offset and uv1_scale properties.
        else if (auto uvc = DynamicCast<NiUVController>(ctrl)) {
            if (!mesh_instance || mesh_path.is_empty()) {
                UtilityFunctions::push_warning("[CTRL] NiUVController: no MeshInstance3D found");
                continue;
            }
            if (is_shader_material) {
                UtilityFunctions::push_warning("[CTRL] NiUVController: ShaderMaterial not supported, skipping");
                continue;
            }

            NiUVDataRef uv_data = uvc->GetData();
            if (!uv_data) {
                UtilityFunctions::push_warning("[CTRL] NiUVController: no UV data");
                continue;
            }

            const auto& groups = uv_data->GetUvGroups();
            // Group indices: [0]=U offset, [1]=V offset, [2]=U tiling, [3]=V tiling
            struct UvTrackInfo {
                int group_idx;
                const char* prop_suffix;
                const char* label;
            };
            UvTrackInfo uv_tracks[] = {
                {0, ":uv1_offset:x", "U_offset"},
                {1, ":uv1_offset:y", "V_offset"},
                {2, ":uv1_scale:x",  "U_tiling"},
                {3, ":uv1_scale:y",  "V_tiling"},
            };

            int total_keys = 0;
            for (const auto& info : uv_tracks) {
                const auto& key_list = groups[info.group_idx].keys;
                if (key_list.empty()) continue;

                String track_path = mesh_path + ":surface_material_override/0" + info.prop_suffix;
                int track = anim->add_track(Animation::TYPE_VALUE);
                anim->track_set_path(track, NodePath(track_path));
                anim->value_track_set_update_mode(track, Animation::UPDATE_CONTINUOUS);

                for (const auto& k : key_list) {
                    anim->track_insert_key(track, k.time - start_time, k.data);
                }
                total_keys += (int)key_list.size();
            }

            UtilityFunctions::print("[CTRL] NiUVController: ", total_keys,
                " keys across 4 groups -> ", mesh_path);
        }
        // --- NiVisController ---
        // Toggles node visibility at keyframed times (blinking lights, phased effects).
        // Maps to Node3D visible property via discrete boolean keys.
        else if (auto vc = DynamicCast<NiVisController>(ctrl)) {
            // NiVisController targets node visibility, not material
            if (vc->GetInterpolator() != NULL) vc->NormalizeKeys();

            // Try legacy NiVisData first, then fall back to interpolator (NiBoolInterpolator -> NiBoolData).
            // Civ4 v20.0.0.4 stores visibility keys on the interpolator, not in legacy GetData().
            std::vector<Niflib::Key<unsigned char>> keys;
            NiVisDataRef vis_data = vc->GetData();
            if (vis_data) {
                keys = vis_data->GetKeys();
            }
            if (keys.empty()) {
                auto bool_interp = DynamicCast<NiBoolInterpolator>(vc->GetInterpolator());
                if (bool_interp) {
                    NiBoolDataRef bool_data = bool_interp->GetData();
                    if (bool_data) keys = bool_data->GetKeys();
                }
            }
            if (keys.empty()) {
                // Expected: visibility often controlled by KFM animation system, not inline data
                UtilityFunctions::print("[CTRL] NiVisController: no inline vis data on '",
                    String::utf8(nif_display_name(StaticCast<NiObject>(pc.target_object)).c_str()),
                    "' (likely driven by .kf animation)");
                continue;
            }

            String track_path = node_path + ":visible";
            int track = anim->add_track(Animation::TYPE_VALUE);
            anim->track_set_path(track, NodePath(track_path));
            anim->value_track_set_update_mode(track, Animation::UPDATE_DISCRETE);

            for (const auto& k : keys) {
                bool visible = (k.data != 0);
                anim->track_insert_key(track, k.time - start_time, visible);
            }

            UtilityFunctions::print("[CTRL] NiVisController: ", (int)keys.size(),
                " keys -> ", track_path);
        }
    }

    // Finalize animation
    if (anim->get_track_count() == 0) {
        // No tracks created — remove the empty player
        root_godot_node->remove_child(anim_player);
        memdelete(anim_player);
        return;
    }

    anim->set_length(max_duration > 0.0f ? max_duration : 1.0f);
    if (any_loop) {
        anim->set_loop_mode(Animation::LOOP_LINEAR);
    }

    Ref<AnimationLibrary> lib;
    lib.instantiate();
    lib->add_animation("scene_controllers", anim);
    anim_player->add_animation_library("", lib);
    anim_player->play("scene_controllers");

    UtilityFunctions::print("[CTRL] _SceneAnimPlayer created: ", anim->get_track_count(),
        " tracks, duration=", anim->get_length(), "s, loop=", any_loop ? "YES" : "NO");
}
