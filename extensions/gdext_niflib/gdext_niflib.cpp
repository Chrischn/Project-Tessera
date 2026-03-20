// =============================================================================
// File:              gdext_niflib.cpp
// Author(s):         Chrischn89
// Godot Version:     4.5
// Description:
//   Entry point: _bind_methods, load_nif_scene orchestrator, version queries
//
// License:
//   Released under the terms of the GNU General Public License version 3.0
// =============================================================================
//
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
#include "nif_coordinate.hpp"

//niflib Headers
#include <NIF_IO.h>
#include <gen/Header.h>
#include <nif_basic_types.h>
#include <obj/NiNode.h>
#include <obj/NiTriShape.h>

//godot-cpp Headers
#include <godot_cpp/variant/utility_functions.hpp>
#include <godot_cpp/classes/mesh_instance3d.hpp>

//Standard Library
#include <exception>


using namespace godot;
using namespace Niflib;

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
                // For re-parented skeletons: only skip if prefix was large (>5°).
                // Re-parenting fixes prefix double-counting but NOT composition law errors.
                // Near-zero prefix means R is purely composition error → still needs correction.
                if (reparented_skeletons.count(skel_root_ni)) {
                    auto pfx_first = prefix_all_first.find(skel_root_ni);
                    if (pfx_first != prefix_all_first.end()) {
                        float pfx_angle = godot::Math::rad_to_deg(pfx_first->second.get_angle());
                        if (pfx_angle > 5.0f) continue;
                    } else {
                        continue;
                    }
                }
                auto rc_it = rotation_correction_cache.find(skel_root_ni);
                if (rc_it == rotation_correction_cache.end()) continue;
                auto con_it = rotation_correction_consistent.find(skel_root_ni);
                if (con_it == rotation_correction_consistent.end() || !con_it->second) continue;
                int skinned_mesh_count = 0;
                for (int ci = 0; ci < skel->get_child_count(); ++ci) {
                    MeshInstance3D* mi = Object::cast_to<MeshInstance3D>(skel->get_child(ci));
                    if (mi && mi->get_skin().is_valid()) skinned_mesh_count++;
                }
                // Apply correction when:
                // 1. At least one mismatch mesh exists (correction was computed)
                // 2. All skinned meshes share the same prefix (correction is safe for all)
                auto mc_it = rotation_correction_mismatch_count.find(skel_root_ni);
                int mismatch_count = (mc_it != rotation_correction_mismatch_count.end()) ? mc_it->second : 0;
                auto pfx_con = prefix_all_consistent.find(skel_root_ni);
                bool all_prefix_same = pfx_con != prefix_all_consistent.end() && pfx_con->second;

                // Diagnostic: log the deferred correction decision for every skeleton
                {
                    bool is_reparented = reparented_skeletons.count(skel_root_ni) > 0;
                    auto pfx_f = prefix_all_first.find(skel_root_ni);
                    float pfx_a = (pfx_f != prefix_all_first.end())
                        ? godot::Math::rad_to_deg(pfx_f->second.get_angle()) : -1.0f;
                    godot::Quaternion corr_q = rc_it->second.basis.orthonormalized().get_rotation_quaternion();
                    float corr_angle = godot::Math::rad_to_deg(corr_q.get_angle());
                    bool will_apply = (mismatch_count > 0 && all_prefix_same);
                    UtilityFunctions::print("[CORR] skel_bones=", skel->get_bone_count(),
                        " reparented=", is_reparented ? "YES" : "NO",
                        " pfx_angle=", pfx_a,
                        " pfx_consistent=", all_prefix_same ? "YES" : "NO",
                        " mismatch=", mismatch_count, "/", skinned_mesh_count,
                        " R_consistent=", con_it->second ? "YES" : "NO",
                        " corr_angle=", corr_angle,
                        " APPLIED=", will_apply ? "YES" : "NO");
                }

                if (mismatch_count > 0 && all_prefix_same) {
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
        // Build scene-graph controller animations (UV scroll, flip, alpha, color, visibility)
        build_scene_animations(base_path, godotnode);
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
        prefix_all_first.clear();
        prefix_all_consistent.clear();
        pending_scene_controllers.clear();

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


