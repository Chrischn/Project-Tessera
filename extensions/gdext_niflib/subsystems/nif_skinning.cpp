// =============================================================================
// File:              nif_skinning.cpp
// Author(s):         Chrischn89
// Godot Version:     4.5
// Description:
//   Skeletal skinning: NiSkinInstance -> Godot Skeleton3D + Skin resource
//
// License:
//   Released under the terms of the GNU General Public License version 3.0
// =============================================================================
#include "gdext_niflib.hpp"
#include "nif_coordinate.hpp"

#include <obj/NiNode.h>
#include <obj/NiSkinInstance.h>
#include <obj/NiSkinData.h>
#include <obj/NiSkinPartition.h>
#include <gen/SkinWeight.h>

#include <godot_cpp/variant/utility_functions.hpp>
#include <godot_cpp/classes/mesh_instance3d.hpp>
#include <godot_cpp/classes/sphere_mesh.hpp>
#include <godot_cpp/classes/immediate_mesh.hpp>
#include <godot_cpp/classes/label3d.hpp>
#include <godot_cpp/classes/standard_material3d.hpp>

using namespace godot;
using namespace Niflib;

// =============================================================================
// Skeletal skinning setup
// =============================================================================
// build_skeleton() constructs a Godot Skeleton3D from a NIF NiSkinInstance and
// prepares a Skin resource with the correct inverse-rest bind poses.
//
// CRITICAL DESIGN NOTES (hard-won):
//
// 1. Coordinate frame for bone rests:
//    Bone world transforms from niflib are in NIF world space.  We express them
//    relative to the parent NiNode (parent_world_godot.inverse * bone_world) so
//    that the Skeleton3D -- which sits at identity under the parent NiNode's Godot
//    node -- matches what the vertices expect.
//
// 2. Explicit Skin resource (create_skin_from_rest_transforms):
//    Without a Skin, Godot uses identity bind poses.  The GPU skinning formula is:
//      bone_matrix[i] = bone_global_pose[i] * bind_pose[i]
//    With identity bind_pose: bone_matrix = pose (not identity at rest) -> distortion.
//    create_skin_from_rest_transforms() sets bind_pose[i] = bone_global_rest[i].inverse()
//    so at rest: pose * inverse_rest = identity -> no deformation.
//
// 3. reset_bone_poses() is mandatory:
//    In Godot 4, Skeleton3D bone poses default to identity, NOT to the rest pose.
//    If poses stay at identity: bone_matrix = identity * inverse_rest = inverse_rest
//    -> every vertex gets the inverse-rest transform applied -> crumpled mesh.
//    reset_bone_poses() sets pose[i] = rest[i] so the formula cancels correctly.
//

