The folder "external" holds all 3rd-party external libraries used in the project.

## Libraries

### niflib

C++ library for reading NetImmerse/Gamebryo `.nif`, `.kf`, and `.kfm` files. Originally from [niftools/niflib](https://github.com/niftools/niflib) (BSD-like license).

Built as a **static library** via the unified CMake build (`CMakeLists.txt` at project root). Compiled with C++14 (niflib requirement) while the rest of the project uses C++17.

#### Custom Modifications

We maintain several modifications to niflib for Civ IV support:

- **KFM 1.0 text format parser** (`src/kfm.cpp`) — the original niflib only supported KFM versions 1.2.4b and 2.0.0.0b (binary). We added parsing for the text-based KFM 1.0 format (`KEY#VALUE#` fields) used by some Civ IV assets.

- **NiBSplineCompPoint3Interpolator::SampleKeys()** (`src/obj/NiBSplineCompPoint3Interpolator.cpp`, `include/obj/NiBSplineCompPoint3Interpolator.h`) — niflib's auto-generated stub had no implementation for sampling compressed B-spline position data. We implemented `SampleKeys()` and `GetNumControlPoints()` following the existing `NiBSplineCompTransformInterpolator::SampleTranslateKeys()` pattern, cross-referenced with OpenMW's implementation for correctness.

### godot-cpp

Git submodule providing the Godot 4.5 C++ bindings for GDExtension development. Located at `godot-cpp/`.

## GDExtension Binding

To use niflib from Godot, a GDExtension called `niflib.gdextension` exists under `extensions/`. The GDExtension source files at `extensions/src/gdext_niflib/` provide translation functions that convert niflib's NIF data structures into Godot scene nodes (`Node3D`, `MeshInstance3D`, `Skeleton3D`, `Animation`, etc.).

## Build

The project uses a unified CMake build:

```bash
cd {project folder location}
cmake -B build -G "Visual Studio 17 2022" -A x64   # first-time setup
cmake --build build --config Debug
```

This builds godot-cpp, niflib (static), and the GDExtension DLL in one step. Output: `bin/niflib.windows.template_debug.x86_64.dll`.

For more on GDExtension development, see: https://docs.godotengine.org/en/stable/tutorials/scripting/gdextension/index.html
