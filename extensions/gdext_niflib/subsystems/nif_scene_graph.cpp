// =============================================================================
// File:              nif_scene_graph.cpp
// Author(s):         Chrischn89
// Godot Version:     4.5
// Description:
//   Scene graph traversal: NiNode hierarchy -> Godot Node3D hierarchy.
//   Extracted from gdext_niflib.cpp. Contains:
//     - apply_nif_transform()       — NIF local transform -> Godot Node3D
//     - process_ni_node()           — recursive NiNode dispatch with full type chain
//     - process_skinned_geometry()  — unified skinned mesh handler (dedup skeleton)
//     - process_ni_billboard_node() — NiBillboardNode handler
//     - process_ni_lod_node()       — NiLODNode handler (LOD range metadata)
//     - process_ni_switch_node()    — NiSwitchNode handler (active child visibility)
//
// License:
//   Released under the terms of the GNU General Public License version 3.0
// =============================================================================

#include "gdext_niflib.hpp"
#include "nif_coordinate.hpp"

#include <obj/NiNode.h>
#include <obj/NiAVObject.h>
#include <obj/NiTriShape.h>
#include <obj/NiTriStrips.h>
#include <obj/NiTriBasedGeom.h>
#include <obj/NiBillboardNode.h>
#include <obj/NiLODNode.h>
#include <obj/NiSwitchNode.h>
#include <obj/NiSortAdjustNode.h>
#include <obj/NiParticles.h>
#include <obj/NiLines.h>
#include <obj/NiLight.h>
#include <obj/NiCamera.h>
#include <obj/NiTextureEffect.h>
#include <obj/NiSkinInstance.h>
#include <obj/NiRangeLODData.h>
#include <gen/LODRange.h>

#include <godot_cpp/variant/utility_functions.hpp>
#include <godot_cpp/variant/dictionary.hpp>
#include <godot_cpp/classes/mesh_instance3d.hpp>

using namespace godot;
using namespace Niflib;


// =============================================================================
// Transform helper
// =============================================================================

// Applies the NIF node's local transform (translation, rotation, scale) to a Godot Node3D.
void GdextNiflib::apply_nif_transform(NiAVObjectRef av_obj, Node3D* godot_node) {
    if (!av_obj || !godot_node) return;
    // Build basis from rotation, then bake uniform scale into it.
    // Previous code called set_scale() then set_basis() — set_basis() overwrites scale.
    godot::Basis basis = nif_matrix33_to_godot(av_obj->GetLocalRotation());
    apply_uniform_scale(basis, av_obj->GetLocalScale());
    godot_node->set_transform(godot::Transform3D(
        basis, nif_to_godot_vec3(av_obj->GetLocalTranslation())));
}


