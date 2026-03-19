#pragma once
// =============================================================================
// gdext_niflib.hpp
// GDExtension class declaration for NIF -> Godot scene translation.
//
// GdextNiflib is a RefCounted GDExtension class. One instance is created per
// NIF load call. It holds all per-NIF state (caches, maps) that is populated
// during load and cleared at the end of load_nif_scene().
// =============================================================================

//niflib Headers
#include <niflib.h>
#include <obj/NiObject.h>
#include <obj/NiNode.h>
#include <obj/NiTriShape.h>
#include <obj/NiTriStrips.h>
#include <obj/NiTriBasedGeom.h>
#include <obj/NiTriBasedGeomData.h>
#include <obj/NiAVObject.h>
#include <obj/NiProperty.h>
#include <obj/NiSkinInstance.h>
#include <obj/NiSkinData.h>
#include <obj/NiSkinPartition.h>
#include <gen/SkinWeight.h>
#include <nif_versions.h>

//godot-cpp Headers
#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/classes/node3d.hpp>
#include <godot_cpp/classes/standard_material3d.hpp>
#include <godot_cpp/classes/image.hpp>
#include <godot_cpp/classes/image_texture.hpp>
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/skeleton3d.hpp>
#include <godot_cpp/classes/bone_attachment3d.hpp>
#include <godot_cpp/classes/skin.hpp>
#include <godot_cpp/classes/material.hpp>
#include <godot_cpp/classes/animation.hpp>
#include <godot_cpp/classes/animation_player.hpp>
#include <godot_cpp/classes/animation_library.hpp>
#include <godot_cpp/classes/shader.hpp>
#include <godot_cpp/classes/shader_material.hpp>
#include <godot_cpp/classes/resource_loader.hpp>
#include <godot_cpp/variant/color.hpp>

//Standard Library
#include <string>
#include <map>
#include <unordered_map>

namespace godot {
class GdextNiflib : public RefCounted {
    GDCLASS(GdextNiflib, RefCounted);
protected:
    static void _bind_methods();
public:
    // --- GDExtension connectivity ---
    String ping() const;

    // --- NIF version / header queries (GDScript-callable) ---
    String get_nif_version_as_string(const String& file_path) const;
    unsigned int get_nif_version_as_uint(const String& file_path) const;
    Dictionary get_nif_header_info(const String& file_path) const;
    Dictionary get_nif_header(const String& file_path) const;
    bool isValidNIFVersion(unsigned int version_code) const;

    // --- Primary entry point ---
    // Parses file_path (NIF v20.0.0.4) and builds a Godot scene under godotnode.
    // base_path is the game data root used for texture lookup.
    // All per-NIF state is cleared after this returns.
    void load_nif_scene(const String& file_path, Node3D* godotnode, const String& base_path);

    // --- Animation ---
    // Loads .kfm / .kf animation data alongside the NIF, builds an AnimationPlayer and
    // adds it as a child of root_godot_node.  No-op (prints diagnostic) if no .kfm exists.
    // Must be called after process_ni_node() so skeleton_cache is populated.
    void build_animations(const godot::String& nif_path,
                          const godot::String& base_path,
                          godot::Node3D* root_godot_node);

    // --- Scene graph traversal ---
    // Recursively maps NiNode hierarchy to Node3D hierarchy.
    void process_ni_node(Niflib::NiNodeRef ni_node, Node3D* parent_godot, const String& base_path);
    // Shared geometry builder for NiTriShape and NiTriStrips (both are NiTriBasedGeom).
    // Handles SurfaceTool build, skinning, material, and transform. Called by the two wrappers below.
    Node3D* process_tri_geometry(Niflib::NiTriBasedGeomRef geom, const String& base_path,
        Skeleton3D* skeleton = nullptr, Niflib::NiNodeRef parent_ni_node = Niflib::NiNodeRef());
    // Builds a MeshInstance3D from a NiTriShape (triangle list).
    Node3D* process_ni_tri_shape(Niflib::NiTriShapeRef tri_shape, const String& base_path,
        Skeleton3D* skeleton = nullptr, Niflib::NiNodeRef parent_ni_node = Niflib::NiNodeRef());
    // Builds a MeshInstance3D from a NiTriStrips (triangle strip, converted to triangles).
    Node3D* process_ni_tri_strips(Niflib::NiTriStripsRef tri_strips, const String& base_path,
        Skeleton3D* skeleton = nullptr, Niflib::NiNodeRef parent_ni_node = Niflib::NiNodeRef());
    // Applies NIF local transform (translation/rotation/scale) to a Godot Node3D.
    void apply_nif_transform(Niflib::NiAVObjectRef av_obj, Node3D* godot_node);
    // Builds a StandardMaterial3D from a NIF property list.
    godot::Ref<godot::Material> create_material_from_properties(
        const std::vector<Niflib::Ref<Niflib::NiProperty>>& properties,
        bool has_vertex_colors,
        const String& base_path,
        const std::string& shape_name = "");
    // Loads a DDS/BMP/TGA texture via VFS (FPK archives + disk). Cached per NIF load.
    godot::Ref<godot::ImageTexture> load_dds_texture(const String& base_path, const std::string& nif_tex_path);

