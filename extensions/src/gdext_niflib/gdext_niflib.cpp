//GDExtension Header
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
//Standard Library
#include <vector>
#include <algorithm>
#include <exception>


using namespace godot;
using namespace Niflib;

// --- Coordinate conversion helpers ---
// NIF (Civ IV): X-right, Z-up, Y-forward (left-handed)
// Godot:        X-right, Y-up, Z-backward (right-handed)
// Swap Y<->Z and negate new Z to convert.
static godot::Vector3 nif_to_godot_vec3(const Niflib::Vector3& v) {
    return godot::Vector3(v.x, v.z, -v.y);
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
godot::Ref<StandardMaterial3D> GdextNiflib::create_material_from_properties(
    const std::vector<Niflib::Ref<NiProperty>>& properties,
    bool has_vertex_colors,
    const String& base_path)
{
    godot::Ref<StandardMaterial3D> mat;
    mat.instantiate();

    NiMaterialPropertyRef mat_prop;
    NiAlphaPropertyRef alpha_prop;
    NiTexturingPropertyRef tex_prop;

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
                    UtilityFunctions::print("[TEX] Skipping TeamColor BASE_MAP, using DECAL_0 instead");
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
                mat->set_texture(StandardMaterial3D::TEXTURE_ALBEDO, tex);
                godot::Color current = mat->get_albedo();
                mat->set_albedo(godot::Color(1.0f, 1.0f, 1.0f, current.a));
                UtilityFunctions::print("[TEX] Albedo from slot ", slot, ": ",
                    String::utf8(albedo_desc.source->GetTextureFileName().c_str()));
                albedo_loaded = true;
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

        UtilityFunctions::print("[ALPHA] blend=", blend, " test=", test,
            " src=", src_blend, " dst=", dst_blend, " threshold=", threshold);

        // threshold=0 with blend is typically env map blending in Civ IV, not real transparency
        if (threshold > 0) {
            if (blend && test) {
                mat->set_transparency(StandardMaterial3D::TRANSPARENCY_ALPHA_SCISSOR);
                mat->set_alpha_scissor_threshold(threshold / 255.0f);
            } else if (test) {
                mat->set_transparency(StandardMaterial3D::TRANSPARENCY_ALPHA_SCISSOR);
                mat->set_alpha_scissor_threshold(threshold / 255.0f);
            } else if (blend) {
                mat->set_transparency(StandardMaterial3D::TRANSPARENCY_ALPHA);
            }
        }
    }

    // Default cull mode (back-face culling enabled)
    mat->set_cull_mode(StandardMaterial3D::CULL_BACK);

    return mat;
}

// Processes a NiTriShape: builds a MeshInstance3D with geometry + material.
// Phases 1-3: geometry (vertices, normals, UVs, vertex colors) + material colors.
Node3D* GdextNiflib::process_ni_tri_shape(NiTriShapeRef tri_shape, const String& base_path) {
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

    // Phase 2: UV coordinates
    int uv_count = data->GetUVSetCount();
    std::vector<Niflib::TexCoord> uvs;
    if (uv_count > 0) {
        uvs = data->GetUVSet(0);
    }
    bool has_uvs = !uvs.empty() && (uvs.size() == vertices.size());

    // Phase 2: Vertex colors
    std::vector<Niflib::Color4> colors = data->GetColors();
    bool has_colors = !colors.empty() && (colors.size() == vertices.size());

    // Build mesh via SurfaceTool
    Ref<SurfaceTool> st;
    st.instantiate();
    st->begin(Mesh::PRIMITIVE_TRIANGLES);

    // Per-vertex attributes (order matters: set attributes BEFORE add_vertex)
    for (size_t i = 0; i < vertices.size(); ++i) {
        if (has_normals) {
            st->set_normal(nif_to_godot_vec3(normals[i]));
        }
        if (has_uvs) {
            st->set_uv(godot::Vector2(uvs[i].u, uvs[i].v));
        }
        if (has_colors) {
            st->set_color(godot::Color(colors[i].r, colors[i].g, colors[i].b, colors[i].a));
        }
        st->add_vertex(nif_to_godot_vec3(vertices[i]));
    }

    // Triangle indices (swap v2/v3 to fix winding order after coordinate handedness change)
    for (const auto& tri : triangles) {
        st->add_index(tri.v1);
        st->add_index(tri.v3);
        st->add_index(tri.v2);
    }

    // Generate normals only if the NIF didn't provide them
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

    // Phase 3: Material from NIF properties
    std::vector<Niflib::Ref<NiProperty>> properties = tri_shape->GetProperties();
    Ref<StandardMaterial3D> mat = create_material_from_properties(properties, has_colors, base_path);
    mesh_instance->set_surface_override_material(0, mat);

    // Apply transform from the NiTriShape node
    apply_nif_transform(StaticCast<NiAVObject>(tri_shape), mesh_instance);

    return mesh_instance;
}

// Processes a NiTriStrips: builds a MeshInstance3D with geometry + material.
// NiTriStripsData::GetTriangles() converts strips to triangles, so mesh building is identical to NiTriShape.
Node3D* GdextNiflib::process_ni_tri_strips(NiTriStripsRef tri_strips, const String& base_path) {
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

    Ref<SurfaceTool> st;
    st.instantiate();
    st->begin(Mesh::PRIMITIVE_TRIANGLES);

    for (size_t i = 0; i < vertices.size(); ++i) {
        if (has_normals) st->set_normal(nif_to_godot_vec3(normals[i]));
        if (has_uvs) st->set_uv(godot::Vector2(uvs[i].u, uvs[i].v));
        if (has_colors) st->set_color(godot::Color(colors[i].r, colors[i].g, colors[i].b, colors[i].a));
        st->add_vertex(nif_to_godot_vec3(vertices[i]));
    }

    for (const auto& tri : triangles) {
        st->add_index(tri.v1);
        st->add_index(tri.v3);
        st->add_index(tri.v2);
    }

    if (!has_normals) st->generate_normals();

    Ref<ArrayMesh> mesh = st->commit();
    if (!mesh.is_valid()) return nullptr;

    MeshInstance3D* mesh_instance = memnew(MeshInstance3D);
    std::string name = nif_display_name(StaticCast<NiObject>(tri_strips));
    mesh_instance->set_name(String::utf8(name.c_str()));
    mesh_instance->set_mesh(mesh);

    std::vector<Niflib::Ref<NiProperty>> properties = tri_strips->GetProperties();
    Ref<StandardMaterial3D> mat = create_material_from_properties(properties, has_colors, base_path);
    mesh_instance->set_surface_override_material(0, mat);

    apply_nif_transform(StaticCast<NiAVObject>(tri_strips), mesh_instance);

    return mesh_instance;
}

// Recursively processes the NIF scene graph starting from an NiNode.
// Uses GetChildren() (scene graph only) instead of GetRefs() (all references).
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
            Node3D* mesh_node = process_ni_tri_shape(tri_shape, base_path);
            if (mesh_node) {
                godot_node->add_child(mesh_node);
            }
        }
        else if (NiTriStripsRef tri_strips = DynamicCast<NiTriStrips>(child)) {
            Node3D* mesh_node = process_ni_tri_strips(tri_strips, base_path);
            if (mesh_node) {
                godot_node->add_child(mesh_node);
            }
        }
        else {
            UtilityFunctions::print("Skipping unhandled NIF type: ",
                String::utf8(child->GetType().GetTypeName().c_str()));
        }
    }
}

