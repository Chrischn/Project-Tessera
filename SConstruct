#!/usr/bin/env python
import os
import glob
import sys

EnsureSConsVersion(4, 0)

env = SConscript("external/godot-cpp/SConstruct")

# For reference:
# - CCFLAGS are compilation flags shared between C and C++
# - CFLAGS are for C-specific compilation flags
# - CXXFLAGS are for C++-specific compilation flags
# - CPPFLAGS are for pre-processor flags
# - CPPDEFINES are for pre-processor defines
# - LINKFLAGS are for linking flags

EXT_NAME = ARGUMENTS.get("gdextensionname", "niflib")
EXT_DIR = f"extensions/src/"
INPUT_DIR = f"external/{EXT_NAME}"
OUTPUT_FOLDER = "bin/"

# --- Added: small helper to build output paths consistently across platforms
def sterilized_path(rel_path: str) -> str:
    # Join with OUT_DIR and normalize to forward slashes so Godot/SCons stay happy on Windows too.
    return os.path.join(rel_path).replace("\\", "/")

OUTPUT_FOLDER = sterilized_path(OUTPUT_FOLDER)
print("OUTPUT_FOLDER: " + OUTPUT_FOLDER)


cmake_cfg  = "Debug" if env["target"] == "template_debug" else "Release"
nif_build = "external/niflib/build_vs2022_static"



# tweak this if you want to use different folders, or more folders, to store your source code in.
#env.Append(CPPPATH=["external/{}".format(EXT_NAME)])
env.Append(CPPPATH=["external/godot-cpp/gdextension/"])
env.Append(CPPPATH=["extensions/src"])
env.Append(CPPPATH=["external/niflib/include"])
sources = glob.glob(os.path.join(EXT_DIR, "**", "*.cpp"), recursive=True)

print(f"Sources found ({len(sources)}):")
for s in sources:
    print("  ", s)

if not sources:
    Exit(f"No .cpp files found in {EXT_DIR}.")



env.Append(CPPDEFINES=["NIFLIB_STATIC_LINK"])
env.Append(LIBPATH=[f"{nif_build}/{cmake_cfg}"])
env.Append(LIBS=["niflib_static.lib"])

if env["platform"] == "macos":
    library = env.SharedLibrary(
        "{}{}.{}.{}.framework/{}.{}.{}".format(OUTPUT_FOLDER, EXT_NAME, env["platform"], env["target"], EXT_NAME, env["platform"], env["target"]),
        source=sources,
    )
elif env["platform"] == "ios":
    # iOS: build a static library (.a). Simulator builds are marked by an extra ".simulator".
    if env["ios_simulator"]:
        library = env.StaticLibrary(
            "{}{}.{}.{}.simulator.a".format(OUTPUT_FOLDER, EXT_NAME, env["platform"], env["target"]),
            source=sources,
        )
    else:
        library = env.StaticLibrary(
            "{}{}.{}.{}.a".format(OUTPUT_FOLDER, EXT_NAME, env["platform"], env["target"]),
            source=sources,
        )
else:
    if env["platform"] == "windows":
        env.Append(CXXFLAGS=["/EHsc"])
    # Other platforms (Windows/Linux/Android/...): build a shared library.
    # env["suffix"] comes from godot-cpp (e.g. ".windows.template_debug.x86_64"),
    # env["SHLIBSUFFIX"] is the platform's shared-lib extension (".dll", ".so", ".dylib").
    library = env.SharedLibrary(
        "{}{}{}{}".format(OUTPUT_FOLDER, EXT_NAME, env["suffix"], env["SHLIBSUFFIX"]),
        source=sources,
    )

Default(library)