// Grows an existing skeleton with bones from a new NiSkinInstance.
// Called when a cached skeleton is reused by a mesh that references bones
// not in the original skeleton (e.g. cavalry: rein builds skeleton, then
// CavalryHorse/CavalryRider add their own bones).
void GdextNiflib::grow_skeleton(Skeleton3D* skeleton, NiSkinInstanceRef skin, NiNodeRef parent_ni_node) {
    if (!skeleton || !skin) return;

    std::vector<NiNodeRef> new_bones = skin->GetBones();
    NiNodeRef skel_root = skin->GetSkeletonRoot();

    // Collect existing skeleton bones by name for quick lookup.
    std::set<std::string> existing_names;
    for (int i = 0; i < skeleton->get_bone_count(); ++i) {
        existing_names.insert(skeleton->get_bone_name(i).utf8().get_data());
    }

    // Find skin bones not in skeleton, plus intermediate ancestors.
    std::set<NiNodeRef> to_add;
    for (const auto& bone : new_bones) {
        if (bone == NULL) continue;
        if (existing_names.count(bone->GetName())) continue;
        to_add.insert(bone);

        // Walk up to find intermediate nodes between this bone and an existing bone.
        NiNodeRef walk = bone->GetParent();
        while (walk != NULL && walk != skel_root && walk != parent_ni_node) {
            if (existing_names.count(walk->GetName())) break;  // reached existing bone
            to_add.insert(walk);
            walk = walk->GetParent();
        }
    }

    if (to_add.empty()) return;

    // Sort by depth (parents before children) so parents are added first.
    std::vector<NiNodeRef> sorted_add(to_add.begin(), to_add.end());
    std::sort(sorted_add.begin(), sorted_add.end(),
        [&](const NiNodeRef& a, const NiNodeRef& b) {
            int da = 0, db = 0;
            for (NiNodeRef w = a->GetParent(); w != NULL && w != skel_root; w = w->GetParent()) da++;
            for (NiNodeRef w = b->GetParent(); w != NULL && w != skel_root; w = w->GetParent()) db++;
            return da < db;
        });

    // Add new bones to skeleton.
    for (const auto& bone : sorted_add) {
        std::string bone_name = bone->GetName();
        if (bone_name.empty()) bone_name = "bone_" + std::to_string(skeleton->get_bone_count());

        int godot_idx = skeleton->add_bone(String::utf8(bone_name.c_str()));
        bone_index_map[bone] = godot_idx;

        // Set parent: walk up NIF hierarchy to find a bone already in the skeleton.
        NiNodeRef parent = bone->GetParent();
        while (parent != NULL) {
            auto it = bone_index_map.find(parent);
            if (it != bone_index_map.end()) {
                skeleton->set_bone_parent(godot_idx, it->second);
                break;
            }
            // Also check by name (bone might have been added by original build_skeleton).
            int pidx = skeleton->find_bone(String::utf8(parent->GetName().c_str()));
            if (pidx >= 0) {
                skeleton->set_bone_parent(godot_idx, pidx);
                bone_index_map[parent] = pidx;  // cache for next lookups
                break;
            }
            parent = parent->GetParent();
        }

        // Set rest transform from NiNode local.
        godot::Transform3D rest = nif_matrix44_to_godot(bone->GetLocalTransform());
        skeleton->set_bone_rest(godot_idx, rest);
    }

    // Re-initialize poses after adding new bones.
    skeleton->reset_bone_poses();

    UtilityFunctions::print("[SKIN] Grew skeleton by ", (int)sorted_add.size(),
        " bones, now ", skeleton->get_bone_count(), " total");
}

// Computes bone global rest by walking parent chain of local rests.
// Safe alternative to get_bone_global_rest() which has cache corruption issues.
godot::Transform3D GdextNiflib::compute_bone_global_rest(Skeleton3D* skeleton, int bone_idx) {
    godot::Transform3D global = skeleton->get_bone_rest(bone_idx);
    int parent = skeleton->get_bone_parent(bone_idx);
    while (parent >= 0) {
        global = skeleton->get_bone_rest(parent) * global;
        parent = skeleton->get_bone_parent(parent);
    }
    return global;
}

// Checks if 'node' is a descendant of 'potential_ancestor' in the NIF hierarchy.
bool GdextNiflib::is_nif_descendant(Niflib::NiNodeRef node, Niflib::NiNodeRef potential_ancestor) {
    Niflib::NiNodeRef walk = node;
    while (walk != NULL) {
        if (walk == potential_ancestor) return true;
        walk = walk->GetParent();
    }
    return false;
}

// Finds the lowest common ancestor of two NiNodes by collecting ancestors of 'a'
// then walking 'b' upward until a match is found.
Niflib::NiNodeRef GdextNiflib::find_common_ancestor(Niflib::NiNodeRef a, Niflib::NiNodeRef b) {
    std::set<Niflib::NiNode*> ancestors_a;
    for (Niflib::NiNodeRef w = a; w != NULL; w = w->GetParent())
        ancestors_a.insert(w);
    for (Niflib::NiNodeRef w = b; w != NULL; w = w->GetParent())
        if (ancestors_a.count(w)) return w;
    return NULL;
}

