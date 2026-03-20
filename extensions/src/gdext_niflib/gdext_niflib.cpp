// =============================================================================
// gdext_niflib.cpp
// NIF -> Godot scene translation for Civilization IV NIF files (version 20.0.0.4).
//
// Responsibilities:
//   - Coordinate system conversion (NIF right-handed Z-up -> Godot right-handed Y-up)
//   - Scene graph traversal: NiNode hierarchy -> Godot Node3D hierarchy
//   - Geometry processing: NiTriShape / NiTriStrips -> ArrayMesh (via SurfaceTool)
//   - Material and texture loading: NiProperty set -> StandardMaterial3D or ShaderMaterial
//   - Skeletal skinning: NiSkinInstance -> Skeleton3D + Skin resource
//
// Key NIF types used: NiNode, NiTriShape, NiTriStrips, NiSkinInstance, NiSkinData,
//   NiTexturingProperty, NiMaterialProperty, NiAlphaProperty, NiVertexColorProperty.
//
// --- Coordinate system ---
//   NIF:   right-handed, X-right, Y-forward, Z-up
//   Godot: right-handed, X-right, Y-up,      Z-backward
//   Mapping: Godot(x, y, z) = NIF(x, z, -y)
//   Rotations: R_godot = P * R_nif * P^T  (see nif_matrix33_to_godot)
//   NOTE: niflib stores row-vector matrices (v*M), so the stored Matrix33 is R^T.
//         nif_matrix33_to_godot transposes before applying P*R*P^T.
//
// --- Winding order ---
//   The handedness change reverses which triangle winding is "front-facing".
//   NIF CW front-face must become Godot CCW front-face.
//   Fix: swap v2↔v3 in every triangle's index list.
//
// --- Geometry paths ---
//   1. Static mesh:   vertices transformed by node's local transform (apply_nif_transform)
//   2. Skinned mesh:  MeshInstance3D at identity under Skeleton3D;
//                     geometry's local transform baked into vertex positions
//   3. Team-color:    ShaderMaterial with team_color uniform; DXT3 alpha
//                     used as blend mask (NOT transparency)
//
// --- Shader ALPHA gotcha ---
//   Never write ALPHA in a Godot spatial shader for opaque meshes.
//   Even ALPHA = 1.0 pushes the mesh into the transparent render pipeline,
//   which has no depth prepass → incorrect face sorting → see-through artifacts.
// =============================================================================
#include "gdext_niflib.hpp"

//niflib Headers
#include <NIF_IO.h>
#include <gen/Header.h>
#include <nif_basic_types.h>
#include <obj/NiNode.h>
#include <obj/NiObjectNET.h>
#include <obj/NiAVObject.h>
#include <obj/NiProperty.h>
#include <obj/NiSourceTexture.h>
#include <obj/NiTriShape.h>
#include <obj/NiTriShapeData.h>
#include <obj/NiTriStrips.h>
#include <obj/NiTriStripsData.h>
#include <obj/NiTexturingProperty.h>
#include <obj/NiMaterialProperty.h>
#include <obj/NiAlphaProperty.h>
#include <obj/NiVertexColorProperty.h>
#include <obj/NiSpecularProperty.h>
// Animation
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
#include <gen/ControllerLink.h>

//godot-cpp Headers
#include <godot_cpp/variant/utility_functions.hpp>
#include <godot_cpp/classes/array_mesh.hpp>
#include <godot_cpp/classes/surface_tool.hpp>
#include <godot_cpp/classes/mesh_instance3d.hpp>
#include <godot_cpp/classes/standard_material3d.hpp>
#include <godot_cpp/classes/texture2d.hpp>
#include <godot_cpp/classes/sphere_mesh.hpp>
#include <godot_cpp/classes/immediate_mesh.hpp>
#include <godot_cpp/classes/label3d.hpp>
#include <godot_cpp/classes/animation.hpp>
#include <godot_cpp/classes/animation_player.hpp>
#include <godot_cpp/classes/animation_library.hpp>
#include <godot_cpp/classes/file_access.hpp>
//Standard Library
#include <vector>
#include <set>
#include <algorithm>
#include <exception>
#include <fstream>


using namespace godot;
using namespace Niflib;

// =============================================================================
// Coordinate conversion helpers
// =============================================================================
// NIF (Civ IV v20.0.0.4): right-handed, Z-up, Y-forward
// Godot:                  right-handed, Y-up, Z-backward
//
// Per-component mapping:
//   Godot(x, y, z) = NIF(x, z, -y)
//
// Applied to: vertex positions, normals, and translation vectors.
//
// Rotation matrices:
//   R_godot = P * R_nif * P^T
//   where P is the permutation matrix | 1  0  0 |
//                                      | 0  0  1 |
//                                      | 0 -1  0 |
//   This is NOT a simple component swap; the full conjugation accounts for
//   how rotations interact with the axis remapping.
//
// Winding order:
//   The axis remapping (Z-up → Y-up) reverses which triangle winding is
//   considered "front-facing".  NIF uses CW front-face; Godot uses CCW.
//   Fix: swap v2↔v3 in every triangle's index list.
//   Without this swap, all meshes render inside-out (backfaces visible,
//   frontfaces culled by cull_back).
//
// Shader ALPHA gotcha:
//   Never write ALPHA in a Godot spatial shader for opaque meshes.  Even
//   ALPHA = 1.0 causes Godot to route the mesh through the transparent
//   render pipeline, which has no depth prepass and uses painter's-algorithm
//   sorting.  This produces see-through artifacts on complex geometry.

