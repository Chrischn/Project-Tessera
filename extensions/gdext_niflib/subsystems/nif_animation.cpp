// =============================================================================
// File:              nif_animation.cpp
// Author(s):         Chrischn89
// Godot Version:     4.5
// Description:
//   KFM/KF skeletal animation loading and AnimationPlayer construction.
//   NIF animation support: KFM/KF loading, keyframe interpolation,
//   AnimationPlayer / AnimationLibrary construction.
//   Extracted from gdext_niflib.cpp — see that file's header for coordinate
//   system conventions and general architecture notes.
//
// License:
//   Released under the terms of the GNU General Public License version 3.0
// =============================================================================
#include "gdext_niflib.hpp"
#include "nif_coordinate.hpp"

#include <kfm.h>
#include <obj/NiControllerSequence.h>
#include <obj/NiTransformInterpolator.h>
#include <obj/NiKeyframeController.h>
#include <obj/NiTransformData.h>
#include <obj/NiKeyframeData.h>
#include <obj/NiBSplineCompTransformInterpolator.h>
#include <obj/NiFloatInterpolator.h>
#include <obj/NiFloatData.h>
#include <obj/NiBoolInterpolator.h>
#include <obj/NiBoolData.h>
#include <obj/NiPoint3Interpolator.h>
#include <obj/NiPosData.h>
#include <obj/NiBSplineCompFloatInterpolator.h>
#include <obj/NiBSplineCompPoint3Interpolator.h>
#include <obj/NiStringPalette.h>
#include <obj/NiBlendTransformInterpolator.h>
#include <obj/NiBlendFloatInterpolator.h>
#include <obj/NiBlendBoolInterpolator.h>
#include <obj/NiBlendPoint3Interpolator.h>
#include <gen/ControllerLink.h>

#include <godot_cpp/variant/utility_functions.hpp>
#include <godot_cpp/classes/animation.hpp>
#include <godot_cpp/classes/animation_player.hpp>
#include <godot_cpp/classes/animation_library.hpp>

#include <fstream>

using namespace godot;
using namespace Niflib;

// Linearly interpolates a NIF float-key array at the given time.
// Before the first key: returns first value. After the last key: returns last value.
// Used for XYZ Euler rotation keys where each axis has its own key array.
static float sample_float_keys(const std::vector<Niflib::Key<float>>& keys, float t) {
    if (keys.empty()) return 0.0f;
    if (keys.size() == 1 || t <= keys.front().time) return keys.front().data;
    if (t >= keys.back().time) return keys.back().data;

    // Binary search for the bracketing keys
    for (size_t i = 1; i < keys.size(); ++i) {
        if (t <= keys[i].time) {
            float t0 = keys[i - 1].time;
            float t1 = keys[i].time;
            float alpha = (t1 > t0) ? (t - t0) / (t1 - t0) : 0.0f;
            return keys[i - 1].data + alpha * (keys[i].data - keys[i - 1].data);
        }
    }
    return keys.back().data;
}