// Populates bone_index_map (NiNode* -> Godot bone index) for weight assignment.
Skeleton3D* GdextNiflib::build_skeleton(NiSkinInstanceRef skin, NiNodeRef parent_ni_node) {
    if (!skin) return nullptr;

    NiSkinDataRef skin_data = skin->GetSkinData();
    if (!skin_data) return nullptr;

    std::vector<NiNodeRef> bones = skin->GetBones();
    unsigned int skin_bone_count = (unsigned int)bones.size();
    if (skin_bone_count == 0) return nullptr;

    // Collect skin bones into a set for quick lookup.
    std::set<NiNodeRef> bone_set(bones.begin(), bones.end());
    bone_set.erase(NiNodeRef());  // remove null if present

    // Find intermediate NIF nodes that must be skeleton bones.
    // Two cases: (a) nodes between two skin bones (e.g. BIP Neck between
    // BIP Spine1 and clavicles), and (b) nodes between the skeleton's Godot
    // parent (parent_ni_node) and root skin bones (e.g. BIP between
    // MD NonAccum and BIP Pelvis).  Case (b) is critical: these nodes are
    // animated, and without them the skeleton can't chain transforms correctly.
    // Nodes above parent_ni_node (MD, MD NonAccum, Scene Root) stay as
    // scene graph Node3Ds — the skeleton sits under parent_ni_node.
    NiNodeRef skel_root = skin->GetSkeletonRoot();
    for (unsigned int i = 0; i < skin_bone_count; ++i) {
        if (bones[i] == NULL) continue;
        std::vector<NiNodeRef> candidates;
        NiNodeRef walk = bones[i]->GetParent();
        bool should_add = false;
        while (walk != NULL && walk != skel_root) {
            if (bone_set.count(walk)) {
                should_add = true;  // Found existing bone ancestor
                break;
            }
            if (walk == parent_ni_node) {
                should_add = true;  // Reached skeleton's Godot parent
                break;
            }
            candidates.push_back(walk);
            walk = walk->GetParent();
        }
        if (should_add) {
            for (auto& node : candidates) {
                bone_set.insert(node);
                bones.push_back(node);
            }
        }
    }
    unsigned int bone_count = (unsigned int)bones.size();

    Skeleton3D* skeleton = memnew(Skeleton3D);
    skeleton->set_name("Skeleton3D");

    // Step 1: Add all bones (skin + intermediate) and build index map
    bone_index_map.clear();
    for (unsigned int i = 0; i < bone_count; ++i) {
        NiNodeRef bone = bones[i];
        if (bone == NULL) continue;

        std::string bone_name = bone->GetName();
        if (bone_name.empty()) bone_name = "bone_" + std::to_string(i);

        int godot_idx = skeleton->add_bone(String::utf8(bone_name.c_str()));
        bone_index_map[bone] = godot_idx;
    }

    // Step 2: Set bone parent hierarchy
    for (unsigned int i = 0; i < bone_count; ++i) {
        NiNodeRef bone = bones[i];
        if (bone == NULL) continue;

        // Walk up NIF parent chain to find a parent that is also in our bone list
        NiNodeRef parent = bone->GetParent();
        while (parent != NULL) {
            auto it = bone_index_map.find(parent);
            if (it != bone_index_map.end()) {
                skeleton->set_bone_parent(bone_index_map[bone], it->second);
                break;
            }
            parent = parent->GetParent();
        }
        // If no parent found in bone list, bone stays as root (-1 parent, the default)
    }

    // Step 3: Set rest poses from NiNode LOCAL transforms.
    // This matches godotwind's approach: bone rest = NiNode's local transform
    // (relative to its NIF parent). Animation keyframes are also in NIF-parent-local
    // space, so they directly replace the rest pose with no correction needed.
    for (unsigned int i = 0; i < bone_count; ++i) {
        NiNodeRef bone = bones[i];
        if (bone == NULL) continue;

        godot::Transform3D rest = nif_matrix44_to_godot(bone->GetLocalTransform());
        skeleton->set_bone_rest(bone_index_map[bone], rest);
    }

    // Initialize bone poses from rests.
    // In Godot 4, bone poses default to identity -- NOT the rest transform.
    // Without this, skinning computes: T = identity * inverse_rest = inverse_rest != identity.
    skeleton->reset_bone_poses();

    unsigned int intermediate_count = bone_count - skin_bone_count;
    UtilityFunctions::print("[SKIN] Built skeleton: ", bone_count, " bones (",
        skin_bone_count, " skin + ", intermediate_count, " intermediate), root=",
        skel_root ? String::utf8(skel_root->GetName().c_str()) : String("<null>"));

    // Create default Skin from rest transforms. Per-mesh custom skins (for NiSkinData
    // mismatch cases) are built in process_tri_geometry() instead.
    Ref<Skin> skin_res = skeleton->create_skin_from_rest_transforms();
    skin_cache[skel_root] = skin_res;

    return skeleton;
}