static godot::Vector3 nif_to_godot_vec3(const Niflib::Vector3& v) {
    return godot::Vector3(v.x, v.z, -v.y);
}

// Converts a NIF quaternion to a Godot quaternion.
// Axis permutation NIF(x,y,z) → Godot(x,z,-y).
static godot::Quaternion nif_quat_to_godot(const Niflib::Quaternion& q) {
    return godot::Quaternion(q.x, q.z, -q.y, q.w).normalized();
}

// Converts a NIF Matrix33 rotation to a Godot Basis via R_godot = P * R_nif * P^T.
// niflib stores matrices in row-vector convention (v * M), so the stored matrix is
// the TRANSPOSE of the column-vector rotation.  We read r[j][i] (transposing) to get
// the column-vector rotation before applying the coordinate permutation P.
// Used by nif_matrix44_to_godot(), apply_nif_transform(), and skinned geometry paths.
static godot::Basis nif_matrix33_to_godot(const Niflib::Matrix33& r) {
    godot::Basis b;
    b.set_column(0, godot::Vector3( r[0][0],  r[0][2], -r[0][1]));  // X column
    b.set_column(1, godot::Vector3( r[2][0],  r[2][2], -r[2][1]));  // Y column (was Z)
    b.set_column(2, godot::Vector3(-r[1][0], -r[1][2],  r[1][1]));  // Z column (was -Y)
    return b;
}

// Applies uniform scale to a Basis (all three column vectors).
static void apply_uniform_scale(godot::Basis& basis, float scale) {
    if (std::abs(scale - 1.0f) > 0.0001f) {
        basis.set_column(0, basis.get_column(0) * scale);
        basis.set_column(1, basis.get_column(1) * scale);
        basis.set_column(2, basis.get_column(2) * scale);
    }
}

// Converts a NIF Matrix44 (4x4 transform) to a Godot Transform3D.
godot::Transform3D GdextNiflib::nif_matrix44_to_godot(const Niflib::Matrix44& mat) {
    Niflib::Matrix33 r;
    Niflib::Vector3 t;
    float scale;
    mat.Decompose(t, r, scale);

    godot::Vector3 origin = nif_to_godot_vec3(t);
    godot::Basis basis = nif_matrix33_to_godot(r);
    apply_uniform_scale(basis, scale);

    return godot::Transform3D(basis, origin);
}

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

// Converts NIF XYZ Euler angles (radians, intrinsic XYZ order) to a Godot quaternion.
// Verified rotation order from OpenMW (components/nifosg/controller.cpp): Qx * Qy * Qz.
// Axis mapping NIF→Godot: NIF-X→Godot-X, NIF-Y→Godot(-Z), NIF-Z→Godot-Y.
static godot::Quaternion nif_euler_xyz_to_godot(float rx, float ry, float rz) {
    godot::Quaternion qx(godot::Vector3(1, 0, 0), rx);
    godot::Quaternion qy(godot::Vector3(0, 0, -1), ry);
    godot::Quaternion qz(godot::Vector3(0, 1, 0), rz);
    return qx * qy * qz;
}

// Normalizes per-vertex bone weights for Godot GPU skinning (max 4 influences).
// Sorts by weight descending, truncates to 4, pads with zeros, re-normalizes sum to 1.0.
static void normalize_bone_weights(
    std::vector<std::vector<std::pair<int, float>>>& vertex_weights)
{
    for (auto& vw : vertex_weights) {
        std::sort(vw.begin(), vw.end(),
            [](const std::pair<int,float>& a, const std::pair<int,float>& b) {
                return a.second > b.second;
            });
        if (vw.size() > 4) vw.resize(4);
        while (vw.size() < 4) vw.push_back({0, 0.0f});
        float sum = 0.0f;
        for (auto& p : vw) sum += p.second;
        if (sum > 0.0001f) {
            for (auto& p : vw) p.second /= sum;
        }
    }
}

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
static bool is_nif_descendant(Niflib::NiNodeRef node, Niflib::NiNodeRef potential_ancestor) {
    Niflib::NiNodeRef walk = node;
    while (walk != NULL) {
        if (walk == potential_ancestor) return true;
        walk = walk->GetParent();
    }
    return false;
}

