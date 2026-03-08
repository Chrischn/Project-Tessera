#pragma once

//niflib Headers
#include <niflib.h>
#include <obj/NiObject.h>
#include <obj/NiNode.h>
#include <obj/NiTriShape.h>
#include <obj/NiTriStrips.h>
#include <obj/NiAVObject.h>
#include <obj/NiProperty.h>
#include <nif_versions.h>

//godot-cpp Headers
#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/classes/node3d.hpp>
#include <godot_cpp/classes/standard_material3d.hpp>
#include <godot_cpp/classes/image.hpp>
#include <godot_cpp/classes/image_texture.hpp>
#include <godot_cpp/classes/engine.hpp>

//Standard Library
#include <string>
#include <unordered_map>

namespace godot {
class GdextNiflib : public RefCounted {
    GDCLASS(GdextNiflib, RefCounted);
protected:
    static void _bind_methods();
public:
    //GDExtension functionality
    String ping() const;

    // NIF loading functionality
    String get_nif_version_as_string(const String& file_path) const;
    unsigned int get_nif_version_as_uint(const String& file_path) const;
    Dictionary get_nif_header_info(const String& file_path) const;
    Dictionary get_nif_header(const String& file_path) const;
    bool isValidNIFVersion(unsigned int version_code) const;
    void load_nif_scene(const String& file_path, Node3D* godotnode, const String& base_path);

    // Scene graph traversal and mesh building
    void process_ni_node(Niflib::NiNodeRef ni_node, Node3D* parent_godot, const String& base_path);
    Node3D* process_ni_tri_shape(Niflib::NiTriShapeRef tri_shape, const String& base_path);
    Node3D* process_ni_tri_strips(Niflib::NiTriStripsRef tri_strips, const String& base_path);
    void apply_nif_transform(Niflib::NiAVObjectRef av_obj, Node3D* godot_node);
    godot::Ref<godot::StandardMaterial3D> create_material_from_properties(
        const std::vector<Niflib::Ref<Niflib::NiProperty>>& properties,
        bool has_vertex_colors,
        const String& base_path);
    godot::Ref<godot::ImageTexture> load_dds_texture(const String& base_path, const std::string& nif_tex_path);

    // Per-NIF state (set in load_nif_scene, cleared after)
    String current_nif_dir;
    std::unordered_map<std::string, godot::Ref<godot::ImageTexture>> texture_cache;

    static std::string nif_display_name(const Niflib::NiObjectRef& obj);

};
}