// Debug visualization: adds colored spheres, bone-to-bone lines, and name labels
// for every bone in the skeleton. Organized into separate sub-nodes so GDScript
// can toggle each layer independently via the 'I' key.
// Sub-nodes: _BoneDebug/_Lines, _BoneDebug/_Spheres, _BoneDebug/_Labels
void GdextNiflib::debug_visualize_skeleton(Skeleton3D* skeleton) {
    if (!skeleton) return;

    int bone_count = skeleton->get_bone_count();
    if (bone_count == 0) return;

    // --- Materials ---
    Ref<StandardMaterial3D> sphere_mat;
    sphere_mat.instantiate();
    sphere_mat->set_shading_mode(StandardMaterial3D::SHADING_MODE_UNSHADED);
    sphere_mat->set_albedo(Color(1.0f, 0.2f, 0.2f, 0.9f));
    sphere_mat->set_flag(StandardMaterial3D::FLAG_DISABLE_DEPTH_TEST, true);
    sphere_mat->set_transparency(StandardMaterial3D::TRANSPARENCY_ALPHA);
    sphere_mat->set_render_priority(10);

    Ref<StandardMaterial3D> line_mat;
    line_mat.instantiate();
    line_mat->set_shading_mode(StandardMaterial3D::SHADING_MODE_UNSHADED);
    line_mat->set_albedo(Color(1.0f, 1.0f, 0.0f, 0.8f));
    line_mat->set_flag(StandardMaterial3D::FLAG_DISABLE_DEPTH_TEST, true);
    line_mat->set_transparency(StandardMaterial3D::TRANSPARENCY_ALPHA);
    line_mat->set_render_priority(10);

    Ref<SphereMesh> sphere_mesh;
    sphere_mesh.instantiate();
    sphere_mesh->set_radius(2.0f);
    sphere_mesh->set_height(4.0f);
    sphere_mesh->set_radial_segments(8);
    sphere_mesh->set_rings(4);

    // --- Container hierarchy ---
    Node3D* debug_root = memnew(Node3D);
    debug_root->set_name("_BoneDebug");
    skeleton->add_child(debug_root);

    Node3D* lines_group = memnew(Node3D);
    lines_group->set_name("_Lines");
    debug_root->add_child(lines_group);

    Node3D* spheres_group = memnew(Node3D);
    spheres_group->set_name("_Spheres");
    debug_root->add_child(spheres_group);

    Node3D* labels_group = memnew(Node3D);
    labels_group->set_name("_Labels");
    labels_group->set_visible(false);  // labels hidden by default (cluttered)
    debug_root->add_child(labels_group);

    // --- Collect bone positions & create markers ---
    std::vector<Vector3> bone_positions(bone_count);

    for (int i = 0; i < bone_count; ++i) {
        Transform3D global_rest = skeleton->get_bone_global_rest(i);
        Vector3 pos = global_rest.origin;
        bone_positions[i] = pos;

        // Sphere
        MeshInstance3D* marker = memnew(MeshInstance3D);
        marker->set_name(String("_bone_") + String::num_int64(i));
        marker->set_mesh(sphere_mesh);
        marker->set_surface_override_material(0, sphere_mat);
        marker->set_position(pos);
        spheres_group->add_child(marker);

        // Label
        Label3D* label = memnew(Label3D);
        String bone_name = skeleton->get_bone_name(i);
        label->set_text(String("[") + String::num_int64(i) + String("] ") + bone_name);
        label->set_font_size(32);
        label->set_pixel_size(0.05f);
        label->set_position(pos + Vector3(0.0f, 3.0f, 0.0f));
        label->set_billboard_mode(StandardMaterial3D::BILLBOARD_ENABLED);
        label->set_modulate(Color(1.0f, 1.0f, 1.0f, 1.0f));
        label->set_draw_flag(Label3D::FLAG_DISABLE_DEPTH_TEST, true);
        label->set_render_priority(11);
        labels_group->add_child(label);
    }

    // --- Lines connecting parent-child bones ---
    // Check if there are any parent-child connections before creating the surface.
    // A 1-bone skeleton has no connections, and ImmediateMesh errors on empty surfaces.
    bool has_lines = false;
    for (int i = 0; i < bone_count && !has_lines; ++i) {
        int parent_idx = skeleton->get_bone_parent(i);
        if (parent_idx >= 0 && parent_idx < bone_count) has_lines = true;
    }

    if (has_lines) {
        Ref<ImmediateMesh> line_mesh;
        line_mesh.instantiate();
        line_mesh->surface_begin(Mesh::PRIMITIVE_LINES);
        for (int i = 0; i < bone_count; ++i) {
            int parent_idx = skeleton->get_bone_parent(i);
            if (parent_idx >= 0 && parent_idx < bone_count) {
                line_mesh->surface_add_vertex(bone_positions[parent_idx]);
                line_mesh->surface_add_vertex(bone_positions[i]);
            }
        }
        line_mesh->surface_end();
        MeshInstance3D* line_instance = memnew(MeshInstance3D);
        line_instance->set_name("_bone_lines");
        line_instance->set_mesh(line_mesh);
        line_instance->set_surface_override_material(0, line_mat);
        lines_group->add_child(line_instance);
    }

    UtilityFunctions::print("[DEBUG] Bone visualization: ", bone_count, " bones");
}