// =============================================================================
// Scene graph traversal
// =============================================================================
// Recursively maps the NIF scene graph to Godot nodes.
// Uses GetChildren() (scene graph), not GetRefs() (all NIF references).
//
// Scene tree built for a skinned unit:
//   NiNode ("MD NonAccum") -> Node3D  (apply_nif_transform applied)
//     Skeleton3D (identity)            <- one per skeleton root, reused across shapes
//       MeshInstance3D (identity)      <- skinned; skeleton_path="..", explicit Skin
//       _BoneDebug (debug vis)
//
// Skeleton caching: shapes sharing the same skeleton root reuse one Skeleton3D.
// On cache hit, bone_index_map is rebuilt by name-lookup so each shape's bone
// subset is correctly mapped even if NiNode pointers differ across shapes.
void GdextNiflib::process_ni_node(NiNodeRef ni_node, Node3D* parent_godot, const String& base_path) {
    if (!ni_node || !parent_godot) return;

    // Create Node3D for this NiNode
    Node3D* godot_node = memnew(Node3D);
    std::string name = nif_display_name(StaticCast<NiObject>(ni_node));
    godot_node->set_name(String::utf8(name.c_str()));

    // Apply NIF transform
    apply_nif_transform(StaticCast<NiAVObject>(ni_node), godot_node);
    parent_godot->add_child(godot_node);
    ni_to_godot_node[ni_node] = godot_node;  // Track for skeleton re-parenting

    // Process extra data, controllers, and collision on this NiNode
    process_extra_data(StaticCast<NiObjectNET>(ni_node), godot_node);
    process_scene_controllers(StaticCast<NiObjectNET>(ni_node), godot_node);
    process_ni_collision(StaticCast<NiAVObject>(ni_node), godot_node);

    // Iterate scene graph children only
    std::vector<NiAVObjectRef> children = ni_node->GetChildren();
    for (const auto& child : children) {
        if (child == NULL) continue;

        // --- Most-derived NiNode subclasses first (inheritance order matters) ---
        if (NiBillboardNodeRef billboard = DynamicCast<NiBillboardNode>(child)) {
            process_ni_billboard_node(billboard, godot_node, base_path);
        }
        else if (NiLODNodeRef lod = DynamicCast<NiLODNode>(child)) {
            process_ni_lod_node(lod, godot_node, base_path);
        }
        else if (NiSwitchNodeRef sw = DynamicCast<NiSwitchNode>(child)) {
            process_ni_switch_node(sw, godot_node, base_path);
        }
        else if (DynamicCast<NiSortAdjustNode>(child) != NULL) {
            UtilityFunctions::print("[STUB] NiSortAdjustNode treated as NiNode: '",
                String::utf8(nif_display_name(StaticCast<NiObject>(child)).c_str()), "'");
            process_ni_node(DynamicCast<NiNode>(child), godot_node, base_path);
        }
        else if (NiNodeRef child_node = DynamicCast<NiNode>(child)) {
            process_ni_node(child_node, godot_node, base_path);
        }
        // --- Geometry ---
        else if (NiTriShapeRef tri_shape = DynamicCast<NiTriShape>(child)) {
            NiSkinInstanceRef skin = tri_shape->GetSkinInstance();
            if (skin != NULL) {
                process_skinned_geometry(StaticCast<NiTriBasedGeom>(tri_shape),
                    ni_node, godot_node, base_path, true);
            } else {
                Node3D* mesh_node = process_ni_tri_shape(tri_shape, base_path);
                if (mesh_node) godot_node->add_child(mesh_node);
            }
        }
        else if (NiTriStripsRef tri_strips = DynamicCast<NiTriStrips>(child)) {
            NiSkinInstanceRef skin = tri_strips->GetSkinInstance();
            if (skin != NULL) {
                process_skinned_geometry(StaticCast<NiTriBasedGeom>(tri_strips),
                    ni_node, godot_node, base_path, false);
            } else {
                Node3D* mesh_node = process_ni_tri_strips(tri_strips, base_path);
                if (mesh_node) godot_node->add_child(mesh_node);
            }
        }
        else if (NiLinesRef lines = DynamicCast<NiLines>(child)) {
            Node3D* mesh_node = process_ni_lines(lines, base_path);
            if (mesh_node) godot_node->add_child(mesh_node);
        }
        // --- Particles ---
        else if (NiParticlesRef particles = DynamicCast<NiParticles>(child)) {
            process_ni_particle_system(particles, godot_node, base_path);
        }
        // --- Lights ---
        else if (NiLightRef light = DynamicCast<NiLight>(child)) {
            process_ni_light(light, godot_node);
        }
        // --- Camera ---
        else if (DynamicCast<NiCamera>(child) != NULL) {
            UtilityFunctions::print("[STUB] NiCamera skipped: '",
                String::utf8(nif_display_name(StaticCast<NiObject>(child)).c_str()), "'");
        }
        // --- Texture Effect ---
        else if (DynamicCast<NiTextureEffect>(child) != NULL) {
            UtilityFunctions::print("[STUB] NiTextureEffect skipped: '",
                String::utf8(nif_display_name(StaticCast<NiObject>(child)).c_str()),
                "' — Godot equivalent: environment reflection probe");
        }
        else {
            UtilityFunctions::print("Skipping unhandled NIF type: ",
                String::utf8(child->GetType().GetTypeName().c_str()));
        }
    }
}


// =============================================================================
// Skinned geometry deduplication
// =============================================================================
// Unified handler for skinned NiTriShape and NiTriStrips. Builds or reuses a
// Skeleton3D from skeleton_cache, handles re-parenting to common ancestor when
// sibling branches share a skeleton, and rebuilds bone_index_map per shape.