void GdextNiflib::build_animations(const godot::String& nif_path,
                                   const godot::String& base_path,
                                   godot::Node3D* root_godot_node) {
    (void)base_path; // reserved for future VFS texture lookup in .kf files
    if (!root_godot_node) return;
    // --- Derive .kfm path from .nif path ---
    std::string nif_path_std = nif_path.utf8().get_data();
    std::string kfm_path_std = nif_path_std;
    if (kfm_path_std.size() >= 4) {
        std::string ext4 = kfm_path_std.substr(kfm_path_std.size() - 4);
        for (auto& c : ext4) c = (char)tolower((unsigned char)c);
        if (ext4 == ".nif") {
            kfm_path_std = kfm_path_std.substr(0, kfm_path_std.size() - 4) + ".kfm";
        }
    }

    // Load .kfm — use the istream overload directly to avoid Kfm::Read(string)'s
    // strict EOF validation, which throws on files with trailing padding bytes.
    Niflib::Kfm kfm;
    {
        std::ifstream kfm_in(kfm_path_std, std::ios::binary);
        if (!kfm_in.good()) {
            return; // No .kfm found
        }
        try {
            unsigned int kfm_ver = kfm.Read(kfm_in);
            if (kfm_ver == 0xFFFFFFFE /*VER_INVALID*/ || kfm_ver == 0xFFFFFFFF /*VER_UNSUPPORTED*/) {
                UtilityFunctions::push_error("[ANIM] Unsupported .kfm version: ",
                    String::utf8(kfm_path_std.c_str()));
                return;
            }
        } catch (const std::exception& e) {
            UtilityFunctions::push_error("[ANIM] Failed to read .kfm: ",
                String::utf8(kfm_path_std.c_str()), " — ", e.what());
            return;
        } catch (...) {
            UtilityFunctions::push_error("[ANIM] Failed to read .kfm (unknown exception): ",
                String::utf8(kfm_path_std.c_str()));
            return;
        }
    }
    if (kfm.actions.empty()) return;

    // NIF directory — .kf filenames in the KFM are relative to this
    size_t last_sep = nif_path_std.find_last_of("/\\");
    std::string nif_dir_std = (last_sep != std::string::npos)
        ? nif_path_std.substr(0, last_sep) : ".";

    // --- Create AnimationPlayer ---
    AnimationPlayer* anim_player = memnew(AnimationPlayer);
    anim_player->set_name("AnimationPlayer");
    // Add to tree now so that root_node (..) resolves correctly at runtime
    root_godot_node->add_child(anim_player);

    godot::Ref<AnimationLibrary> lib;
    lib.instantiate();

    // Helper lambda: walk any Node3D's parent chain to build a NodePath string
    // relative to root_godot_node.  Returns "" if node is not a descendant of root_godot_node.
    auto node_rel_path = [&](Node3D* node) -> String {
        std::vector<std::string> parts;
        Node* cur = node;
        while (cur != nullptr && cur != root_godot_node) {
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
    };

    // --- Process each KfmAction ---
    for (const Niflib::KfmAction& action : kfm.actions) {
        if (action.action_filename.empty()) continue;

        std::string kf_path_std = nif_dir_std + "/" + action.action_filename;
        {
            std::ifstream kf_test(kf_path_std);
            if (!kf_test.good()) {
                continue; // Missing .kf file
            }
        }

        Niflib::NiObjectRef kf_root;
        try {
            kf_root = ReadNifTree(kf_path_std);
        } catch (const std::exception& e) {
            UtilityFunctions::push_error("[ANIM] Failed to read .kf: ",
                String::utf8(action.action_filename.c_str()), " — ", e.what());
            continue;
        }
        if (!kf_root) continue;

        Niflib::NiControllerSequenceRef seq =
            DynamicCast<Niflib::NiControllerSequence>(kf_root);
        if (!seq) continue;

        float seq_start = seq->GetStartTime();
        float seq_stop  = seq->GetStopTime();
        float duration  = seq_stop - seq_start;
        if (duration < 0.001f) duration = 0.1f;

        std::vector<Niflib::ControllerLink> ctrl_data = seq->GetControllerData();

        godot::Ref<Animation> anim;
        anim.instantiate();
        anim->set_length(duration);
        anim->set_loop_mode(seq->GetCycleType() == Niflib::CYCLE_LOOP
            ? Animation::LOOP_LINEAR
            : Animation::LOOP_NONE);

        int tracks_created = 0;
        int euler_rot_count = 0, quat_rot_count = 0, bspline_rot_count = 0;

        for (const Niflib::ControllerLink& link : ctrl_data) {
            // --- Resolve target name ---
            std::string target_name = std::string(link.nodeName);
            if (target_name.empty() && link.stringPalette != NULL) {
                target_name = link.stringPalette->GetSubStr(
                    (short)link.nodeNameOffset);
            }
            if (target_name.empty()) continue;

            // --- Resolve keyframe data + interpolator (shared by bone and node paths) ---
            Niflib::NiKeyframeDataRef kf_data = NULL;
            Niflib::NiTransformInterpolatorRef ti = NULL;
            if (link.interpolator != NULL) {
                ti = DynamicCast<Niflib::NiTransformInterpolator>(link.interpolator);
                if (ti != NULL) {
                    Niflib::NiTransformDataRef td = ti->GetData();
                    if (td != NULL) {
                        kf_data = StaticCast<Niflib::NiKeyframeData>(td);
                    }
                }
            }
            if (!kf_data && link.controller != NULL) {
                Niflib::NiKeyframeControllerRef kfc =
                    DynamicCast<Niflib::NiKeyframeController>(link.controller);
                if (kfc != NULL) {
                    kf_data = kfc->GetData();
                }
            }

            // --- Try NiBSplineCompTransformInterpolator ---
            Niflib::NiBSplineCompTransformInterpolatorRef bsi = NULL;
            if (!ti && !kf_data && link.interpolator != NULL) {
                bsi = DynamicCast<Niflib::NiBSplineCompTransformInterpolator>(link.interpolator);
            }

            // --- Try NiFloatInterpolator (alpha/transparency) ---
            Niflib::NiFloatInterpolatorRef fi = NULL;
            if (!ti && !kf_data && !bsi && link.interpolator != NULL) {
                fi = DynamicCast<Niflib::NiFloatInterpolator>(link.interpolator);
            }

            // --- Try NiBoolInterpolator (visibility) ---
            Niflib::NiBoolInterpolatorRef bi = NULL;
            if (!ti && !kf_data && !bsi && !fi && link.interpolator != NULL) {
                bi = DynamicCast<Niflib::NiBoolInterpolator>(link.interpolator);
            }

            // --- Try NiPoint3Interpolator (position) ---
            Niflib::NiPoint3InterpolatorRef p3i = NULL;
            if (!ti && !kf_data && !bsi && !fi && !bi && link.interpolator != NULL) {
                p3i = DynamicCast<Niflib::NiPoint3Interpolator>(link.interpolator);
            }

            // --- Try NiBSplineCompFloatInterpolator (compressed float/alpha) ---
            Niflib::NiBSplineCompFloatInterpolatorRef bsfi = NULL;
            if (!ti && !kf_data && !bsi && !fi && !bi && !p3i && link.interpolator != NULL) {
                bsfi = DynamicCast<Niflib::NiBSplineCompFloatInterpolator>(link.interpolator);
            }

            // --- Try NiBSplineCompPoint3Interpolator (compressed B-spline position) ---
            Niflib::NiBSplineCompPoint3InterpolatorRef bsp3i = NULL;
            if (!ti && !kf_data && !bsi && !fi && !bi && !p3i && !bsfi && link.interpolator != NULL) {
                bsp3i = DynamicCast<Niflib::NiBSplineCompPoint3Interpolator>(link.interpolator);
            }

            // --- NiBlend*Interpolator stubs (animation blending/transitions) ---
            if (!ti && !kf_data && !bsi && !fi && !bi && !p3i && !bsfi && !bsp3i && link.interpolator != NULL) {
                if (DynamicCast<NiBlendTransformInterpolator>(link.interpolator) != NULL) {
                    UtilityFunctions::push_warning("[STUB] NiBlendTransformInterpolator: animation blend not yet implemented for '",
                        String::utf8(target_name.c_str()), "'");
                    continue;
                }
                if (DynamicCast<NiBlendFloatInterpolator>(link.interpolator) != NULL) {
                    UtilityFunctions::push_warning("[STUB] NiBlendFloatInterpolator: float blend not yet implemented for '",
                        String::utf8(target_name.c_str()), "'");
                    continue;
                }
                if (DynamicCast<NiBlendBoolInterpolator>(link.interpolator) != NULL) {
                    UtilityFunctions::push_warning("[STUB] NiBlendBoolInterpolator: bool blend not yet implemented for '",
                        String::utf8(target_name.c_str()), "'");
                    continue;
                }
                if (DynamicCast<NiBlendPoint3Interpolator>(link.interpolator) != NULL) {
                    UtilityFunctions::push_warning("[STUB] NiBlendPoint3Interpolator: point3 blend not yet implemented for '",
                        String::utf8(target_name.c_str()), "'");
                    continue;
                }
            }

            // Skip truly unknown interpolator types
            if (!ti && !kf_data && !bsi && !fi && !bi && !p3i && !bsfi && !bsp3i) {
                if (link.interpolator != NULL) {
                    UtilityFunctions::push_warning("[ANIM] Skipping unsupported interpolator type '",
                        String::utf8(link.interpolator->GetType().GetTypeName().c_str()),
                        "' for target '", String::utf8(target_name.c_str()), "'");
                }
                continue;
            }

            // --- Find bone in skeleton cache ---
            Skeleton3D* skel = nullptr;
            for (auto& skel_entry : skeleton_cache) {
                int idx = skel_entry.second->find_bone(
                    String::utf8(target_name.c_str()));
                if (idx >= 0) {
                    skel = skel_entry.second;
                    break;
                }
            }

            // Determine track path: bone track (Skeleton3D:BoneName) or scene graph node track.
            String track_path;

            if (skel) {
                // --- Bone track ---
                String skel_path = node_rel_path(skel);
                if (skel_path.is_empty()) {
                    UtilityFunctions::push_warning("[ANIM] Skeleton3D not under root node, skipping bone: ",
                        String::utf8(target_name.c_str()));
                    continue;
                }
                track_path = skel_path + ":" + String::utf8(target_name.c_str());
            } else {
                // --- Scene graph node track (e.g. "MD", "MD NonAccum", "Editable Mesh") ---
                Node3D* target_node = nullptr;
                String search_name = String::utf8(target_name.c_str());
                Node* found = root_godot_node->find_child(search_name, true, false);
                if (found) target_node = Object::cast_to<Node3D>(found);

                if (!target_node) continue;

                track_path = node_rel_path(target_node);
                if (track_path.is_empty()) continue;
            }

            // --- Float property track (alpha/transparency) ---
            if (fi) {
                if (skel) continue;  // float properties target scene nodes, not bones
                Niflib::NiFloatDataRef fd = fi->GetData();
                if (fd) {
                    auto float_keys = fd->GetKeys();
                    if (!float_keys.empty()) {
                        int track = anim->add_track(Animation::TYPE_VALUE);
                        anim->track_set_path(track, NodePath(track_path + ":transparency"));
                        for (const auto& k : float_keys) {
                            anim->track_insert_key(track, k.time - seq_start, 1.0f - k.data);
                        }
                        tracks_created++;
                    }
                } else {
                    float alpha = fi->GetFloatValue();
                    if (alpha < 1.0f) {
                        int track = anim->add_track(Animation::TYPE_VALUE);
                        anim->track_set_path(track, NodePath(track_path + ":transparency"));
                        anim->track_insert_key(track, 0.0, 1.0f - alpha);
                        tracks_created++;
                    }
                }
                continue;
            }

            // --- Bool property track (visibility) ---
            if (bi) {
                if (skel) continue;  // visibility targets scene nodes, not bones
                Niflib::NiBoolDataRef bd = bi->GetData();
                if (bd) {
                    auto bool_keys = bd->GetKeys();
                    if (!bool_keys.empty()) {
                        int track = anim->add_track(Animation::TYPE_VALUE);
                        anim->track_set_path(track, NodePath(track_path + ":visible"));
                        for (const auto& k : bool_keys) {
                            anim->track_insert_key(track, k.time - seq_start, (bool)(k.data != 0));
                        }
                        tracks_created++;
                    }
                } else {
                    bool vis = bi->GetBoolValue();
                    int track = anim->add_track(Animation::TYPE_VALUE);
                    anim->track_set_path(track, NodePath(track_path + ":visible"));
                    anim->track_insert_key(track, 0.0, vis);
                    tracks_created++;
                }
                continue;
            }

            // --- Point3 property track (position on scene nodes) ---
            if (p3i) {
                if (skel) continue;  // position properties target scene nodes, not bones
                Niflib::NiPosDataRef pd = p3i->GetData();
                if (pd) {
                    auto pos_keys = pd->GetKeys();
                    if (!pos_keys.empty()) {
                        int track = anim->add_track(Animation::TYPE_POSITION_3D);
                        anim->track_set_path(track, NodePath(track_path));
                        for (const auto& k : pos_keys) {
                            anim->position_track_insert_key(track, k.time - seq_start,
                                nif_to_godot_vec3(k.data));
                        }
                        tracks_created++;
                    }
                } else {
                    Niflib::Vector3 val = p3i->GetPoint3Value();
                    if (val.x < 1e+18f && val.y < 1e+18f && val.z < 1e+18f) {
                        int track = anim->add_track(Animation::TYPE_POSITION_3D);
                        anim->track_set_path(track, NodePath(track_path));
                        anim->position_track_insert_key(track, 0.0, nif_to_godot_vec3(val));
                        tracks_created++;
                    }
                }
                continue;
            }

            // --- B-spline compressed float interpolator (alpha/transparency) ---
            if (bsfi) {
                if (skel) continue;  // float properties target scene nodes, not bones
                unsigned int npts = bsfi->GetNumControlPoints();
                if (npts < 2) npts = 2;
                int degree = 3;  // cubic B-spline
                auto float_keys = bsfi->SampleKeys(npts, degree);
                if (!float_keys.empty()) {
                    int track = anim->add_track(Animation::TYPE_VALUE);
                    anim->track_set_path(track, NodePath(track_path + ":transparency"));
                    for (const auto& k : float_keys) {
                        anim->track_insert_key(track, k.time - seq_start, 1.0f - k.data);
                    }
                    tracks_created++;
                }
                continue;
            }

            // --- B-spline compressed Point3 interpolator (position on scene nodes) ---
            if (bsp3i) {
                if (skel) continue;  // position properties target scene nodes, not bones
                unsigned int npts = bsp3i->GetNumControlPoints();
                if (npts < 2) npts = 2;
                int degree = 3;  // cubic B-spline
                auto pos_keys = bsp3i->SampleKeys(npts, degree);
                if (!pos_keys.empty()) {
                    int track = anim->add_track(Animation::TYPE_POSITION_3D);
                    anim->track_set_path(track, NodePath(track_path));
                    for (const auto& k : pos_keys) {
                        anim->position_track_insert_key(track, k.time - seq_start,
                            nif_to_godot_vec3(k.data));
                    }
                    tracks_created++;
                }
                continue;
            }

            // --- Inline interpolator fallback ---
            if (!kf_data && ti != NULL) {
                Niflib::Vector3 it = ti->GetTranslation();
                Niflib::Quaternion ir = ti->GetRotation();
                bool has_trans = (it.x < 1e+18f && it.y < 1e+18f && it.z < 1e+18f);
                bool has_rot = !(ir.w == 0.0f && ir.x == 0.0f && ir.y == 0.0f && ir.z == 0.0f)
                            && (std::abs(ir.w) < 1e+18f && std::abs(ir.x) < 1e+18f
                             && std::abs(ir.y) < 1e+18f && std::abs(ir.z) < 1e+18f);

                if (has_trans) {
                    int track = anim->add_track(Animation::TYPE_POSITION_3D);
                    anim->track_set_path(track, NodePath(track_path));
                    godot::Vector3 pos = nif_to_godot_vec3(it);
                    anim->position_track_insert_key(track, 0.0, pos);
                }
                if (has_rot) {
                    int track = anim->add_track(Animation::TYPE_ROTATION_3D);
                    anim->track_set_path(track, NodePath(track_path));
                    godot::Quaternion rot = nif_quat_to_godot(ir);
                    anim->rotation_track_insert_key(track, 0.0, rot);
                }
                if (has_trans || has_rot) tracks_created++;
                continue;
            }

            // --- B-spline compressed transform interpolator ---
            if (bsi) {
                unsigned int npts = bsi->GetNumControlPoints();
                if (npts < 2) npts = 2;
                int degree = 3;  // cubic B-spline

                // Translation
                auto bsp_trans = bsi->SampleTranslateKeys(npts, degree);
                if (!bsp_trans.empty()) {
                    int track = anim->add_track(Animation::TYPE_POSITION_3D);
                    anim->track_set_path(track, NodePath(track_path));
                    for (const auto& k : bsp_trans) {
                        godot::Vector3 pos = nif_to_godot_vec3(k.data);
                        anim->position_track_insert_key(track, k.time - seq_start, pos);
                    }
                }

                // Rotation
                auto bsp_rot = bsi->SampleQuatRotateKeys(npts, degree);
                if (!bsp_rot.empty()) {
                    int track = anim->add_track(Animation::TYPE_ROTATION_3D);
                    anim->track_set_path(track, NodePath(track_path));
                    for (const auto& k : bsp_rot) {
                        godot::Quaternion rot = nif_quat_to_godot(k.data);
                        anim->rotation_track_insert_key(track, k.time - seq_start, rot);
                    }
                    bspline_rot_count++;
                }

                tracks_created++;
                continue;
            }

            if (!kf_data) continue;

            // --- Translation track (keyframe data) ---
            auto trans_keys = kf_data->GetTranslateKeys();
            if (!trans_keys.empty()) {
                int track = anim->add_track(Animation::TYPE_POSITION_3D);
                anim->track_set_path(track, NodePath(track_path));
                for (const Niflib::Key<Niflib::Vector3>& k : trans_keys) {
                    godot::Vector3 pos = nif_to_godot_vec3(k.data);
                    anim->position_track_insert_key(track, k.time - seq_start, pos);
                }
            }

            // --- Rotation track (keyframe data) ---
            if (kf_data->GetRotateType() == Niflib::XYZ_ROTATION_KEY) {
                // NIF XYZ_ROTATION_KEY: separate float-key arrays per axis (Euler angles).
                // Intrinsic XYZ order verified from OpenMW (components/nifosg/controller.cpp).
                auto x_keys = kf_data->GetXRotateKeys();
                auto y_keys = kf_data->GetYRotateKeys();
                auto z_keys = kf_data->GetZRotateKeys();

                // Collect all unique timestamps across the three axes
                std::set<float> euler_times;
                for (const auto& k : x_keys) euler_times.insert(k.time);
                for (const auto& k : y_keys) euler_times.insert(k.time);
                for (const auto& k : z_keys) euler_times.insert(k.time);

                if (!euler_times.empty()) {
                    int track = anim->add_track(Animation::TYPE_ROTATION_3D);
                    anim->track_set_path(track, NodePath(track_path));

                    for (float t : euler_times) {
                        float rx = sample_float_keys(x_keys, t);
                        float ry = sample_float_keys(y_keys, t);
                        float rz = sample_float_keys(z_keys, t);

                        godot::Quaternion rot = nif_euler_xyz_to_godot(rx, ry, rz);
                        anim->rotation_track_insert_key(track, t - seq_start, rot);
                    }
                    euler_rot_count++;
                }
            } else {
                auto quat_keys = kf_data->GetQuatRotateKeys();
                if (!quat_keys.empty()) {
                    int track = anim->add_track(Animation::TYPE_ROTATION_3D);
                    anim->track_set_path(track, NodePath(track_path));
                    for (const Niflib::Key<Niflib::Quaternion>& k : quat_keys) {
                        godot::Quaternion rot = nif_quat_to_godot(k.data);
                        anim->rotation_track_insert_key(
                            track, k.time - seq_start, rot);
                    }
                    quat_rot_count++;
                }
            }

            // --- Scale track (keyframe data) ---
            auto scale_keys = kf_data->GetScaleKeys();
            if (!scale_keys.empty()) {
                int track = anim->add_track(Animation::TYPE_SCALE_3D);
                anim->track_set_path(track, NodePath(track_path));
                for (const Niflib::Key<float>& k : scale_keys) {
                    float s = k.data;
                    anim->scale_track_insert_key(track, k.time - seq_start,
                        godot::Vector3(s, s, s));
                }
            }

            tracks_created++;
        }

        UtilityFunctions::print("[ANIM] clip='",
            String::utf8(action.action_name.c_str()),
            "' tracks=", tracks_created,
            " rot_euler=", euler_rot_count,
            " rot_quat=", quat_rot_count,
            " rot_bspline=", bspline_rot_count);

        // Use action_name as clip name; fall back to filename stem
        String anim_name = String::utf8(action.action_name.c_str());
        if (anim_name.is_empty()) {
            anim_name = String::utf8(action.action_filename.c_str()).get_basename().get_file();
        }
        lib->add_animation(anim_name, anim);
    }

    anim_player->add_animation_library("", lib);
}
