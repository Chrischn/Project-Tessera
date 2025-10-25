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
# The Project

### What is this all about?
I've wished for a modern Civ 4 release in 64-bit for ages, just like many of you probably did/do too. The moddability of Civ 4 is unmatched in the series but the constant crashes and the long loading times when playing with some of the amazing mods out there are caused by a 32-bit technical barrier, that can only partially be worked around, without changes to the underlying engine. 

The 25th of October 2025 marks the 20th anniversary of Civ 4's original release with no official remaster in sight and that's why it's about time to take things into our own hands if we ever want to see it happen! Inspired by [Blake00](https://forums.civfanatics.com/threads/rebuilding-parts-of-civ4-multithreading-64bit-memory-access-to-increase-civ4s-speed-in-large-games.688441/)'s initial thread in the CivFanatics Forums and [snowern](https://forums.civfanatics.com/threads/mini-engine-progress.691873/)'s 64-bit Mini-Engine and despite my lack of a professional software engineering background, I decided to kick things off! 

*So to answer the question*: I want this project to be the starting point for a community-driven effort to build a modern implementation of a runtime engine (based on Godot) that can play mods created for Civilization IV and its expansions with 64bit-support. 

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
## Current Status / Goals (last updated: 2025-10-25)

### Status
* Verification of the original .exe file works

* Loading of .fpk archives works

* Basic main menu is implemented

* Loading of .nif files works (only meshes are drawn in 3D for now)

Summary: In its current state it's possible to load a .nif file in 3D and see it's mesh model.

### Goals
* Rework the build pipe (Scons -> CMake)

* Add option for development on Linux

* Continue development of .nif file import function and translation to Godot nodes

* Plan out the integration of CvGameCoreDLL -> what about compatibility with saves and mods?

* Create basic landscape drawing in 3D world (Godot GridMap? https://docs.godotengine.org/en/stable/tutorials/3d/using_gridmaps.html)

* XML config loading (requires CvGameCoreDLL?)

* GUI theme config loading (requires CvGameCoreDLL?)


-------
## Getting Started (Windows-only)(for developers)

These instructions will get you a copy of the project up and running on your local machine for development and testing purposes.

### Prerequisites

* Download and install Git: https://git-scm.com/downloads/win

* Download Godot (for Windows): https://github.com/godotengine/godot/releases/download/4.5-stable/Godot_v4.5-stable_win64.exe.zip

* Download and install Visual Studio 2022 (Community): https://visualstudio.microsoft.com/de/vs/

	* During installation - choose `Desktop development with C++` from workloads

* Download and install Python: https://www.python.org/downloads/

	* During installation - add `Python to PATH option`

### Installing

A step by step set of instructions that tell you how to get a development env running:

* (For installing Scons) Start "CMD":

	* `py -m pip install -U pip scons`

* (For initializing the Git repos) Start "Git CMD":

	* `cd [YourDesiredFolder]`
	* `git clone https://github.com/Chrischn/Project-Tessera.git`
	* `git submodule update --init --recursive`


* (For manually building the static niflib library) Go to folder Project-Tessera/external/niflib and start "build_static.bat". 


* (For building the project for use in Godot Editor) Start Developer Command Prompt for VS 2022 and use these commands:

	* `cd [Project-Tessera Location]`
	* `scons platform=windows custom_api_file="extension_api.json" target=template_debug debug_symbols=yes debug_crt=yes`


* Open project.godot with "Godot_v4.5-stable_win64_console.exe"

* Press Play Button top-right

* Select your **Civ4BeyondSword.exe** once prompted (only required on the first start)

* Select "Single Player > Play Now!"

* Enjoy!

-------
## Contributing

[WORK IN PROGRESS] Please read [CONTRIBUTING.md](CONTRIBUTING.md) for details on the process for submitting pull requests to the project or how to help out.

## Versioning

[WORK IN PROGRESS] [SemVer](http://semver.org/) for versioning. For the versions available, see the [tags on this repository](https://github.com/Chrischn/Project-Tessera/tags). 

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