void GdextNiflib::reparent_bone_attachments(NiNodeRef ni_node, Node3D* root_godot_node) {
    if (!ni_node || skeleton_cache.empty()) return;
    for (auto& [skel_root, skeleton] : skeleton_cache) {
        reparent_bone_attachments_recursive(skel_root, skeleton, root_godot_node);
    }
}

void GdextNiflib::reparent_bone_attachments_recursive(
        NiNodeRef ni_node, Skeleton3D* skeleton, Node3D* root_godot_node) {
    if (!ni_node) return;

    std::string name = ni_node->GetName();
    int bone_idx = skeleton->find_bone(String::utf8(name.c_str()));

    if (bone_idx >= 0) {
        // This NIF node IS a bone — check children for non-bone attachment nodes.
        for (const auto& child : ni_node->GetChildren()) {
            NiNodeRef child_node = DynamicCast<NiNode>(child);
            if (!child_node) continue;

            std::string child_name = child_node->GetName();
            if (skeleton->find_bone(String::utf8(child_name.c_str())) >= 0)
                continue;  // child is also a bone, skip

            // Child is NOT a bone but parent IS — this is an attachment node.
            Node* found = root_godot_node->find_child(
                String::utf8(child_name.c_str()), true, false);
            Node3D* attachment_node = found ? Object::cast_to<Node3D>(found) : nullptr;
            if (!attachment_node) continue;

            // Skip if attachment_node contains the skeleton — reparenting it would
            // create a cyclic dependency AND orphan the skeleton from the scene tree.
            if (attachment_node->is_ancestor_of(skeleton)) continue;

            // Debug: log attachment reparenting details
            godot::Transform3D att_xform = attachment_node->get_transform();
            Niflib::Vector3 nif_t = child_node->GetLocalTranslation();
            Niflib::Matrix33 nif_r = child_node->GetLocalRotation();
            UtilityFunctions::print("[ATTACH] '", String::utf8(child_name.c_str()),
                "' -> bone '", String::utf8(name.c_str()), "' (idx=", bone_idx, ")",
                " nif_local_pos=(", nif_t.x, ",", nif_t.y, ",", nif_t.z, ")",
                " godot_local_pos=(", att_xform.origin.x, ",", att_xform.origin.y, ",", att_xform.origin.z, ")");

            BoneAttachment3D* ba = memnew(BoneAttachment3D);
            ba->set_bone_name(String::utf8(name.c_str()));
            ba->set_bone_idx(bone_idx);
            skeleton->add_child(ba);

            // Reparent: the node already has its NIF local transform applied
            // (relative to the bone), which is what BoneAttachment3D expects.
            attachment_node->get_parent()->remove_child(attachment_node);
            ba->add_child(attachment_node);
        }
    }

    // Recurse into children to find deeper bone → non-bone transitions.
    for (const auto& child : ni_node->GetChildren()) {
        NiNodeRef child_node = DynamicCast<NiNode>(child);
        if (child_node) {
            reparent_bone_attachments_recursive(child_node, skeleton, root_godot_node);
        }
    }
}