void GdextNiflib::process_skinned_geometry(NiTriBasedGeomRef geom,
    NiNodeRef ni_node, Node3D* godot_node, const String& base_path, bool is_tri_shape) {

    NiSkinInstanceRef skin = DynamicCast<NiTriBasedGeom>(geom)->GetSkinInstance();
    NiNodeRef skel_root = skin->GetSkeletonRoot();
    std::vector<NiNodeRef> skin_bones = skin->GetBones();
    std::string shape_name = nif_display_name(StaticCast<NiObject>(geom));
    Skeleton3D* skeleton = nullptr;

    auto cache_it = skeleton_cache.find(skel_root);
    if (cache_it != skeleton_cache.end()) {
        skeleton = cache_it->second;

        // Re-parent skeleton to common ancestor if sibling branch detected
        auto host_it = skeleton_host_map.find(skel_root);
        if (host_it != skeleton_host_map.end()) {
            NiNodeRef current_host = host_it->second;
            if (ni_node != current_host && !is_nif_descendant(ni_node, current_host)) {
                NiNodeRef common = find_common_ancestor(ni_node, current_host);
                if (common != NULL) {
                    auto ca_it = ni_to_godot_node.find(common);
                    if (ca_it != ni_to_godot_node.end() && ca_it->second != skeleton->get_parent()) {
                        skeleton->get_parent()->remove_child(skeleton);
                        ca_it->second->add_child(skeleton);
                        skeleton_host_map[skel_root] = common;
                        reparented_skeletons.insert(skel_root);
                    }
                }
            }
        }

        // Rebuild bone_index_map for this shape's bone set
        bone_index_map.clear();
        bool has_missing = false;
        for (const auto& bone_node : skin_bones) {
            if (bone_node == NULL) continue;
            std::string bname = bone_node->GetName();
            int gidx = skeleton->find_bone(String::utf8(bname.c_str()));
            if (gidx >= 0) {
                bone_index_map[bone_node] = gidx;
            } else {
                has_missing = true;
            }
        }
        if (has_missing) {
            grow_skeleton(skeleton, skin, ni_node);
            Ref<Skin> skin_res = skeleton->create_skin_from_rest_transforms();
            skin_cache[skel_root] = skin_res;
        }
    } else {
        skeleton = build_skeleton(skin, ni_node);
        if (skeleton) {
            skeleton_cache[skel_root] = skeleton;
            skeleton_host_map[skel_root] = ni_node;
            godot_node->add_child(skeleton);
            if (debug_show_bones) {
                debug_visualize_skeleton(skeleton);
            }
        }
    }

    if (skeleton) {
        Node3D* mesh_node = is_tri_shape
            ? process_ni_tri_shape(DynamicCast<NiTriShape>(geom), base_path, skeleton, ni_node)
            : process_ni_tri_strips(DynamicCast<NiTriStrips>(geom), base_path, skeleton, ni_node);
        if (mesh_node) skeleton->add_child(mesh_node);
    } else {
        UtilityFunctions::push_warning("[SKIN] Skeleton build failed, treating as static: ",
            String::utf8(shape_name.c_str()));
        Node3D* mesh_node = is_tri_shape
            ? process_ni_tri_shape(DynamicCast<NiTriShape>(geom), base_path)
            : process_ni_tri_strips(DynamicCast<NiTriStrips>(geom), base_path);
        if (mesh_node) godot_node->add_child(mesh_node);
    }
}


// =============================================================================
// NiBillboardNode
// =============================================================================
// Billboard nodes face the camera. The billboard mode is stored as metadata
// on the Godot Node3D for runtime scripts to implement the facing behavior.

void GdextNiflib::process_ni_billboard_node(NiBillboardNodeRef billboard,
    Node3D* parent_godot, const String& base_path) {
    if (!billboard || !parent_godot) return;

    Node3D* godot_node = memnew(Node3D);
    std::string name = nif_display_name(StaticCast<NiObject>(billboard));
    godot_node->set_name(String::utf8(name.c_str()));
    apply_nif_transform(StaticCast<NiAVObject>(billboard), godot_node);
    parent_godot->add_child(godot_node);
    ni_to_godot_node[billboard] = godot_node;

    BillboardMode mode = billboard->GetBillboardMode();
    godot_node->set_meta("nif_billboard_mode", (int)mode);
    UtilityFunctions::print("[NODE] NiBillboardNode '", String::utf8(name.c_str()),
        "' mode=", (int)mode);

    std::vector<NiAVObjectRef> children = billboard->GetChildren();
    for (const auto& child : children) {
        if (child == NULL) continue;
        if (NiNodeRef child_node = DynamicCast<NiNode>(child)) {
            process_ni_node(child_node, godot_node, base_path);
        }
        else if (NiTriShapeRef tri = DynamicCast<NiTriShape>(child)) {
            Node3D* mesh = process_ni_tri_shape(tri, base_path);
            if (mesh) godot_node->add_child(mesh);
        }
        else if (NiTriStripsRef strip = DynamicCast<NiTriStrips>(child)) {
            Node3D* mesh = process_ni_tri_strips(strip, base_path);
            if (mesh) godot_node->add_child(mesh);
        }
    }
}


