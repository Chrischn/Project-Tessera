// =============================================================================
// File:              register_types.cpp
// Author(s):         Chrischn89
// Godot Version:     4.5
// Description:
//   GDExtension registration boilerplate for Godot module system
//
// License:
//   Released under the terms of the GNU General Public License version 3.0
// =============================================================================
#include <gdextension_interface.h>
#include <godot_cpp/godot.hpp>
#include <godot_cpp/core/class_db.hpp>
#include "gdext_niflib.hpp"

using namespace godot;

void initialize_niflib(ModuleInitializationLevel p_level) {
    if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE) return;
    //ClassDB::register_class<GdextNiflib>();
    GDREGISTER_RUNTIME_CLASS(GdextNiflib);
}

void uninitialize_niflib(ModuleInitializationLevel p_level) {
    if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE) return;
}

extern "C" {GDExtensionBool GDE_EXPORT niflib_library_init(GDExtensionInterfaceGetProcAddress p_get_proc_address, const GDExtensionClassLibraryPtr p_library, GDExtensionInitialization *r_initialization)
{
    godot::GDExtensionBinding::InitObject init_obj(p_get_proc_address, p_library, r_initialization);
    init_obj.register_initializer(initialize_niflib);
    init_obj.register_terminator(uninitialize_niflib);
    init_obj.set_minimum_library_initialization_level(MODULE_INITIALIZATION_LEVEL_SCENE);
    return init_obj.init();
}
}