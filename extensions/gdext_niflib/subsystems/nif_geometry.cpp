// =============================================================================
// File:              nif_geometry.cpp
// Author(s):         Chrischn89
// Godot Version:     4.5
// Description:
//   Mesh building: NiTriShape/NiTriStrips -> Godot ArrayMesh via SurfaceTool
//
// License:
//   Released under the terms of the GNU General Public License version 3.0
// =============================================================================
#include "gdext_niflib.hpp"
#include "nif_coordinate.hpp"

#include <obj/NiTriShape.h>
#include <obj/NiTriShapeData.h>
#include <obj/NiTriStrips.h>
#include <obj/NiTriStripsData.h>
#include <obj/NiTriBasedGeom.h>
#include <obj/NiTriBasedGeomData.h>
#include <obj/NiSkinInstance.h>
#include <obj/NiSkinData.h>
#include <obj/NiSkinPartition.h>
#include <obj/NiLines.h>
#include <gen/SkinWeight.h>

#include <godot_cpp/variant/utility_functions.hpp>
#include <godot_cpp/classes/array_mesh.hpp>
#include <godot_cpp/classes/surface_tool.hpp>
#include <godot_cpp/classes/mesh_instance3d.hpp>

using namespace godot;
using namespace Niflib;

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

        // Track prefix consistency across ALL skinned meshes on this skeleton.
        // Correction is only safe when all meshes share the same prefix rotation.
        {
            Niflib::NiNodeRef pfx_skel_root = skin->GetSkeletonRoot();
            godot::Transform3D pfx = nif_matrix44_to_godot(prefix_nif);
            godot::Quaternion pfx_q = pfx.basis.orthonormalized().get_rotation_quaternion();
            auto pfx_it = prefix_all_first.find(pfx_skel_root);
            if (pfx_it == prefix_all_first.end()) {
                prefix_all_first[pfx_skel_root] = pfx_q;
                prefix_all_consistent[pfx_skel_root] = true;
            } else if (prefix_all_consistent[pfx_skel_root]) {
                float dot = std::abs(pfx_it->second.dot(pfx_q));
                float angle = dot < 1.0f
                    ? godot::Math::rad_to_deg(2.0f * std::acos(std::min(dot, 1.0f)))
                    : 0.0f;
                if (angle > 15.0f) {
                    prefix_all_consistent[pfx_skel_root] = false;
                }
            }
        }

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

            // Track hybrid correction for non-re-parented skeletons.
            // When prefix is large (>5°): use inv(prefix) to remove only double-counting.
            // When prefix is near zero: use inv(R) to fix composition law error.
            {
                godot::Transform3D pfx = nif_matrix44_to_godot(prefix_nif);
                float pfx_angle = godot::Math::rad_to_deg(
                    pfx.basis.orthonormalized().get_rotation_quaternion().get_angle());

                godot::Transform3D correction;
                bool found_correction = false;

                if (pfx_angle > 5.0f) {
                    // Large prefix: use inv(prefix), but only if R is also significant (>5°).
                    // If R ≈ 0° (positional-only mismatch), no rotation correction needed.
                    for (unsigned int ci = 0; ci < skin_bone_count; ++ci) {
                        if (skin_bones[ci] == NULL) continue;
                        auto cit = bone_index_map.find(skin_bones[ci]);
                        if (cit == bone_index_map.end()) continue;
                        godot::Transform3D br = compute_bone_global_rest(skeleton, cit->second);
                        godot::Transform3D R = br * per_mesh_skin->get_bind_pose(cit->second);
                        float R_deg = godot::Math::rad_to_deg(
                            R.basis.orthonormalized().get_rotation_quaternion().get_angle());
                        if (R_deg > 5.0f) {
                            correction = godot::Transform3D(pfx.basis.inverse().orthonormalized(), godot::Vector3());
                            found_correction = true;
                        }
                        break;
                    }
                } else {
                    // Near-zero prefix: use inv(R) to correct composition law error
                    for (unsigned int ci = 0; ci < skin_bone_count; ++ci) {
                        if (skin_bones[ci] == NULL) continue;
                        auto cit = bone_index_map.find(skin_bones[ci]);
                        if (cit == bone_index_map.end()) continue;
                        godot::Transform3D bone_rest = compute_bone_global_rest(skeleton, cit->second);
                        godot::Transform3D R = bone_rest * per_mesh_skin->get_bind_pose(cit->second);
                        correction = godot::Transform3D(R.basis.inverse().orthonormalized(), godot::Vector3());
                        found_correction = true;
                        break;
                    }
                }

                if (found_correction) {
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
                        if (angle_deg > 15.0f) {
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

    // Collect scene-graph controllers on this geometry for build_scene_animations()
    process_scene_controllers(StaticCast<NiObjectNET>(geom), mesh_instance);

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

Node3D* GdextNiflib::process_ni_lines(NiLinesRef lines, const String& base_path) {
    // STUB: NiLines not yet implemented.
    // NiLines is debug line geometry (outlines, wireframes). Rare in Civ4.
    // Godot equivalent: ImmediateMesh with PRIMITIVE_LINES.
    UtilityFunctions::print("[STUB] NiLines skipped: '",
        String::utf8(nif_display_name(StaticCast<NiObject>(lines)).c_str()), "'");
    return nullptr;
}
