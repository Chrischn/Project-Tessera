cmake_minimum_required(VERSION 3.19)
project(tessera-niflib-ext LANGUAGES CXX)

# =============================================================================
# C++ Standard
# Decision: C++17 — matches godot-cpp minimum; supported by GCC 9+, Clang 10+,
# MSVC 2019+. See docs/decisions.md.
# =============================================================================
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# =============================================================================
# godot-cpp
# GODOTCPP_CUSTOM_API_FILE must point to the extension_api.json extracted from
# the exact Godot version in use. Regenerate if Godot is updated:
#   <godot_exe> --dump-extension-api
# =============================================================================
set(GODOTCPP_CUSTOM_API_FILE "${CMAKE_SOURCE_DIR}/extension_api.json")
add_subdirectory(external/godot-cpp)

# =============================================================================
# niflib
# Builds both niflib (shared) and niflib_static (static) — both targets are
# unconditional in niflib's CMakeLists.txt. We link niflib_static only.
# Note: niflib_static sets NIFLIB_STATIC_LINK on itself but does not propagate
# it to consumers; we add it explicitly to niflib_gdext below.
#
# niflib was written for C++03/11 and is incompatible with C++17:
#   - Niflib::byte clashes with std::byte (added C++17)
#   - std::binary_function / std::mem_fun_ref were removed in C++17
# Override to C++14 for niflib targets only; our code stays at C++17.
# =============================================================================
add_subdirectory(external/niflib)
set_target_properties(niflib niflib_static PROPERTIES
    CXX_STANDARD 14
    CXX_STANDARD_REQUIRED ON
)

# =============================================================================
# GDExtension DLL: niflib_gdext
# =============================================================================
file(GLOB_RECURSE EXT_SOURCES
    "${CMAKE_SOURCE_DIR}/extensions/src/*.cpp"
)

add_library(niflib_gdext SHARED ${EXT_SOURCES})

target_link_libraries(niflib_gdext PRIVATE
    godot::cpp
    niflib_static
)

target_include_directories(niflib_gdext PRIVATE
    "${CMAKE_SOURCE_DIR}/extensions/src"
    "${CMAKE_SOURCE_DIR}/external/niflib/include"
)

target_compile_definitions(niflib_gdext PRIVATE
    NIFLIB_STATIC_LINK
)

# Output filename must match what niflib.gdextension declares:
#   windows.debug.x86_64   = "res://bin/niflib.windows.template_debug.x86_64.dll"
#   windows.release.x86_64 = "res://bin/niflib.windows.template_release.x86_64.dll"
#
# SUFFIX with generator expressions is evaluated too late for MSBuild path
# normalisation — use RUNTIME_OUTPUT_NAME_<CONFIG> instead (no genex needed).
# RUNTIME_OUTPUT_DIRECTORY_<CONFIG> prevents VS from appending Debug/ or Release/.
set_target_properties(niflib_gdext PROPERTIES
    PREFIX ""
    SUFFIX ".dll"
    RUNTIME_OUTPUT_NAME_DEBUG   "niflib.windows.template_debug.x86_64"
    RUNTIME_OUTPUT_NAME_RELEASE "niflib.windows.template_release.x86_64"
    RUNTIME_OUTPUT_DIRECTORY_DEBUG   "${CMAKE_SOURCE_DIR}/bin"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_SOURCE_DIR}/bin"
)
