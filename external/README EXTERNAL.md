The folder "external" holds all the 3rd-party external libraries/repositories that are used in the project.

For usage in Godot it is usually necessary to bind them to Godot via a so called "GDExtension".
For more information about how to do so read this: [https://docs.godotengine.org/en/stable/tutorials/scripting/gdextension/index.html]

Example:
The "niflib" library is a C++ library that adds functions to load .nif files. 
In order for Godot to know about this library (entry symbol), a GDExtension called "niflib.gdextension" had to be created under "extensions/". 
This GDExtension binds the (for now statically built) niflib library (/external/niflib/build_vs2022_static/Debug/niflib_static.lib) to the Godot-provided bindings found in the "extension_api.json". 
In order to effectively use the provided functions though, it is necessary to tell Godot about the functions and/or to write translation functions to convert data types to something Godot can understand. The source files for these functions are under "/extensions/src/gdext_niflib".
In the last step, the build step for the project (via Scons for now), everything gets linked together and is then ready to be used from inside the Godot-editor.