// =============================================================================
// NiLODNode
// =============================================================================
// Level-of-detail node. Each child corresponds to an LOD level; distance ranges
// are stored as metadata. Only the first child (highest detail) is visible by
// default — runtime scripts switch visibility based on camera distance.

void GdextNiflib::process_ni_lod_node(NiLODNodeRef lod_node,
    Node3D* parent_godot, const String& base_path) {
    if (!lod_node || !parent_godot) return;

    Node3D* godot_node = memnew(Node3D);
    std::string name = nif_display_name(StaticCast<NiObject>(lod_node));
    godot_node->set_name(String::utf8(name.c_str()));
    apply_nif_transform(StaticCast<NiAVObject>(lod_node), godot_node);
    parent_godot->add_child(godot_node);
    ni_to_godot_node[lod_node] = godot_node;

    std::vector<LODRange> lod_levels = lod_node->GetLODLevels();
    std::vector<NiAVObjectRef> children = lod_node->GetChildren();

    UtilityFunctions::print("[NODE] NiLODNode '", String::utf8(name.c_str()),
        "' children=", (int)children.size(), " lod_levels=", (int)lod_levels.size());

    Dictionary lod_dict;
    for (size_t i = 0; i < lod_levels.size(); ++i) {
        Dictionary range;
        range["near"] = lod_levels[i].nearExtent;
        range["far"] = lod_levels[i].farExtent;
        lod_dict[(int)i] = range;
    }
    godot_node->set_meta("nif_lod_ranges", lod_dict);

    for (size_t i = 0; i < children.size(); ++i) {
        if (children[i] == NULL) continue;

        Node3D* child_godot = nullptr;
        if (NiNodeRef child_node = DynamicCast<NiNode>(children[i])) {
            process_ni_node(child_node, godot_node, base_path);
            child_godot = Object::cast_to<Node3D>(
                godot_node->get_child(godot_node->get_child_count() - 1));
        }
        else if (NiTriShapeRef tri = DynamicCast<NiTriShape>(children[i])) {
            child_godot = process_ni_tri_shape(tri, base_path);
            if (child_godot) godot_node->add_child(child_godot);
        }
        else if (NiTriStripsRef strip = DynamicCast<NiTriStrips>(children[i])) {
            child_godot = process_ni_tri_strips(strip, base_path);
            if (child_godot) godot_node->add_child(child_godot);
        }

        if (child_godot && i > 0) {
            child_godot->set_visible(false);
        }
    }
}


// =============================================================================
// NiSwitchNode
// =============================================================================
// Switch node: only the child at active_index is visible. Other children are
// hidden. The active index is stored as metadata for runtime toggling.

void GdextNiflib::process_ni_switch_node(NiSwitchNodeRef switch_node,
    Node3D* parent_godot, const String& base_path) {
    if (!switch_node || !parent_godot) return;

    Node3D* godot_node = memnew(Node3D);
    std::string name = nif_display_name(StaticCast<NiObject>(switch_node));
    godot_node->set_name(String::utf8(name.c_str()));
    apply_nif_transform(StaticCast<NiAVObject>(switch_node), godot_node);
    parent_godot->add_child(godot_node);
    ni_to_godot_node[switch_node] = godot_node;

    int active_index = switch_node->GetActiveIndex();
    std::vector<NiAVObjectRef> children = switch_node->GetChildren();

    godot_node->set_meta("nif_switch_active_index", active_index);
    UtilityFunctions::print("[NODE] NiSwitchNode '", String::utf8(name.c_str()),
        "' active_index=", active_index, " children=", (int)children.size());

    for (size_t i = 0; i < children.size(); ++i) {
        if (children[i] == NULL) continue;

        Node3D* child_godot = nullptr;
        if (NiNodeRef child_node = DynamicCast<NiNode>(children[i])) {
            process_ni_node(child_node, godot_node, base_path);
            child_godot = Object::cast_to<Node3D>(
                godot_node->get_child(godot_node->get_child_count() - 1));
        }
        else if (NiTriShapeRef tri = DynamicCast<NiTriShape>(children[i])) {
            child_godot = process_ni_tri_shape(tri, base_path);
            if (child_godot) godot_node->add_child(child_godot);
        }
        else if (NiTriStripsRef strip = DynamicCast<NiTriStrips>(children[i])) {
            child_godot = process_ni_tri_strips(strip, base_path);
            if (child_godot) godot_node->add_child(child_godot);
        }

        if (child_godot && (int)i != active_index) {
            child_godot->set_visible(false);
        }
    }
}
