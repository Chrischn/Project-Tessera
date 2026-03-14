// =============================================================================
// gdext_niflib.cpp
// NIF -> Godot scene translation for Civilization IV NIF files (version 20.0.0.4).
//
// Responsibilities:
//   - Coordinate system conversion (NIF left-handed Z-up -> Godot right-handed Y-up)
//   - Scene graph traversal: NiNode hierarchy -> Godot Node3D hierarchy
//   - Geometry processing: NiTriShape / NiTriStrips -> ArrayMesh (via SurfaceTool)
//   - Material and texture loading: NiProperty set -> StandardMaterial3D
//   - Skeletal skinning: NiSkinInstance -> Skeleton3D + Skin resource
//
// Key NIF types used: NiNode, NiTriShape, NiTriStrips, NiSkinInstance, NiSkinData,
//   NiTexturingProperty, NiMaterialProperty, NiAlphaProperty, NiVertexColorProperty.
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
//Standard Library
#include <vector>
#include <algorithm>
#include <exception>


using namespace godot;
using namespace Niflib;

// =============================================================================
// Coordinate conversion helpers
// =============================================================================
// NIF (Civ IV v20.0.0.4): left-handed, Z-up, Y-forward
// Godot:                  right-handed, Y-up, Z-backward
//
// Per-component mapping: Godot(x, y, z) = NIF(x, z, -y)
// Applied to all vertex positions, normals, and translation vectors.
// Rotation matrices use: R_godot = P * R_nif * P^T  (see nif_matrix44_to_godot)
// Winding order fix: triangle indices v2/v3 are swapped (handedness flip reverses
// front face direction; swap restores correct CCW winding for Godot).
static godot::Vector3 nif_to_godot_vec3(const Niflib::Vector3& v) {
    return godot::Vector3(v.x, v.z, -v.y);
}