    // --- Skeletal skinning ---
    // Constructs a Skeleton3D with correct rest poses and an explicit Skin resource.
    // Populates bone_index_map. See critical design notes in gdext_niflib.cpp.
    Skeleton3D* build_skeleton(Niflib::NiSkinInstanceRef skin, Niflib::NiNodeRef parent_ni_node);
    void grow_skeleton(Skeleton3D* skeleton, Niflib::NiSkinInstanceRef skin, Niflib::NiNodeRef parent_ni_node);
    // Computes bone global rest by walking parent chain of local rests.
    // Safe alternative to get_bone_global_rest() which has cache corruption issues.
    godot::Transform3D compute_bone_global_rest(Skeleton3D* skeleton, int bone_idx);
    // Converts a NIF Matrix44 (world transform) to a Godot Transform3D with coord conversion.
    godot::Transform3D nif_matrix44_to_godot(const Niflib::Matrix44& mat);

    // --- Bone attachment reparenting ---
    // After process_ni_node builds the Node3D tree, attachment nodes (weapons, props)
    // are children of static bone-mirror Node3Ds. This pass reparents them under
    // BoneAttachment3D on the Skeleton3D so they follow animated bones.
    void reparent_bone_attachments(Niflib::NiNodeRef ni_node, Node3D* root_godot_node);
    void reparent_bone_attachments_recursive(Niflib::NiNodeRef ni_node,
        Skeleton3D* skeleton, Node3D* root_godot_node);

    // --- Debug visualization ---
    // Adds colored sphere/line/label overlays for each skeleton bone (toggled by 'I' key).
    void debug_visualize_skeleton(Skeleton3D* skeleton);
    bool debug_show_bones = true;

    // --- Per-NIF state ---
    // All fields below are populated during load_nif_scene() and cleared on completion.
    // They are public because process_ni_* methods set/read them during the load.

    // Directory of the NIF file being loaded (used as texture fallback search path).
    String current_nif_dir;
    // Texture cache: avoids reloading the same DDS/BMP file multiple times within one NIF.
    std::unordered_map<std::string, godot::Ref<godot::ImageTexture>> texture_cache;
    // Maps NIF bone NiNode* -> Godot Skeleton3D bone index, rebuilt per shape processed.
    std::map<Niflib::NiNode*, int> bone_index_map;
    // Maps NIF skeleton root NiNode* -> Godot Skeleton3D. Reused across shapes that share
    // the same skeleton root (e.g. head + body meshes sharing one skeleton).
    std::map<Niflib::NiNode*, Skeleton3D*> skeleton_cache;
    // Maps NIF skeleton root NiNode* -> explicit Skin resource (inverse-rest bind poses).
    // Created once per skeleton via create_skin_from_rest_transforms(), shared across shapes.
    std::map<Niflib::NiNode*, godot::Ref<godot::Skin>> skin_cache;
    // Rotation correction: tracks per-skeleton R^{-1} and consistency across meshes.
    std::map<Niflib::NiNode*, godot::Transform3D> rotation_correction_cache;
    std::map<Niflib::NiNode*, bool> rotation_correction_consistent;
    std::map<Niflib::NiNode*, int> rotation_correction_mismatch_count;
    // Maps NIF NiNode -> the NiNode whose Godot Node3D hosts each Skeleton3D.
    std::map<Niflib::NiNode*, Niflib::NiNode*> skeleton_host_map;
    // Maps NIF NiNode -> Godot Node3D for skeleton re-parenting to common ancestor.
    std::map<Niflib::NiNode*, godot::Node3D*> ni_to_godot_node;

    // --- Team color ---
    // Applied at runtime to all meshes that carry a DARK_MAP (slot 1) mask texture.
    // Defaults to white (no tinting). Set before calling load_nif_scene().
    godot::Color team_color = godot::Color(1.0f, 1.0f, 1.0f, 1.0f);
    void set_team_color(godot::Color c) { team_color = c; }
    godot::Color get_team_color() const { return team_color; }

        // Returns NiObjectNET name if available, otherwise the NIF type name.
    static std::string nif_display_name(const Niflib::NiObjectRef& obj);

};
}
