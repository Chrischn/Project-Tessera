[![Project Tessera](/assets/logos/Project_Tessera_Logo.svg)](https://github.com/Chrischn/Project-Tessera/)
[![GitHub license](https://img.shields.io/github/license/Chrischn/Project-Tessera?style=flat-square)](https://github.com/Chrischn/Project-Tessera/LICENSE.md)
-------

A prototype for an independent, open-source implementation of a runtime engine for mods for the 2005 game “Sid Meier’s Civilization® IV” and its expansions “Warlords” and “Beyond the Sword”, powered by the Godot game engine.

Status: `prototype / work in progress`



## Legal & Trademarks

This project is not affiliated with, sponsored, or endorsed by Take-Two Interactive Software, Inc., Firaxis Games or Gamebase Co., Ltd.
“Sid Meier’s Civilization”, “Civilization”, and related marks are trademarks or registered trademarks of Take-Two Interactive Software, Inc. in the U.S. and other countries. “Gamebryo, LightSpeed” and related marks are trademarks or registered trademarks of Gamebase Co., Ltd. 
All trademarks are the property of their respective owners.

## No bundled game assets

No original game assets (art, audio, text, binaries, installers, or DRM files) are distributed with this project.
You must own a legitimate copy of Sid Meier’s Civilization® IV and the expansions Warlords and Beyond the Sword to use this project. On first run, the engine will prompt you to locate your Civ IV installation and will use (read-only) files from there.

## Clean-room implementation

The source code in this repository (excluding third-party code) was written from scratch, based on publicly available information and observed behavior for interoperability. No decompiled, disassembled, or leaked source code from Civilization IV, its expansions or the Gamebryo engine was used.

-------
## Architecture

Architecture diagrams (C4 model, PlantUML):
- [`docs/C1_Context.puml`](docs/C1_Context.puml) — System context
- [`docs/C2_Container.puml`](docs/C2_Container.puml) — Container diagram (Godot, GDExtension, TesseraHost, Relay, CvGameCoreDLL)
- [`docs/C3_Component.puml`](docs/C3_Component.puml) — GDExtension component detail
- [`docs/Data_Extraction_Architecture.puml`](docs/Data_Extraction_Architecture.puml) — Python bridge data flow

> *View these with any PlantUML renderer, VS Code PlantUML extension, or paste into [plantuml.com](https://www.plantuml.com/plantuml)*

For detailed architecture decisions, see [`docs/ADRs.md`](docs/ADRs.md) (ADR-001 through ADR-012).

-------
# The Project

### What is this all about?
I've wished for a modern Civ 4 release in 64-bit for ages, just like many of you probably did/do too. The modability of Civ 4 is unmatched in the series but the constant crashes and the long loading times when playing with some of the amazing mods out there are caused by a 32-bit technical barrier, that can only partially be worked around, without changes to the underlying engine. 

The 25th of October 2025 marks the 20th anniversary of Civ 4's original release with no official remaster in sight and that's why it's about time to take things into our own hands if we ever want to see it happen! Inspired by [Blake00](https://forums.civfanatics.com/threads/rebuilding-parts-of-civ4-multithreading-64bit-memory-access-to-increase-civ4s-speed-in-large-games.688441/)'s initial thread in the CivFanatics Forums and [snowern](https://forums.civfanatics.com/threads/mini-engine-progress.691873/)'s 64-bit Mini-Engine and despite my lack of a professional software engineering background, I decided to kick things off! 

*So to anwser the question*: I want this project to be the starting point for a community-driven effort to build a modern implementation of a runtime engine (based on Godot) that can play mods created for Civilization IV and its expansions with 64bit-support. 

We have all the mosaic tiles, we just need to put them together!

### Why is it called "Project Tessera"?

* To cite the legend himself, **Soren Johnson**, Lead Designer & AI Programmer of Civilization IV: *"Civilization is a [Tile-Based game](https://www.youtube.com/watch?v=y7AV3tNYd5g&t=2533s). That's the fundamental feature of the game."*

	Great! But what's that got to do with the word "Tessera"!?

* [Wikipedia](https://en.wikipedia.org/wiki/Tessera): *"A tessera (plural: tesserae, diminutive tessella) is an individual **tile**, usually formed in the shape of a square, used in creating a mosaic."*

	Is there a more fitting name for a working title to describe a tile-based, historic game??

* Still not convinvced? "Tessera/Tessara" means ["FOUR"](https://en.wiktionary.org/wiki/tessera-#English) in old-greek... *it's meant to be!*

### Some more inspiration:
* [Civilization IV: Prototyping](https://www.youtube.com/watch?v=QTM7TT7bOUk)

* [Play Early, Play Often: Prototyping Civilization 4 (GDC 2006)](https://www.youtube.com/watch?v=y7AV3tNYd5g)

* [Making Of: Soren Johnson On Civ 4](https://www.rockpapershotgun.com/making-of-soren-johnson-on-civ-4)

* [DESIGNER NOTES - Soren Johnson's Game Design Journal](https://www.designer-notes.com/category/civ/)

-------
## Current Status (last updated: 2026-03-22)

### NIF Pipeline (Godot GDExtension, C++17)
* NIF mesh rendering (NiTriShape, NiTriStrips) with skeletal skinning and bone attachments
* Skeletal animations (KFM/KF) with scale keyframes
* Team colors (ShaderMaterial), specular maps, debug skeleton visualization
* Scene controllers: UV scroll, flipbook, transparency, color, visibility (via AnimationPlayer)
* Lights: NiPointLight, NiDirectionalLight, NiSpotLight → Godot equivalents
* NiBillboardNode, NiLODNode, NiSwitchNode, NiStencilProperty (double-sided)
* Particle placeholder (billboard Sprite3D)

### TesseraHost Bridge (C++17 + VS2003 relay)
* Loads **unmodified** 32-bit CvGameCoreDLL.dll — vanilla BTS and total conversion mods (tested with Rise of Mankind, Caveman2Cosmos)
* XML config loading: all 9 loading steps, 106 XML files parsed via pugixml
* Data extraction via embedded Python 2.4 + Boost.Python bindings (92 techs, 159 buildings, 123 units, 36 civilizations, 224 unit art definitions, and more)
* Art define lookups: game type → NIF/KFM asset path (e.g., `ART_DEF_UNIT_WARRIOR` → `Art/Units/Warrior/Warrior.nif`)
* TCP protocol between Godot (64-bit) and TesseraHost (32-bit) via length-prefixed JSON
* Linux support: TesseraHost runs under Wine, TCP bridge works across Wine↔native

### Core Infrastructure
* Exe verification (SHA256 hash check of Civ4BeyondSword.exe)
* FPK archive loading (ROT-1 + XOR signature)
* Virtual File System with asset override priority (mod → BTS → Warlords → base)
* Main menu with font loading
* Build system: unified CMake (godot-cpp + niflib + GDExtension)

### Next Up
* Terrain/landscape rendering
* GUI themes
* Python mod script execution (pybind11 integration)
* Save/load, multiplayer


-------
## Getting Started (Windows-only)(for developers)

These instructions will get you a copy of the project up and running on your local machine for development and testing purposes.

### Prerequisites

* Download and install Git: https://git-scm.com/downloads/win

* Download Godot (for Windows): https://github.com/godotengine/godot/releases/download/4.5-stable/Godot_v4.5-stable_win64.exe.zip

* Download and install Visual Studio 2022 (Community): https://visualstudio.microsoft.com/de/vs/

	* During installation - choose `Desktop development with C++` from workloads

* Download and install Python: https://www.python.org/downloads/

	* During installation - enable the `Add Python to PATH` option

* Download and install CMake: https://cmake.org/download/

### Installing

A step by step set of instructions that tell you how to get a development env running:

* (For initializing the Git repos) Start "Git CMD":

	* `cd [YourDesiredFolder]`
	* `git clone https://github.com/Chrischn/Project-Tessera.git`
	* `git submodule update --init --recursive`


* (For building the GDExtension — NIF pipeline) Start Developer Command Prompt for VS 2022:

	* `cd [Project-Tessera Location]`
	* `cmake -B build -G "Visual Studio 17 2022" -A x64`  *(configure — run once, or after CMakeLists.txt changes)*
	* `cmake --build build --config Debug`

* (For building TesseraHost — game logic bridge) Same command prompt:

	* `cd [Project-Tessera Location]\host`
	* `cmake -B build -G "Visual Studio 17 2022" -A Win32`  *(note: Win32, not x64)*
	* `cmake --build build --config Release`  *(note: Release, not Debug — required for CRT compatibility)*

* The TesseraRelay.dll is pre-built and included in the repo. No action needed unless you modify `host/relay/src/`.

* Open project.godot with "Godot_v4.5-stable_win64_console.exe"

* Press Play Button top-right

* Select your **Civ4BeyondSword.exe** once prompted (only required on the first start)

* The init screen will spawn TesseraHost, load XML data (~40 seconds), then proceed to the main menu

-------
## Contributing

Please read [CONTRIBUTING.md](CONTRIBUTING.md) for details on submitting pull requests, and [STYLE_GUIDE.md](STYLE_GUIDE.md) for coding conventions.

## Versioning

This project uses [SemVer](http://semver.org/) for versioning. Current version: **0.1.0** (prototype). For the versions available, see the [tags on this repository](https://github.com/Chrischn/Project-Tessera/tags).

-------
## Authors

* **Chrischn89** - *Initial work* - [GitHub page](https://github.com/Chrischn)

*Your name could be here!*

See also the list of [contributors](https://github.com/Chrischn/Project-Tessera/graphs/contributors) who participated in this project.

-------
## License

* Project code: 

	* This program is licensed under GPL version 3.0. See [LICENSE.md](LICENSE.md) for the full text.

* Game engine: 

	* Godot Engine is licensed under the MIT License. See the included Godot license for details.

* Third-party components: 

	* See [NOTICE.md](NOTICE.md) for a list of bundled dependencies and their licenses.

-------
## Acknowledgments
A big THANK YOU to all these amazing people, who inspired all of this:

* Soren Johnson - [Mohawk Games](https://mohawkgames.com/podcasts/interview-with-soren-johnson-design-director/) - [MobyGames page](https://www.mobygames.com/person/50568/soren-johnson/)
* Dorian Newcomb - [Redacted Initiative](https://www.redactedinitiative.com/) - [MobyGames page](https://www.mobygames.com/person/50573/dorian-newcomb/)
* Dale Kent - [Mohawk Games](https://mohawkgames.com/podcasts/interview-with-dale-kent-daniels-umanovskis/)
* Blake00 - [CivFanatics page](https://forums.civfanatics.com/members/blake00.284327/)
* snowern - [CivFanatics page](https://forums.civfanatics.com/members/snowern.304450/)
* Nightinggale - [CivFanatics page](https://forums.civfanatics.com/members/nightinggale.158038/)