// Finds the lowest common ancestor of two NiNodes by collecting ancestors of 'a'
// then walking 'b' upward until a match is found.
static Niflib::NiNodeRef find_common_ancestor(Niflib::NiNodeRef a, Niflib::NiNodeRef b) {
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

//Binds functions for use from inside Godot Editor. Hint: not every function needs to be bound 
void GdextNiflib::_bind_methods() {
    ClassDB::bind_method(D_METHOD("ping"), &GdextNiflib::ping);
    ClassDB::bind_method(D_METHOD("get_nif_version", "file_path"), &GdextNiflib::get_nif_version_as_string);
    ClassDB::bind_method(D_METHOD("get_nif_header_info", "file_path"), &GdextNiflib::get_nif_header_info);
    ClassDB::bind_method(D_METHOD("get_nif_header", "file_path"), &GdextNiflib::get_nif_header);
    ClassDB::bind_method(D_METHOD("check_nif_version_code", "version_code"), &GdextNiflib::isValidNIFVersion);
    ClassDB::bind_method(D_METHOD("load_nif_scene", "file_path", "root", "base_path"), &GdextNiflib::load_nif_scene);
    ClassDB::bind_method(D_METHOD("set_team_color", "color"), &GdextNiflib::set_team_color);
    ClassDB::bind_method(D_METHOD("get_team_color"), &GdextNiflib::get_team_color);
    ADD_PROPERTY(PropertyInfo(Variant::COLOR, "team_color"), "set_team_color", "get_team_color");
}

// Check if GDExtension is working correctly
String GdextNiflib::ping() const {
    return "gdext_niflib OK";
}

// Receives a file path to a .nif file and reads version information
// Returns a String  with all the version number
// See also "get_nif_version_as_uint"
String GdextNiflib::get_nif_version_as_string(const String& file_path) const {
    try {
        unsigned int version = Niflib::GetNifVersion(file_path.utf8().get_data());
        return Niflib::FormatVersionString(version).c_str();
    } catch(const std::exception& e) {
        return "Error: " + String(e.what());
    }
}

// Receives a file path to a .nif file and reads version information
// Returns a unsinged int with version number
// See also "get_nif_version_as_string"
unsigned int GdextNiflib::get_nif_version_as_uint(const String& file_path) const {
    try {
        unsigned int version = Niflib::GetNifVersion(file_path.utf8().get_data());
        return version;
    }
    catch (const std::exception& e) {
        UtilityFunctions::print("Error: ", e.what());
        return 0;
    }
}

// Receives a file path to a .nif file and reads the header information
// Returns a Godot Dictionary with all the information
// See also "get_nif_header"
Dictionary GdextNiflib::get_nif_header_info(const String& file_path) const {
    Dictionary result;
    try {
        Niflib::NifInfo info = Niflib::ReadHeaderInfo(file_path.utf8().get_data());
        result["version"] = Niflib::FormatVersionString(info.version).c_str();
        result["user_version"] = (int)info.userVersion;
        result["user_version2"] = (int)info.userVersion2;
        result["endian"] = (int)info.endian;
        result["creator"] = info.creator.c_str();
        result["export_info1"] = info.exportInfo1.c_str();
        result["export_info2"] = info.exportInfo2.c_str();
        result["success"] = true;
    } catch (const std::exception& e) {
        result["success"] = false;
        result["error"] = e.what();
    }
    return result;
}


// Receives a file path to a .nif file and reads the header information
// Returns a Godot Dictionary with all the information
// See also "get_nif_header_info"
Dictionary GdextNiflib::get_nif_header(const String& file_path) const {
    Dictionary result;
    try {
        Niflib::Header header = Niflib::ReadHeader(file_path.utf8().get_data());
        
        result["version"] = Niflib::FormatVersionString(header.version).c_str();
        result["user_version"] = (int)header.userVersion;
        result["user_version2"] = (int)header.userVersion2;
        result["endian_type"] = (int)header.endianType;
        result["num_blocks"] = (int)header.numBlocks;
        result["num_block_types"] = (int)header.blockTypes.size();
        
        // Add block types array
        Array block_types;
        for (const auto& type : header.blockTypes) {
            block_types.push_back(type.c_str());
        }
        result["block_types"] = block_types;
        
        // Creator and export info
        result["creator"] = header.exportInfo.creator.str.c_str();
        result["export_info1"] = header.exportInfo.exportInfo1.str.c_str();
        result["export_info2"] = header.exportInfo.exportInfo2.str.c_str();
        
        // Copyright lines (first few)
        Array copyright;
        for (size_t i = 0; i < 3; ++i) {
            copyright.push_back(header.copyright[i].line.c_str());
        }
        result["copyright"] = copyright;
        
        result["success"] = true;
    } catch (const std::exception& e) {
        result["success"] = false;
        result["error"] = e.what();
    }
    return result;
}


// Receives a .nif file version number and checks whether it is a valid version or not
// Returns true or false
bool GdextNiflib::isValidNIFVersion(unsigned int version_code) const {
    bool result;
    unsigned int version = version_code;
    
    if (version == Niflib::VER_INVALID) {
        result = false;
        godot::print_line("VER_INVALID");
        UtilityFunctions::print("Provided version: " + String(Niflib::FormatVersionString(version).c_str()));
        godot::print_line("Not a NIF file");
        return result;
    }

    if (version == Niflib::VER_UNSUPPORTED) {
        result = false;
        godot::print_line("VER_UNSUPPORTED");
        UtilityFunctions::print("Provided version: " + String(Niflib::FormatVersionString(version).c_str()));
        godot::print_line("Unsupported NIF version");
        return result;
    }
    
    if (Niflib::IsSupportedVersion(version)) {
        result = true;
        godot::print_line("SUPPORTED_VERSION");
        UtilityFunctions::print("Supported version: " + String(Niflib::FormatVersionString(version).c_str()));
        return result;
    }
    
    result = true;
    godot::print_line("UNKNOWN_VERSION");
    UtilityFunctions::print("Unknown version: " + String(Niflib::FormatVersionString(version).c_str()));
    return result;
}

// =============================================================================
// build_animations
// =============================================================================
// Loads the .kfm file that sits alongside nif_path, then loads each referenced
// .kf clip, parses NiControllerSequence → ControllerLink → NiKeyframeData, and
// builds an AnimationPlayer with one Animation per KfmAction.
//
// Track paths are built by walking the Skeleton3D's parent chain up to
// root_godot_node, forming a path relative to that root.  This is safe for
// nodes that are not yet in a scene tree.
//
// Requires skeleton_cache to be populated — call after process_ni_node().
// Adds the AnimationPlayer as a child of root_godot_node before returning.
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
            tracks_created++;
            // Scale keys deferred.
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

// =============================================================================
// Receives a file path to a .nif file that needs importing into Godot.
// Receives a Godot Node* pointer to an already existing Godot Node to enable the retrieval of data from Godot or to attach new Nodes to it.
void GdextNiflib::load_nif_scene(const String& file_path, Node3D* godotnode, const String& base_path) {
    try {
        if (!isValidNIFVersion(get_nif_version_as_uint(file_path))) {
            UtilityFunctions::push_error("Invalid NIF version: ", file_path);
            return;
        }

        // Store NIF file's directory for texture resolution
        current_nif_dir = file_path.get_base_dir();

        NiObjectRef ref_root = ReadNifTree(file_path.utf8().get_data());
        if (!ref_root || !godotnode) {
            UtilityFunctions::push_error("Failed to read NIF tree or no parent node.");
            return;
        }

        // New architecture: traverse via scene graph (GetChildren), not GetRefs
        NiNodeRef root_node = DynamicCast<NiNode>(ref_root);
        if (root_node) {
            process_ni_node(root_node, godotnode, base_path);

            // Reparent attachment nodes (weapons, props) from static Node3D
            // tree to BoneAttachment3D on the Skeleton3D so they follow
            // animated bones.
            reparent_bone_attachments(root_node, godotnode);

            // Deferred rotation correction for non-re-parented skeletons only.
            // Re-parented skeletons already have correct orientation via common ancestor.
            for (auto& [skel_root_ni, skel] : skeleton_cache) {
                if (reparented_skeletons.count(skel_root_ni)) continue;  // skip re-parented
                auto rc_it = rotation_correction_cache.find(skel_root_ni);
                if (rc_it == rotation_correction_cache.end()) continue;
                auto con_it = rotation_correction_consistent.find(skel_root_ni);
                if (con_it == rotation_correction_consistent.end() || !con_it->second) continue;
                int skinned_mesh_count = 0;
                for (int ci = 0; ci < skel->get_child_count(); ++ci) {
                    MeshInstance3D* mi = Object::cast_to<MeshInstance3D>(skel->get_child(ci));
                    if (mi && mi->get_skin().is_valid()) skinned_mesh_count++;
                }
                auto mc_it = rotation_correction_mismatch_count.find(skel_root_ni);
                int mismatch_count = (mc_it != rotation_correction_mismatch_count.end()) ? mc_it->second : 0;
                if (mismatch_count == skinned_mesh_count) {
                    skel->set_transform(rc_it->second);
                }
            }
        } else {
            // Edge case: root is a single NiTriShape (rare)
            NiTriShapeRef root_shape = DynamicCast<NiTriShape>(ref_root);
            if (root_shape) {
                Node3D* mesh = process_ni_tri_shape(root_shape, base_path);
                if (mesh) godotnode->add_child(mesh);
            } else {
                UtilityFunctions::push_error("NIF root is neither NiNode nor NiTriShape.");
            }
        }

        // Build animations (.kfm/.kf) — must be called before caches are cleared.
        // skeleton_cache is still populated at this point.
        UtilityFunctions::print("[LOAD] process_ni_node done, calling build_animations...");
        build_animations(file_path, base_path, godotnode);
        UtilityFunctions::print("[LOAD] build_animations done, clearing caches");

        // Clear per-NIF state
        texture_cache.clear();
        current_nif_dir = "";
        bone_index_map.clear();
        skeleton_cache.clear();
        skin_cache.clear();
        skeleton_host_map.clear();
        ni_to_godot_node.clear();
        reparented_skeletons.clear();
        rotation_correction_cache.clear();
        rotation_correction_consistent.clear();
        rotation_correction_mismatch_count.clear();

    } catch (const std::exception& e) {
        UtilityFunctions::push_error("Error loading NIF: ", e.what());
    }
}

// Returns the name/value of a provided NiObject if it has one otherwise it returns the type name of the provided NiObject
std::string GdextNiflib::nif_display_name(const Niflib::NiObjectRef& obj) {
    if (!obj) return "<null>";
    if (auto net = Niflib::DynamicCast<Niflib::NiObjectNET>(obj)) {
        const std::string& n = net->GetName();
        if (!n.empty()) return n;
    }
    return obj->GetType().GetTypeName(); // Fallback
}


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
// Material creation
// =============================================================================
// Builds a Godot material from a NIF shape's property list.
// Five sections:
//   1. NiMaterialProperty  → diffuse/emissive/specular/roughness
//   2. Vertex colors        → flag for SRGB vertex color blending
//   3. NiTexturingProperty  → albedo, gloss, normal, and team-color textures
//   4. NiAlphaProperty      → transparency mode (scissor, blend, or opaque)
//   5. Team-color shader    → ShaderMaterial when TeamColor.bmp is detected
//
// Returns StandardMaterial3D for normal meshes, ShaderMaterial for team-color.
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

// =============================================================================
// process_tri_geometry — shared geometry builder for NiTriShape / NiTriStrips
// =============================================================================
// Both NiTriShape and NiTriStrips follow the exact same pipeline after data
// extraction. This function handles it all via the common NiTriBasedGeom /
// NiTriBasedGeomData base classes.  The two public wrappers below are thin
// adapters that just forward here.
// =============================================================================
Node3D* GdextNiflib::process_tri_geometry(NiTriBasedGeomRef geom, const String& base_path,
    Skeleton3D* skeleton, NiNodeRef parent_ni_node) {
    if (!geom) return nullptr;

    // --- Geometry data extraction ---
    NiTriBasedGeomDataRef data = DynamicCast<NiTriBasedGeomData>(geom->GetData());
    if (!data) {
        UtilityFunctions::print("[WARN] process_tri_geometry: no geometry data on '",
            String::utf8(nif_display_name(StaticCast<NiObject>(geom)).c_str()), "'");
        return nullptr;
    }

    std::vector<Niflib::Vector3>  vertices  = data->GetVertices();
    std::vector<Niflib::Triangle> triangles = data->GetTriangles();
    if (vertices.empty() || triangles.empty()) return nullptr;

    std::vector<Niflib::Vector3> normals = data->GetNormals();
    bool has_normals = !normals.empty() && (normals.size() == vertices.size());

    std::vector<Niflib::TexCoord> uvs;
    if (data->GetUVSetCount() > 0) uvs = data->GetUVSet(0);
    bool has_uvs = !uvs.empty() && (uvs.size() == vertices.size());

    std::vector<Niflib::Color4> colors = data->GetColors();
    bool has_colors = !colors.empty() && (colors.size() == vertices.size());

    // --- Skinning detection ---
    // NIF stores per-bone vertex weights in NiSkinInstance/NiSkinData.
    NiSkinInstanceRef skin = geom->GetSkinInstance();
    NiSkinDataRef skin_data;
    bool is_skinned = false;
    // skin_vertex_transform: geometry local transform baked into vertex positions.
    // For skinned meshes, MeshInstance3D is at identity under Skeleton3D, so the
    // geometry's own local offset must be folded into the vertex data directly.
    godot::Transform3D skin_vertex_transform;

    // Per-vertex bone weights: vertex_index -> [(godot_bone_idx, weight), ...]
    std::vector<std::vector<std::pair<int, float>>> vertex_weights;

    if (skin != NULL && skeleton != nullptr) {
        skin_data = skin->GetSkinData();
        if (skin_data != NULL) {
            is_skinned = true;
            vertex_weights.resize(vertices.size());

            // Build the geometry's local transform using shared helpers.
            {
                NiAVObjectRef av = StaticCast<NiAVObject>(geom);
                Niflib::Vector3 gt = av->GetLocalTranslation();
                Niflib::Matrix33 gr = av->GetLocalRotation();
                float gs = av->GetLocalScale();

                godot::Basis basis = nif_matrix33_to_godot(gr);
                apply_uniform_scale(basis, gs);
                skin_vertex_transform = godot::Transform3D(basis, nif_to_godot_vec3(gt));
            }

            // DIAGNOSTIC: log geom_local for all skinned meshes
            {
                godot::Quaternion svt_quat = skin_vertex_transform.basis.orthonormalized().get_rotation_quaternion();
                UtilityFunctions::print("[SKINFIX] mesh='",
                    String::utf8(nif_display_name(StaticCast<NiObject>(geom)).c_str()),
                    "' geom_local_origin=(", skin_vertex_transform.origin.x,
                    ",", skin_vertex_transform.origin.y,
                    ",", skin_vertex_transform.origin.z,
                    ") geom_local_quat=(", svt_quat.x, ",", svt_quat.y,
                    ",", svt_quat.z, ",", svt_quat.w, ")");
            }

            std::vector<NiNodeRef> bones = skin->GetBones();
            unsigned int bone_count = skin_data->GetBoneCount();

            // Invert NIF's per-bone weight lists to per-vertex weight lists
            for (unsigned int b = 0; b < bone_count; ++b) {
                NiNodeRef bone_node = (b < bones.size()) ? bones[b] : NiNodeRef();
                if (bone_node == NULL) continue;

                auto it = bone_index_map.find(bone_node);
                if (it == bone_index_map.end()) continue;
                int godot_bone = it->second;

                std::vector<SkinWeight> weights = skin_data->GetBoneWeights(b);
                for (const auto& sw : weights) {
                    if (sw.index < vertices.size()) {
                        vertex_weights[sw.index].push_back({godot_bone, sw.weight});
                    }
                }
            }

            normalize_bone_weights(vertex_weights);

            // Diagnostic: count vertices with non-zero weights
            unsigned int verts_with_weights = 0;
            for (const auto& vw : vertex_weights) {
                for (const auto& p : vw) {
                    if (p.second > 0.0001f) { ++verts_with_weights; break; }
                }
            }
            NiSkinPartitionRef skin_part = skin->GetSkinPartition();
            UtilityFunctions::print("[SKIN] shape=\"",
                String::utf8(nif_display_name(StaticCast<NiObject>(geom)).c_str()),
                "\" bones=", bone_count,
                " verts=", (int)vertices.size(),
                " verts_with_skindata_weights=", verts_with_weights,
                " skin_partition=", skin_part != NULL ? "YES" : "NO");
        }
    }

    // --- Per-mesh NiSkinData mismatch detection ---
    // Each mesh independently checks whether NiNode rest poses match NiSkinData bind poses.
    // If they don't, a per-mesh custom Skin is built with NiSkinData-derived binds so that
    // animation deformation matches the pose the geometry was authored for.
    godot::Ref<godot::Skin> per_mesh_skin;
    if (is_skinned && parent_ni_node != NULL) {
        Niflib::NiNodeRef skel_root = skin->GetSkeletonRoot();
        Niflib::NiSkinDataRef sd = skin->GetSkinData();
        Niflib::Matrix44 overall_nif = sd->GetOverallTransform();

        // Prefix: transform from parent_ni_node up to skel_root (exclusive), in NIF space.
        // Skeleton is under the first mesh's parent Node3D (godot_node), so prefix
        // covers transforms from this mesh's parent up to skel_root.
        Niflib::Matrix44 prefix_nif;
        Niflib::NiNodeRef walk_node = parent_ni_node;
        while (walk_node != NULL && walk_node != skel_root) {
            prefix_nif = prefix_nif * walk_node->GetLocalTransform();
            walk_node = walk_node->GetParent();
        }
        Niflib::Matrix44 prefix_nif_inv = prefix_nif.Inverse();

        // Compare NiNode-derived rest vs NiSkinData bind for up to 3 skin bones.
        std::vector<Niflib::NiNodeRef> skin_bones = skin->GetBones();
        unsigned int skin_bone_count = (unsigned int)skin_bones.size();
        bool needs_custom_skin = false;
        unsigned int bones_checked = 0;

        for (unsigned int i = 0; i < skin_bone_count && bones_checked < 3; ++i) {
            if (skin_bones[i] == NULL) continue;
            auto it = bone_index_map.find(skin_bones[i]);
            if (it == bone_index_map.end()) continue;

            godot::Transform3D ninode_world = compute_bone_global_rest(skeleton, it->second);

            Niflib::Matrix44 bone_nif = sd->GetBoneTransform(i);
            Niflib::Matrix44 bone_world_nif = bone_nif.Inverse() * overall_nif.Inverse() * prefix_nif_inv;
            godot::Transform3D skindata_world = nif_matrix44_to_godot(bone_world_nif);

            float dist = (ninode_world.origin - skindata_world.origin).length();

            // Also check rotation mismatch (quaternion angle difference).
            // Some meshes (e.g. bomber body) have matching positions but
            // different rotations — these need custom skin too.
            godot::Quaternion q_ninode = ninode_world.basis.orthonormalized().get_rotation_quaternion();
            godot::Quaternion q_skindata = skindata_world.basis.orthonormalized().get_rotation_quaternion();
            float rot_dot = std::abs(q_ninode.dot(q_skindata));
            float rot_angle_deg = rot_dot < 1.0f
                ? godot::Math::rad_to_deg(2.0f * std::acos(std::min(rot_dot, 1.0f)))
                : 0.0f;

            if (dist > 10.0f || rot_angle_deg > 5.0f) {
                needs_custom_skin = true;
                UtilityFunctions::print("[SKIN] Per-mesh mismatch for '",
                    String::utf8(nif_display_name(StaticCast<NiObject>(geom)).c_str()),
                    "' bone=", skeleton->get_bone_name(it->second),
                    " dist=", dist, " rot=", rot_angle_deg, "deg");
                break;
            }
            bones_checked++;
        }

        if (needs_custom_skin) {
            // Dense skin: every bone gets an entry (fixes bs > sbs rendering error).
            // Non-skin bones use default inv(bone_global_rest); skin bones use NiSkinData binds.
            int total_bones = skeleton->get_bone_count();
            per_mesh_skin.instantiate();
            per_mesh_skin->set_bind_count(total_bones);

            for (int bi = 0; bi < total_bones; ++bi) {
                per_mesh_skin->set_bind_bone(bi, bi);
                per_mesh_skin->set_bind_pose(bi,
                    compute_bone_global_rest(skeleton, bi).affine_inverse());
            }

            // Override skin bones with NiSkinData-derived binds.
            for (unsigned int i = 0; i < skin_bone_count; ++i) {
                if (skin_bones[i] == NULL) continue;
                auto it = bone_index_map.find(skin_bones[i]);
                if (it == bone_index_map.end()) continue;

                Niflib::Matrix44 bw_nif = sd->GetBoneTransform(i);
                Niflib::Matrix44 skin_bind_nif = prefix_nif * overall_nif * bw_nif;
                godot::Transform3D skin_bind = nif_matrix44_to_godot(skin_bind_nif);
                per_mesh_skin->set_bind_pose(it->second, skin_bind);
            }

            // DIAGNOSTIC: log R for first bone of each mismatch mesh
            for (unsigned int ci = 0; ci < skin_bone_count; ++ci) {
                if (skin_bones[ci] == NULL) continue;
                auto cit = bone_index_map.find(skin_bones[ci]);
                if (cit == bone_index_map.end()) continue;

                godot::Transform3D bone_rest = compute_bone_global_rest(skeleton, cit->second);
                godot::Transform3D custom_bind = per_mesh_skin->get_bind_pose(cit->second);
                godot::Transform3D R = bone_rest * custom_bind;
                godot::Quaternion R_quat = R.basis.orthonormalized().get_quaternion();

                // Log prefix alongside R to detect double-counting
                godot::Transform3D pfx_godot = nif_matrix44_to_godot(prefix_nif);
                godot::Quaternion pfx_q = pfx_godot.basis.orthonormalized().get_rotation_quaternion();
                float pfx_angle = godot::Math::rad_to_deg(pfx_q.get_angle());

                UtilityFunctions::print("[SKINFIX] mesh='",
                    String::utf8(nif_display_name(StaticCast<NiObject>(geom)).c_str()),
                    "' bone='", skeleton->get_bone_name(cit->second),
                    "' R_origin=(", R.origin.x, ",", R.origin.y, ",", R.origin.z,
                    ") R_quat=(", R_quat.x, ",", R_quat.y, ",", R_quat.z, ",", R_quat.w,
                    ") prefix_origin=(", pfx_godot.origin.x, ",", pfx_godot.origin.y, ",", pfx_godot.origin.z,
                    ") prefix_angle=", pfx_angle, "deg");
                break;
            }

            // Track rotation correction for non-re-parented skeletons.
            {
                godot::Transform3D correction;
                bool found_bone = false;
                for (unsigned int ci = 0; ci < skin_bone_count; ++ci) {
                    if (skin_bones[ci] == NULL) continue;
                    auto cit = bone_index_map.find(skin_bones[ci]);
                    if (cit == bone_index_map.end()) continue;
                    godot::Transform3D bone_rest = compute_bone_global_rest(skeleton, cit->second);
                    godot::Transform3D R = bone_rest * per_mesh_skin->get_bind_pose(cit->second);
                    correction = godot::Transform3D(R.basis.inverse().orthonormalized(), godot::Vector3());
                    found_bone = true;
                    break;
                }
                if (found_bone) {
                    rotation_correction_mismatch_count[skel_root]++;
                    auto rc_it = rotation_correction_cache.find(skel_root);
                    if (rc_it == rotation_correction_cache.end()) {
                        rotation_correction_cache[skel_root] = correction;
                        rotation_correction_consistent[skel_root] = true;
                    } else if (rotation_correction_consistent[skel_root]) {
                        godot::Quaternion stored_q = rc_it->second.basis.orthonormalized().get_rotation_quaternion();
                        godot::Quaternion this_q = correction.basis.orthonormalized().get_rotation_quaternion();
                        float dot = std::abs(stored_q.dot(this_q));
                        float angle_deg = dot < 1.0f
                            ? godot::Math::rad_to_deg(2.0f * std::acos(std::min(dot, 1.0f)))
                            : 0.0f;
                        if (angle_deg > 8.0f) {
                            rotation_correction_consistent[skel_root] = false;
                        }
                    }
                }
            }
        }
    }

    // --- Build mesh via SurfaceTool ---
    Ref<SurfaceTool> st;
    st.instantiate();
    st->begin(Mesh::PRIMITIVE_TRIANGLES);

    // Per-vertex attributes (order matters: set attributes BEFORE add_vertex)
    for (size_t i = 0; i < vertices.size(); ++i) {
        // Convert vertex position from NIF to Godot coordinates
        godot::Vector3 pos = nif_to_godot_vec3(vertices[i]);
        godot::Vector3 norm;
        if (has_normals) norm = nif_to_godot_vec3(normals[i]);

        // For skinned meshes: bake the geometry's local transform into vertices.
        // The MeshInstance3D sits at identity under Skeleton3D (in parent NiNode space),
        // so vertices must be in parent NiNode space. The geometry's local transform
        // maps from geometry space to parent NiNode space — same as apply_nif_transform
        // does for non-skinned meshes, but baked into vertex data.
        if (is_skinned) {
            pos = skin_vertex_transform.xform(pos);
            if (has_normals) norm = skin_vertex_transform.basis.xform(norm).normalized();
        }

        if (has_normals) st->set_normal(norm);
        if (has_uvs)     st->set_uv(godot::Vector2(uvs[i].u, uvs[i].v));
        if (has_colors)  st->set_color(godot::Color(colors[i].r, colors[i].g, colors[i].b, colors[i].a));
        if (is_skinned && i < vertex_weights.size()) {
            PackedInt32Array bone_ids;
            PackedFloat32Array bone_wts;
            bone_ids.resize(4);
            bone_wts.resize(4);
            for (int j = 0; j < 4; ++j) {
                bone_ids[j] = vertex_weights[i][j].first;
                bone_wts[j] = vertex_weights[i][j].second;
            }
            st->set_bones(bone_ids);
            st->set_weights(bone_wts);
        }
        st->add_vertex(pos);
    }

    // --- Triangle winding ---
    // NIF (left-handed) uses CW front-face; Godot (right-handed) uses CCW.
    // The coordinate conversion nif_to_godot_vec3 preserves cross products
    // (det=+1, orthogonal) but the handedness change reverses which winding
    // is "front".  Swapping v2↔v3 converts CW → CCW.
    // Without this swap, all meshes render inside-out (backfaces visible).
    //
    // Confirmed via diagnostic on artillery.nif (478 tris, dot>=0 for all):
    // NIF winding is consistent; swap is universally needed.
    for (const auto& tri : triangles) {
        st->add_index(tri.v1);
        st->add_index(tri.v3);
        st->add_index(tri.v2);
    }

    // Only generate normals if the NIF didn't provide them.
    // NIF normals are already correct; unconditional generate_normals()
    // overrides them and can produce inverted lighting.
    if (!has_normals) st->generate_normals();

    // Generate tangents for Forward+ renderer (needs TBN for normal-mapped materials).
    if (has_uvs) st->generate_tangents();

    Ref<ArrayMesh> mesh = st->commit();
    if (!mesh.is_valid()) return nullptr;

    // --- MeshInstance3D + material ---
    MeshInstance3D* mesh_instance = memnew(MeshInstance3D);
    std::string name = nif_display_name(StaticCast<NiObject>(geom));
    mesh_instance->set_name(String::utf8(name.c_str()));
    mesh_instance->set_mesh(mesh);

    std::vector<Niflib::Ref<NiProperty>> properties = geom->GetProperties();
    Ref<Material> mat = create_material_from_properties(properties, has_colors, base_path, name);
    mesh_instance->set_surface_override_material(0, mat);

    if (is_skinned && skeleton != nullptr) {
        mesh_instance->set_skeleton_path(NodePath(".."));
        if (per_mesh_skin.is_valid()) {
            mesh_instance->set_skin(per_mesh_skin);
        } else {
            NiNodeRef skel_root = skin->GetSkeletonRoot();
            auto skin_it = skin_cache.find(skel_root);
            if (skin_it != skin_cache.end() && skin_it->second.is_valid()) {
                mesh_instance->set_skin(skin_it->second);
            }
        }
    } else {
        apply_nif_transform(StaticCast<NiAVObject>(geom), mesh_instance);
    }

    return mesh_instance;
}

// Thin wrappers — extract type-specific ref and delegate to process_tri_geometry().
Node3D* GdextNiflib::process_ni_tri_shape(NiTriShapeRef tri_shape, const String& base_path,
    Skeleton3D* skeleton, NiNodeRef parent_ni_node) {
    return process_tri_geometry(DynamicCast<NiTriBasedGeom>(tri_shape), base_path, skeleton, parent_ni_node);
}

Node3D* GdextNiflib::process_ni_tri_strips(NiTriStripsRef tri_strips, const String& base_path,
    Skeleton3D* skeleton, NiNodeRef parent_ni_node) {
    return process_tri_geometry(DynamicCast<NiTriBasedGeom>(tri_strips), base_path, skeleton, parent_ni_node);
}

// =============================================================================
// Bone attachment reparenting
// =============================================================================
// After process_ni_node(), attachment nodes (weapons, props) sit under static
// bone-mirror Node3Ds.  This pass finds NIF nodes whose parent is a skeleton
// bone but which are NOT bones themselves, and reparents the corresponding
// Godot Node3D under a BoneAttachment3D on the Skeleton3D.

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

    // Iterate scene graph children only
    std::vector<NiAVObjectRef> children = ni_node->GetChildren();
    for (const auto& child : children) {
        if (child == NULL) continue;

        if (NiNodeRef child_node = DynamicCast<NiNode>(child)) {
            process_ni_node(child_node, godot_node, base_path);
        }
        else if (NiTriShapeRef tri_shape = DynamicCast<NiTriShape>(child)) {
            // Check if this shape is skinned
            NiSkinInstanceRef skin = tri_shape->GetSkinInstance();
            std::string shape_name = nif_display_name(StaticCast<NiObject>(tri_shape));
            if (skin != NULL) {
                NiNodeRef skel_root = skin->GetSkeletonRoot();
                std::vector<NiNodeRef> skin_bones = skin->GetBones();
                // Build or reuse Skeleton3D for this skeleton root
                Skeleton3D* skeleton = nullptr;

                auto cache_it = skeleton_cache.find(skel_root);
                if (cache_it != skeleton_cache.end()) {
                    skeleton = cache_it->second;

                    // Re-parent skeleton to common ancestor if this mesh is on a
                    // different branch than the skeleton's current host (sibling fix).
                    auto host_it = skeleton_host_map.find(skel_root);
                    if (host_it != skeleton_host_map.end()) {
                        Niflib::NiNodeRef current_host = host_it->second;
                        if (ni_node != current_host && !is_nif_descendant(ni_node, current_host)) {
                            Niflib::NiNodeRef common = find_common_ancestor(ni_node, current_host);
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

                    // Rebuild bone_index_map for this shape's bone set from the cached skeleton.
                    // The cached skeleton was built for a previous shape; its bone_index_map may
                    // be stale if this shape references different bone nodes.
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
                        // Regenerate default skin for the expanded skeleton
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
                    Node3D* mesh_node = process_ni_tri_shape(tri_shape, base_path, skeleton, ni_node);
                    if (mesh_node) {
                        skeleton->add_child(mesh_node);
                    }
                } else {
                    UtilityFunctions::push_warning("[SKIN] Skeleton build failed, treating as static: ",
                        String::utf8(shape_name.c_str()));
                    Node3D* mesh_node = process_ni_tri_shape(tri_shape, base_path);
                    if (mesh_node) godot_node->add_child(mesh_node);
                }
            } else {
                Node3D* mesh_node = process_ni_tri_shape(tri_shape, base_path);
                if (mesh_node) {
                    godot_node->add_child(mesh_node);
                }
            }
        }
        else if (NiTriStripsRef tri_strips = DynamicCast<NiTriStrips>(child)) {
            NiSkinInstanceRef skin = tri_strips->GetSkinInstance();
            std::string strip_name = nif_display_name(StaticCast<NiObject>(tri_strips));
            if (skin != NULL) {
                NiNodeRef skel_root = skin->GetSkeletonRoot();
                std::vector<NiNodeRef> skin_bones = skin->GetBones();
                Skeleton3D* skeleton = nullptr;
                auto cache_it = skeleton_cache.find(skel_root);
                if (cache_it != skeleton_cache.end()) {
                    skeleton = cache_it->second;

                    // Re-parent skeleton to common ancestor if sibling branch detected.
                    auto host_it = skeleton_host_map.find(skel_root);
                    if (host_it != skeleton_host_map.end()) {
                        Niflib::NiNodeRef current_host = host_it->second;
                        if (ni_node != current_host && !is_nif_descendant(ni_node, current_host)) {
                            Niflib::NiNodeRef common = find_common_ancestor(ni_node, current_host);
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

                    // Rebuild bone_index_map for this shape's bone set from the cached skeleton.
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
                        // Regenerate default skin for the expanded skeleton
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
                    Node3D* mesh_node = process_ni_tri_strips(tri_strips, base_path, skeleton, ni_node);
                    if (mesh_node) {
                        skeleton->add_child(mesh_node);
                    }
                } else {
                    Node3D* mesh_node = process_ni_tri_strips(tri_strips, base_path);
                    if (mesh_node) godot_node->add_child(mesh_node);
                }
            } else {
                Node3D* mesh_node = process_ni_tri_strips(tri_strips, base_path);
                if (mesh_node) {
                    godot_node->add_child(mesh_node);
                }
            }
        }
        else {
            UtilityFunctions::print("Skipping unhandled NIF type: ",
                String::utf8(child->GetType().GetTypeName().c_str()));
        }
    }
}