// Converts a NIF Matrix44 (4x4 transform) to a Godot Transform3D with coordinate system conversion.
godot::Transform3D GdextNiflib::nif_matrix44_to_godot(const Niflib::Matrix44& mat) {
    // Extract the 3x3 rotation and translation from Matrix44
    Niflib::Matrix33 r;
    Niflib::Vector3 t;
    float scale;
    mat.Decompose(t, r, scale);

    // Convert translation
    godot::Vector3 origin = nif_to_godot_vec3(t);

    // Convert rotation (same formula as apply_nif_transform: R_godot = P * R_nif * P^T)
    godot::Basis basis;
    basis.set_column(0, godot::Vector3( r[0][0],  r[2][0], -r[1][0]));
    basis.set_column(1, godot::Vector3( r[0][2],  r[2][2], -r[1][2]));
    basis.set_column(2, godot::Vector3(-r[0][1], -r[2][1],  r[1][1]));

    // Apply uniform scale
    if (std::abs(scale - 1.0f) > 0.0001f) {
        basis.set_column(0, basis.get_column(0) * scale);
        basis.set_column(1, basis.get_column(1) * scale);
        basis.set_column(2, basis.get_column(2) * scale);
    }

    return godot::Transform3D(basis, origin);
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
// Populates bone_index_map (NiNode* -> Godot bone index) for weight assignment.
Skeleton3D* GdextNiflib::build_skeleton(NiSkinInstanceRef skin, NiNodeRef parent_ni_node) {
    if (!skin) return nullptr;

    NiSkinDataRef skin_data = skin->GetSkinData();
    if (!skin_data) return nullptr;

    std::vector<NiNodeRef> bones = skin->GetBones();
    unsigned int bone_count = (unsigned int)bones.size();
    if (bone_count == 0) return nullptr;

    Skeleton3D* skeleton = memnew(Skeleton3D);
    skeleton->set_name("Skeleton3D");

    // Step 1: Add all bones and build index map
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

    // Step 3: Compute rest poses from bone world transforms.
    // Express bone transforms relative to the parent NiNode so that the
    // Skeleton3D (which sits under the parent's Godot node) doesn't
    // double-apply the hierarchy transforms.
    godot::Transform3D parent_world_godot;
    if (parent_ni_node != NULL) {
        parent_world_godot = nif_matrix44_to_godot(parent_ni_node->GetWorldTransform());
    }

    std::vector<godot::Transform3D> bone_globals(bone_count);
    for (unsigned int i = 0; i < bone_count; ++i) {
        NiNodeRef bone = bones[i];
        if (bone == NULL) continue;

        godot::Transform3D bone_world = nif_matrix44_to_godot(bone->GetWorldTransform());
        // Convert from world space to skeleton-local space
        if (parent_ni_node != NULL) {
            bone_globals[i] = parent_world_godot.affine_inverse() * bone_world;
        } else {
            bone_globals[i] = bone_world;
        }
    }

    // Set rest transforms (local to parent bone)
    for (unsigned int i = 0; i < bone_count; ++i) {
        NiNodeRef bone = bones[i];
        if (bone == NULL) continue;

        int parent_idx = skeleton->get_bone_parent(bone_index_map[bone]);
        godot::Transform3D rest;
        if (parent_idx >= 0) {
            // Find which NIF bone has this parent index
            godot::Transform3D parent_global;
            for (unsigned int j = 0; j < bone_count; ++j) {
                if (bones[j] != NULL && bone_index_map[bones[j]] == parent_idx) {
                    parent_global = bone_globals[j];
                    break;
                }
            }
            rest = parent_global.affine_inverse() * bone_globals[i];
        } else {
            // Root bone: rest relative to skeleton origin
            rest = bone_globals[i];
        }

        skeleton->set_bone_rest(bone_index_map[bone], rest);
    }

    // Initialize bone poses from rests.
    // In Godot 4, bone poses default to identity -- NOT the rest transform.
    // Without this, skinning computes: T = identity * inverse_rest = inverse_rest != identity.
    skeleton->reset_bone_poses();

    NiNodeRef skel_root = skin->GetSkeletonRoot();
    UtilityFunctions::print("[SKIN] Built skeleton: ", bone_count, " bones, root=",
        skel_root ? String::utf8(skel_root->GetName().c_str()) : String("<null>"));

    // Create explicit Skin with inverse-rest bind poses so skinning gives identity at rest.
    // Without this, Godot uses identity bind poses, applying bone_global_rest directly to
    // vertices (bone_global_rest * identity != identity), causing distortion.
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
    Ref<ImmediateMesh> line_mesh;
    line_mesh.instantiate();
    line_mesh->surface_begin(Mesh::PRIMITIVE_LINES);

    bool has_lines = false;
    for (int i = 0; i < bone_count; ++i) {
        int parent_idx = skeleton->get_bone_parent(i);
        if (parent_idx >= 0 && parent_idx < bone_count) {
            line_mesh->surface_add_vertex(bone_positions[parent_idx]);
            line_mesh->surface_add_vertex(bone_positions[i]);
            has_lines = true;
        }
    }
    line_mesh->surface_end();

    if (has_lines) {
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
            UtilityFunctions::print("[TEX] Fallback trying: ", path);
            err = img->load(path);
            if (err == OK) break;
        }
    }

    if (err != OK) {
        UtilityFunctions::print("[TEX] All paths failed for: ", rel_path);
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

        // Clear per-NIF state
        texture_cache.clear();
        current_nif_dir = "";
        bone_index_map.clear();
        skeleton_cache.clear();

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

    // Translation
    Niflib::Vector3 t = av_obj->GetLocalTranslation();
    godot_node->set_position(nif_to_godot_vec3(t));

    // Uniform scale
    float s = av_obj->GetLocalScale();
    godot_node->set_scale(godot::Vector3(s, s, s));

    // Rotation: NIF Matrix33 -> Godot Basis
    // Coordinate transform: R_godot = P * R_nif * P^T
    // where P swaps Y<->Z and negates new Z.
    Niflib::Matrix33 r = av_obj->GetLocalRotation();
    godot::Basis basis;
    basis.set_column(0, godot::Vector3( r[0][0],  r[2][0], -r[1][0]));  // X column
    basis.set_column(1, godot::Vector3( r[0][2],  r[2][2], -r[1][2]));  // Y column (was Z)
    basis.set_column(2, godot::Vector3(-r[0][1], -r[2][1],  r[1][1]));  // Z column (was -Y)
    godot_node->set_basis(basis);
}

// Creates a Godot StandardMaterial3D from NIF property list.
// Phase 3: material colors from NiMaterialProperty.
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
    // Captured for ShaderMaterial passthrough (populated in the blocks below)
    float alpha_scissor_threshold_val = 0.0f;
    godot::Ref<godot::ImageTexture> dark_map_tex;   // populated from TeamColor.bmp (BASE_MAP slot 0) when detected
    godot::Ref<godot::ImageTexture> albedo_tex_ref;  // stored for direct ShaderMaterial transfer (avoids get_texture)
    godot::Ref<godot::ImageTexture> normal_tex_ref;  // stored for direct ShaderMaterial transfer

    for (const auto& prop : properties) {
        if (prop == NULL) continue;
        if (auto m = DynamicCast<NiMaterialProperty>(prop)) mat_prop = m;
        if (auto a = DynamicCast<NiAlphaProperty>(prop))    alpha_prop = a;
        if (auto t = DynamicCast<NiTexturingProperty>(prop)) tex_prop = t;
    }

    if (mat_prop) {
        // Diffuse color + alpha
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

    // Vertex colors
    if (has_vertex_colors) {
        mat->set_flag(StandardMaterial3D::FLAG_SRGB_VERTEX_COLOR, true);
    }

    // Phase 4: Textures from NiTexturingProperty
    if (tex_prop != NULL) {
        // Dump all texture slots for debugging
        const char* slot_names[] = {"BASE_MAP","DARK_MAP","DETAIL_MAP","GLOSS_MAP",
            "GLOW_MAP","BUMP_MAP","NORMAL_MAP","UNKNOWN2","DECAL_0","DECAL_1","DECAL_2","DECAL_3"};
        for (int s = 0; s < 12; ++s) {
            if (tex_prop->HasTexture(s)) {
                TexDesc td = tex_prop->GetTexture(s);
                String fname = (td.source != NULL && td.source->IsTextureExternal())
                    ? String::utf8(td.source->GetTextureFileName().c_str()) : String("(internal)");
                UtilityFunctions::print("[TEX] slot ", s, " (", slot_names[s], "): ", fname);
            }
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
                albedo_loaded = true;
            }
        }
        // GLOSS_MAP (slot 3) -> roughness texture
        // Civ IV GLOSS_MAP is glossiness (bright = glossy = low roughness).
        // Godot's roughness texture is the inverse (bright = rough).
        // TODO: invert gloss->roughness for accurate PBR reproduction.
        if (tex_prop->HasTexture(3)) {
            TexDesc gloss_desc = tex_prop->GetTexture(3);
            if (gloss_desc.source != NULL && gloss_desc.source->IsTextureExternal()) {
                auto tex = load_dds_texture(base_path, gloss_desc.source->GetTextureFileName());
                if (tex.is_valid()) {
                    mat->set_texture(StandardMaterial3D::TEXTURE_ROUGHNESS, tex);
                    UtilityFunctions::print("[TEX] Roughness (GLOSS_MAP) from slot 3: ",
                        String::utf8(gloss_desc.source->GetTextureFileName().c_str()));
                }
            }
        }
        // GLOW_MAP (slot 4) — skipped.
        // Civ IV uses this slot for environment/reflection maps, not emission.
        // TODO: revisit when environment mapping is implemented.
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

    // Phase 5: Alpha / Transparency from NiAlphaProperty
    if (alpha_prop != NULL) {
        bool blend = alpha_prop->GetBlendState();
        bool test = alpha_prop->GetTestState();
        unsigned short src_blend = alpha_prop->GetSourceBlendFunc();
        unsigned short dst_blend = alpha_prop->GetDestBlendFunc();
        unsigned int threshold = alpha_prop->GetTestThreshold();
        // Capture threshold for ShaderMaterial if this shape later uses one.
        // Only when blend=false: blend=true means team-color mask, not cutout.
        if (threshold > 0 && test && !blend) { alpha_scissor_threshold_val = threshold / 255.0f; }

        // Only apply ALPHA_SCISSOR for pure cutout shapes (blend=false, test=true).
        // blend=true means Civ IV is using alpha for team-color blending — the DXT3 alpha
        // channel stores a mask (0 = colorable area, 1 = base texture), not transparency.
        // Applying ALPHA_SCISSOR there discards the colorable-area pixels and creates holes.
        if (threshold > 0 && test && !blend) {
            mat->set_transparency(StandardMaterial3D::TRANSPARENCY_ALPHA_SCISSOR);
            mat->set_alpha_scissor_threshold(threshold / 255.0f);
        }
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

    // If a DARK_MAP mask was loaded, return a ShaderMaterial that performs
    // the team-color blend.  Otherwise return the StandardMaterial3D as usual.
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

// Processes a NiTriShape: builds a MeshInstance3D with geometry + material.
// If skeleton is non-null, detects NiSkinInstance and adds bone weights.
Node3D* GdextNiflib::process_ni_tri_shape(NiTriShapeRef tri_shape, const String& base_path, Skeleton3D* skeleton) {
    if (!tri_shape) return nullptr;

    // Get geometry data
    NiGeometryDataRef geom_data = tri_shape->GetData();
    NiTriShapeDataRef data = DynamicCast<NiTriShapeData>(geom_data);
    if (!data) {
        UtilityFunctions::print("NiTriShape has no data, skipping.");
        return nullptr;
    }

    // Extract all geometry arrays
    std::vector<Niflib::Vector3> vertices = data->GetVertices();
    std::vector<Niflib::Triangle> triangles = data->GetTriangles();
    if (vertices.empty() || triangles.empty()) return nullptr;

    std::vector<Niflib::Vector3> normals = data->GetNormals();
    bool has_normals = !normals.empty() && (normals.size() == vertices.size());

    int uv_count = data->GetUVSetCount();
    std::vector<Niflib::TexCoord> uvs;
    if (uv_count > 0) {
        uvs = data->GetUVSet(0);
    }
    bool has_uvs = !uvs.empty() && (uvs.size() == vertices.size());

    std::vector<Niflib::Color4> colors = data->GetColors();
    bool has_colors = !colors.empty() && (colors.size() == vertices.size());

    // --- Skinning detection ---
    // NIF stores per-bone vertex weights in NiSkinInstance/NiSkinData.
    NiSkinInstanceRef skin = tri_shape->GetSkinInstance();
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

            // Build the geometry's local transform (same components as apply_nif_transform)
            {
                NiAVObjectRef av = StaticCast<NiAVObject>(tri_shape);
                Niflib::Vector3 gt = av->GetLocalTranslation();
                Niflib::Matrix33 gr = av->GetLocalRotation();
                float gs = av->GetLocalScale();

                godot::Vector3 origin = nif_to_godot_vec3(gt);
                godot::Basis basis;
                basis.set_column(0, godot::Vector3( gr[0][0],  gr[2][0], -gr[1][0]));
                basis.set_column(1, godot::Vector3( gr[0][2],  gr[2][2], -gr[1][2]));
                basis.set_column(2, godot::Vector3(-gr[0][1], -gr[2][1],  gr[1][1]));
                if (std::abs(gs - 1.0f) > 0.0001f) {
                    basis.set_column(0, basis.get_column(0) * gs);
                    basis.set_column(1, basis.get_column(1) * gs);
                    basis.set_column(2, basis.get_column(2) * gs);
                }
                skin_vertex_transform = godot::Transform3D(basis, origin);
            }
            UtilityFunctions::print("[SKIN] geom_local origin=(",
                skin_vertex_transform.origin.x, ", ",
                skin_vertex_transform.origin.y, ", ",
                skin_vertex_transform.origin.z, ")");

            std::vector<NiNodeRef> bones = skin->GetBones();
            unsigned int bone_count = skin_data->GetBoneCount();

            // Invert per-bone weights to per-vertex
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

            // Godot GPU skinning supports exactly 4 influences per vertex.
            // Sort descending, truncate to 4, pad with zeros, re-normalize to sum=1.
            for (auto& vw : vertex_weights) {
                std::sort(vw.begin(), vw.end(),
                    [](const std::pair<int,float>& a, const std::pair<int,float>& b) {
                        return a.second > b.second;
                    });
                // Keep top 4
                if (vw.size() > 4) vw.resize(4);
                // Pad to 4
                while (vw.size() < 4) vw.push_back({0, 0.0f});
                // Normalize
                float sum = 0.0f;
                for (auto& p : vw) sum += p.second;
                if (sum > 0.0001f) {
                    for (auto& p : vw) p.second /= sum;
                }
            }

            // Diagnostic: count vertices with non-zero weights from NiSkinData
            unsigned int verts_with_weights = 0;
            unsigned int total_weight_entries = 0;
            for (const auto& vw : vertex_weights) {
                for (const auto& p : vw) {
                    if (p.second > 0.0001f) { ++total_weight_entries; break; }
                }
                // Count separately
                bool has_any = false;
                for (const auto& p : vw) {
                    if (p.second > 0.0001f) { has_any = true; break; }
                }
                if (has_any) ++verts_with_weights;
            }
            NiSkinPartitionRef skin_part = skin->GetSkinPartition();
            UtilityFunctions::print("[SKIN] shape=\"",
                String::utf8(nif_display_name(StaticCast<NiObject>(tri_shape)).c_str()),
                "\" bones=", bone_count,
                " verts=", (int)vertices.size(),
                " verts_with_skindata_weights=", verts_with_weights,
                " skin_partition=", skin_part != NULL ? "YES" : "NO");
        }
    }

    // Build mesh via SurfaceTool
    Ref<SurfaceTool> st;
    st.instantiate();
    st->begin(Mesh::PRIMITIVE_TRIANGLES);

    // Per-vertex attributes (order matters: set attributes BEFORE add_vertex)
    for (size_t i = 0; i < vertices.size(); ++i) {
        // Convert vertex position from NIF to Godot coordinates
        godot::Vector3 pos = nif_to_godot_vec3(vertices[i]);
        godot::Vector3 norm;
        if (has_normals) {
            norm = nif_to_godot_vec3(normals[i]);
        }

        // For skinned meshes: bake the geometry's local transform into vertices.
        // The MeshInstance3D sits at identity under Skeleton3D (in parent NiNode space),
        // so vertices must be in parent NiNode space. The geometry's local transform
        // maps from geometry space to parent NiNode space — same as apply_nif_transform
        // does for non-skinned meshes, but baked into vertex data.
        if (is_skinned) {
            pos = skin_vertex_transform.xform(pos);
            if (has_normals) {
                norm = skin_vertex_transform.basis.xform(norm).normalized();
            }
        }

        if (has_normals) {
            st->set_normal(norm);
        }
        if (has_uvs) {
            st->set_uv(godot::Vector2(uvs[i].u, uvs[i].v));
        }
        if (has_colors) {
            st->set_color(godot::Color(colors[i].r, colors[i].g, colors[i].b, colors[i].a));
        }
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

    // Swap v2↔v3 to convert NIF winding to Godot's CCW front-face convention.
    // Empirically verified: without swap the model renders inverted (back-facing).
    for (const auto& tri : triangles) {
        st->add_index(tri.v1);
        st->add_index(tri.v3);
        st->add_index(tri.v2);
    }

    if (!has_normals) {
        st->generate_normals();
    }

    Ref<ArrayMesh> mesh = st->commit();
    if (!mesh.is_valid()) return nullptr;

    // Create MeshInstance3D
    MeshInstance3D* mesh_instance = memnew(MeshInstance3D);
    std::string name = nif_display_name(StaticCast<NiObject>(tri_shape));
    mesh_instance->set_name(String::utf8(name.c_str()));
    mesh_instance->set_mesh(mesh);

    // Material from NIF properties
    std::vector<Niflib::Ref<NiProperty>> properties = tri_shape->GetProperties();
    Ref<Material> mat = create_material_from_properties(properties, has_colors, base_path, name);
    mesh_instance->set_surface_override_material(0, mat);

    if (is_skinned && skeleton != nullptr) {
        // ".." resolves to the parent Skeleton3D in the scene tree.
        // The Skin resource gives bind_pose[i] = bone_global_rest[i].inverse(), so at rest
        // bone_matrix = rest * inverse_rest = identity -> mesh renders in its bind pose.
        mesh_instance->set_skeleton_path(NodePath(".."));
        NiNodeRef skel_root = skin->GetSkeletonRoot();
        auto skin_it = skin_cache.find(skel_root);
        if (skin_it != skin_cache.end() && skin_it->second.is_valid()) {
            mesh_instance->set_skin(skin_it->second);
        }
    } else {
        apply_nif_transform(StaticCast<NiAVObject>(tri_shape), mesh_instance);
    }

    return mesh_instance;
}

// Processes a NiTriStrips: builds a MeshInstance3D with geometry + material.
// If skeleton is non-null, detects NiSkinInstance and adds bone weights.
Node3D* GdextNiflib::process_ni_tri_strips(NiTriStripsRef tri_strips, const String& base_path, Skeleton3D* skeleton) {
    if (!tri_strips) return nullptr;

    NiGeometryDataRef geom_data = tri_strips->GetData();
    NiTriStripsDataRef data = DynamicCast<NiTriStripsData>(geom_data);
    if (!data) {
        UtilityFunctions::print("NiTriStrips has no data, skipping.");
        return nullptr;
    }

    std::vector<Niflib::Vector3> vertices = data->GetVertices();
    std::vector<Niflib::Triangle> triangles = data->GetTriangles();
    if (vertices.empty() || triangles.empty()) return nullptr;

    std::vector<Niflib::Vector3> normals = data->GetNormals();
    bool has_normals = !normals.empty() && (normals.size() == vertices.size());

    int uv_count = data->GetUVSetCount();
    std::vector<Niflib::TexCoord> uvs;
    if (uv_count > 0) {
        uvs = data->GetUVSet(0);
    }
    bool has_uvs = !uvs.empty() && (uvs.size() == vertices.size());

    std::vector<Niflib::Color4> colors = data->GetColors();
    bool has_colors = !colors.empty() && (colors.size() == vertices.size());

    // Detect skinning (same logic as process_ni_tri_shape)
    NiSkinInstanceRef skin = tri_strips->GetSkinInstance();
    NiSkinDataRef skin_data;
    bool is_skinned = false;
    godot::Transform3D skin_vertex_transform;
    std::vector<std::vector<std::pair<int, float>>> vertex_weights;

    if (skin != NULL && skeleton != nullptr) {
        skin_data = skin->GetSkinData();
        if (skin_data != NULL) {
            is_skinned = true;
            vertex_weights.resize(vertices.size());

            // Build the geometry's local transform (same as apply_nif_transform)
            {
                NiAVObjectRef av = StaticCast<NiAVObject>(tri_strips);
                Niflib::Vector3 gt = av->GetLocalTranslation();
                Niflib::Matrix33 gr = av->GetLocalRotation();
                float gs = av->GetLocalScale();

                godot::Vector3 origin = nif_to_godot_vec3(gt);
                godot::Basis basis;
                basis.set_column(0, godot::Vector3( gr[0][0],  gr[2][0], -gr[1][0]));
                basis.set_column(1, godot::Vector3( gr[0][2],  gr[2][2], -gr[1][2]));
                basis.set_column(2, godot::Vector3(-gr[0][1], -gr[2][1],  gr[1][1]));
                if (std::abs(gs - 1.0f) > 0.0001f) {
                    basis.set_column(0, basis.get_column(0) * gs);
                    basis.set_column(1, basis.get_column(1) * gs);
                    basis.set_column(2, basis.get_column(2) * gs);
                }
                skin_vertex_transform = godot::Transform3D(basis, origin);
            }

            std::vector<NiNodeRef> bones = skin->GetBones();
            unsigned int bone_count = skin_data->GetBoneCount();

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

            // Godot GPU skinning supports exactly 4 influences per vertex.
            // Sort descending, truncate to 4, pad with zeros, re-normalize to sum=1.
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

            // Diagnostic: count vertices with non-zero weights from NiSkinData
            unsigned int verts_with_weights = 0;
            for (const auto& vw2 : vertex_weights) {
                for (const auto& p : vw2) {
                    if (p.second > 0.0001f) { ++verts_with_weights; break; }
                }
            }
            NiSkinPartitionRef skin_part = skin->GetSkinPartition();
            UtilityFunctions::print("[SKIN] shape=\"",
                String::utf8(nif_display_name(StaticCast<NiObject>(tri_strips)).c_str()),
                "\" bones=", bone_count,
                " verts=", (int)vertices.size(),
                " verts_with_skindata_weights=", verts_with_weights,
                " skin_partition=", skin_part != NULL ? "YES" : "NO");
        }
    }

    Ref<SurfaceTool> st;
    st.instantiate();
    st->begin(Mesh::PRIMITIVE_TRIANGLES);

    for (size_t i = 0; i < vertices.size(); ++i) {
        godot::Vector3 pos = nif_to_godot_vec3(vertices[i]);
        godot::Vector3 norm;
        if (has_normals) {
            norm = nif_to_godot_vec3(normals[i]);
        }

        // For skinned meshes: bake geometry's local transform into vertices
        if (is_skinned) {
            pos = skin_vertex_transform.xform(pos);
            if (has_normals) {
                norm = skin_vertex_transform.basis.xform(norm).normalized();
            }
        }

        if (has_normals) st->set_normal(norm);
        if (has_uvs) st->set_uv(godot::Vector2(uvs[i].u, uvs[i].v));
        if (has_colors) st->set_color(godot::Color(colors[i].r, colors[i].g, colors[i].b, colors[i].a));
        if (is_skinned) {
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

    // Swap v2↔v3 for Godot CCW front-face convention (see NiTriShape comment).
    for (const auto& tri : triangles) {
        st->add_index(tri.v1);
        st->add_index(tri.v3);
        st->add_index(tri.v2);
    }

    if (!has_normals) {
        st->generate_normals();
    }

    Ref<ArrayMesh> mesh = st->commit();
    if (!mesh.is_valid()) return nullptr;

    MeshInstance3D* mesh_instance = memnew(MeshInstance3D);
    std::string name = nif_display_name(StaticCast<NiObject>(tri_strips));
    mesh_instance->set_name(String::utf8(name.c_str()));
    mesh_instance->set_mesh(mesh);

    std::vector<Niflib::Ref<NiProperty>> properties = tri_strips->GetProperties();
    Ref<Material> mat = create_material_from_properties(properties, has_colors, base_path, name);
    mesh_instance->set_surface_override_material(0, mat);

    if (is_skinned && skeleton != nullptr) {
        mesh_instance->set_skeleton_path(NodePath(".."));
        // Set explicit Skin so bind poses = inverse-rest (identity skinning at rest)
        NiNodeRef skel_root = skin->GetSkeletonRoot();
        auto skin_it = skin_cache.find(skel_root);
        if (skin_it != skin_cache.end() && skin_it->second.is_valid()) {
            mesh_instance->set_skin(skin_it->second);
        }
    } else {
        apply_nif_transform(StaticCast<NiAVObject>(tri_strips), mesh_instance);
    }

    return mesh_instance;
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
                UtilityFunctions::print("[DIAG] SKINNED NiTriShape: \"",
                    String::utf8(shape_name.c_str()),
                    "\" skel_root=\"",
                    skel_root ? String::utf8(skel_root->GetName().c_str()) : String("<null>"),
                    "\" bones=", (int)skin_bones.size(),
                    " parent_node=\"", String::utf8(name.c_str()), "\"");

                // Build or reuse Skeleton3D for this skeleton root
                Skeleton3D* skeleton = nullptr;

                auto cache_it = skeleton_cache.find(skel_root);
                if (cache_it != skeleton_cache.end()) {
                    skeleton = cache_it->second;
                    // Rebuild bone_index_map for this shape's bone set from the cached skeleton.
                    // The cached skeleton was built for a previous shape; its bone_index_map may
                    // be stale if this shape references different bone nodes.
                    bone_index_map.clear();
                    for (const auto& bone_node : skin_bones) {
                        if (bone_node == NULL) continue;
                        std::string bname = bone_node->GetName();
                        int gidx = skeleton->find_bone(String::utf8(bname.c_str()));
                        if (gidx >= 0) bone_index_map[bone_node] = gidx;
                    }
                    UtilityFunctions::print("[DIAG]   -> Using CACHED skeleton (",
                        skeleton->get_bone_count(), " bones in skeleton vs ",
                        (int)skin_bones.size(), " bones in this shape's skin)");
                } else {
                    skeleton = build_skeleton(skin, ni_node);
                    if (skeleton) {
                        skeleton_cache[skel_root] = skeleton;
                        godot_node->add_child(skeleton);
                        UtilityFunctions::print("[DIAG]   -> Built NEW skeleton under: ",
                            String::utf8(name.c_str()));
                        if (debug_show_bones) {
                            debug_visualize_skeleton(skeleton);
                        }
                    }
                }

                if (skeleton) {
                    Node3D* mesh_node = process_ni_tri_shape(tri_shape, base_path, skeleton);
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
                UtilityFunctions::print("[DIAG] STATIC NiTriShape: \"",
                    String::utf8(shape_name.c_str()), "\"");
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
                UtilityFunctions::print("[DIAG] SKINNED NiTriStrips: \"",
                    String::utf8(strip_name.c_str()),
                    "\" skel_root=\"",
                    skel_root ? String::utf8(skel_root->GetName().c_str()) : String("<null>"),
                    "\" bones=", (int)skin_bones.size(),
                    " parent_node=\"", String::utf8(name.c_str()), "\"");

                Skeleton3D* skeleton = nullptr;
                auto cache_it = skeleton_cache.find(skel_root);
                if (cache_it != skeleton_cache.end()) {
                    skeleton = cache_it->second;
                    // Rebuild bone_index_map for this shape's bone set from the cached skeleton.
                    bone_index_map.clear();
                    for (const auto& bone_node : skin_bones) {
                        if (bone_node == NULL) continue;
                        std::string bname = bone_node->GetName();
                        int gidx = skeleton->find_bone(String::utf8(bname.c_str()));
                        if (gidx >= 0) bone_index_map[bone_node] = gidx;
                    }
                    UtilityFunctions::print("[DIAG]   -> Using CACHED skeleton (",
                        skeleton->get_bone_count(), " bones in skeleton vs ",
                        (int)skin_bones.size(), " bones in this shape's skin)");
                } else {
                    skeleton = build_skeleton(skin, ni_node);
                    if (skeleton) {
                        skeleton_cache[skel_root] = skeleton;
                        godot_node->add_child(skeleton);
                        UtilityFunctions::print("[DIAG]   -> Built NEW skeleton under: ",
                            String::utf8(strip_name.c_str()));
                        if (debug_show_bones) {
                            debug_visualize_skeleton(skeleton);
                        }
                    }
                }

                if (skeleton) {
                    Node3D* mesh_node = process_ni_tri_strips(tri_strips, base_path, skeleton);
                    if (mesh_node) {
                        skeleton->add_child(mesh_node);
                    }
                } else {
                    Node3D* mesh_node = process_ni_tri_strips(tri_strips, base_path);
                    if (mesh_node) godot_node->add_child(mesh_node);
                }
            } else {
                UtilityFunctions::print("[DIAG] STATIC NiTriStrips: \"",
                    String::utf8(strip_name.c_str()), "\"");
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

