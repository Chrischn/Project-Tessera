https://github.com/noboilerplate/DecisionRecord/wiki

# Architecture Decision Log

Lightweight record of key technical decisions for Project Tessera.
Format: decision → rationale → alternatives considered.

---

## ADR-001 — C++ Standard: C++17

**Status:** Accepted (2026-03-07)

**Decision:** Use C++17 as the C++ standard for all project code, including the 32-bit TesseraHost bridge process.

**Rationale:**
- godot-cpp requires C++17 as a minimum
- Broad compiler support: GCC 9+, Clang 10+, MSVC 2019+
- Linux contributors (typically on GCC) and Windows contributors (MSVC) both have full support
- niflib (older codebase) compiles cleanly under C++17
- TesseraHost (32-bit) is independent from godot-cpp but uses C++17 for consistency — all host dependencies (pugixml C++11, yyjson C99) work with C++17
- Linux contributors cross-compiling the host with MinGW-w64 have full C++17 support

**Alternatives considered:**
- C++20: viable for the host process (Glaze JSON library requires it), but rejected because Glaze requires fixed structs incompatible with mod flexibility, and C++20 adds contributor friction for no concrete benefit

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

---

## ADR-004 — XML Parser: pugixml

**Status:** Accepted (2026-03-21)

**Decision:** Use pugixml for XML parsing in the TesseraHost bridge process (backing the CvDLLXMLIFaceBase interface).

**Rationale:**
- 2-3x faster than TinyXML2 in DOM parsing benchmarks
- DOM API maps naturally to CvDLLXMLIFaceBase methods (navigate to child, read value, next sibling)
- MIT license (GPL-3.0 compatible)
- XPath support available if needed later
- Actively maintained, widely adopted

**Alternatives considered:**
- TinyXML2: what Cv4MiniEngine uses, but slower with a less ergonomic API
- RapidXML: marginally faster than pugixml, but limited write support and header-only with in-situ parsing (modifies source buffer)
- expat: SAX/event-based paradigm, not DOM — poor fit for CvDLLXMLIFaceBase which expects navigable DOM tree

---

## ADR-005 — JSON Serializer: yyjson

**Status:** Accepted (2026-03-21)

**Decision:** Use yyjson (C99) for JSON serialization in TesseraHost.

**Rationale:**
- Fastest dynamic JSON library available (~1 GB/s read and write)
- Pure C99 — zero C++ standard pressure, compiles everywhere
- MIT license (GPL-3.0 compatible)
- Dynamic document model supports arbitrary mod data without fixed struct definitions — critical for total conversion mod compatibility where mods extend info types
- Small footprint, easy to vendor

**Alternatives considered:**
- Glaze: fastest overall but requires C++20 and fixed struct reflection — incompatible with dynamic mod data
- nlohmann/json: de facto C++ standard but ~12x slower (81 MB/s); ergonomic API but performance gap is unnecessary
- RapidJSON: 4x faster than nlohmann but still ~3x slower than yyjson; more verbose API

---

## ADR-006 — TCP Socket Implementation: Raw Winsock2

**Status:** Accepted (2026-03-21)

**Decision:** Use direct Winsock2 API calls for the TesseraHost TCP server. Godot side uses built-in StreamPeerTCP.

**Rationale:**
- Localhost TCP server is ~50 lines of boilerplate code
- Zero external dependencies — Winsock2 is built into Windows
- Wine implements Winsock2 fully (critical for Linux support)
- Adding a library for socket/bind/listen/accept/send/recv is over-engineering at this stage

**Alternatives considered:**
- ASockLib, Minimal-Socket, brynet: lightweight socket wrappers, but add a dependency for trivially simple functionality
- Cross-platform socket library: not needed because the host is Windows-only (runs under Wine on Linux); Godot handles the native side

---

## ADR-007 — CvDLL*IFaceBase: Clean-Room Implementation

**Status:** Accepted (2026-03-21)

**Decision:** Write our own ABI-compatible CvDLL*IFaceBase interface definitions rather than including Firaxis SDK headers.

