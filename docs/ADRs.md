https://github.com/noboilerplate/DecisionRecord/wiki

# Architecture Decision Log

Lightweight record of key technical decisions for Project Tessera.
Format: decision → rationale → alternatives considered.

---

## ADR-001 — C++ Standard: C++17

**Status:** Accepted (2026-03-07)

**Decision:** Use C++17 as the C++ standard for all project code.

**Rationale:**
- godot-cpp requires C++17 as a minimum
- Broad compiler support: GCC 9+, Clang 10+, MSVC 2019+
- Linux contributors (typically on GCC) and Windows contributors (MSVC) both have full support
- niflib (older codebase) compiles cleanly under C++17

**Alternatives considered:**
- C++20: viable (GCC 11+, MSVC 2022+ support ~99%), but no meaningful benefit for a Godot extension project at this stage; deferred until a concrete need arises

---

## ADR-002 — Game Engine: Godot 4.5

**Status:** Accepted (2026-03-07)

**Decision:** Target Godot 4.5-stable. Do not upgrade during the build system migration.

**Rationale:**
- Stable release; godot-cpp submodule already pinned to `godot-4.5-stable`
- Godot 4.6 released concurrently — upgrade deferred to avoid simultaneous changes

**Process for future upgrades:**
1. Update godot-cpp submodule to matching tag
2. Extract new `extension_api.json` from target Godot exe: `<godot_exe> --dump-extension-api`
3. Rebuild with CMake

---

## ADR-003 — Build System: CMake (replaces SCons)

**Status:** Accepted (2026-03-07)

**Decision:** Migrate from SCons (GDExtension) + CMake (niflib, manual) to a single unified CMake build.

**Rationale:**
- Both godot-cpp and niflib have native CMake support
- Eliminates CRT mismatch: previously required `debug_crt=yes` workaround because SCons used `/MD` while CMake niflib used `/MDd`; unified CMake enforces matching flags automatically
- Single build command instead of two separate steps
- Better IDE integration with VS/VSCode
- Reference: snowern's Cv4MiniEngine (https://github.com/fortsnek9348/Cv4MiniEngine) independently chose CMake for a similar Civ4 reimplementation

**Alternatives considered:**
- Keep SCons: rejected — two build systems with implicit coupling is fragile
- Meson: not chosen — less tooling support in VS ecosystem; godot-cpp does not provide native Meson support