**Rationale:**
- Every SDK header file contains `"Copyright (c) [YEAR] Firaxis Games, Inc. All rights reserved."` with no open-source license grant — redistribution is legally unsafe
- The WINE project proves the clean-room approach works: implement the same ABI from documentation and testing rather than copying proprietary headers
- SDK at `E:\Programming\Civ4\Beyond the Sword\CvGameCoreDLL\` used as development reference only, never included in the repository
- Cv4MiniEngine referenced for understanding tricky implementation details

**Alternatives considered:**
- Include SDK headers directly: rejected due to copyright restrictions ("All rights reserved", no license grant)
- Require users to provide their own SDK: adds friction and complexity to the build process

---

## ADR-008 — IPC Serialization: JSON with Binary Upgrade Path

**Status:** Accepted (2026-03-21)

**Decision:** Use JSON for the TesseraHost ↔ Godot TCP protocol. Length-prefix framing allows future upgrade to binary.

**Rationale:**
- Human-readable — simplifies debugging during development
- Trivially parsed by Godot's built-in JSON class (no GDExtension dependency for the protocol)
- For one-time startup XML loading (~8.5 MB), even the "slow" path completes in ~100ms
- Length-prefix message framing (`[4-byte length][payload]`) supports swapping payload format without protocol changes

**Alternatives considered:**
- Binary from the start (MessagePack, raw structs): faster but harder to debug, premature optimization for a turn-based game's startup sequence
- Protobuf/gRPC: structured but heavyweight dependency, adds build complexity for minimal benefit at MVP stage

---

## ADR-009 — IPC Transport: TCP Sockets over Named Pipes / Shared Memory

**Status:** Accepted (2026-03-21)

**Decision:** Use TCP on `127.0.0.1` for communication between Godot (64-bit) and TesseraHost (32-bit).

**Rationale:**
- TCP works identically on Windows (native) and Linux (Wine process ↔ native Godot) — the architecture's cross-platform story depends on this
- Civ4 is turn-based — TCP localhost latency is negligible (kernel shortcircuits the network stack)
- Enables future possibilities: remote debugging, headless game server
- Simple to implement on both sides: Winsock2 (host), StreamPeerTCP (Godot)

**Alternatives considered:**
- Named pipes: Windows-specific API (`\\.\pipe\name`), Linux FIFOs have different semantics, Wine-to-native pipe communication is unreliable
- Shared memory: fastest IPC but significantly more complex (synchronization, memory layout contracts, cache coherence), overkill for a turn-based game
- Hybrid (pipes + shared memory): best of both but doubles the IPC surface area to maintain

---

## ADR-010 — Relay DLL for VS2003 CRT Isolation

**Status:** Accepted (2026-03-21)

**Decision:** Insert a thin relay DLL compiled with VS2003 (MSVC 7.1) between CvGameCoreDLL.dll and TesseraHost.exe.

**Rationale:**
- CvGameCoreDLL.dll (VS2003, msvcr71/msvcp71) and TesseraHost.exe (VS2022, ucrt) cannot share STL types across the CRT boundary — std::string, std::wstring, and std::vector have incompatible layouts and allocators
- The relay shares VS2003's CRT natively, so all STL operations between relay and game DLL are safe
- The relay communicates with the VS2022 host via a pure C function pointer table (HostCallbacks) — no STL types cross this boundary
- Pre-built relay binary committed to repo; only contributors modifying the relay need the free Visual C++ Toolkit 2003
- Preserves "unmodified mod DLLs" goal — the relay works with any VS2003-compiled game DLL

**Alternatives considered:**
- Raw memory manipulation of VS2003 STL types from VS2022 host: attempted, proved fragile (crashes on string heap allocation, vector push_back, cross-CRT free — confirmed empirically)
- Calling msvcp71.dll's string::assign from VS2022: partially worked for strings but didn't solve std::vector ABI issues
- Compiling the entire host with VS2003: would lose C++17, pugixml, yyjson, modern tooling
- Recompiling game DLLs for VS2022: breaks mod compatibility (the killer feature)
