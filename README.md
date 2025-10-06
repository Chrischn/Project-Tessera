# Project Tessera

A prototype for an independent, open-source re-implementation of the runtime engine for the 2005 game “Sid Meier’s Civilization® IV” and its expansions “Warlords” and “Beyond the Sword”, powered by the Godot game engine.

Status: prototype / work in progress.

## Legal & Trademarks

This project is not affiliated with, sponsored, or endorsed by Take-Two Interactive Software, Inc. or Firaxis Games.
“Sid Meier’s Civilization”, “Civilization”, and related marks are trademarks or registered trademarks of Take-Two Interactive Software, Inc. in the U.S. and other countries. All trademarks are the property of their respective owners.

## No bundled game assets
No original game assets (art, audio, text, binaries, installers, or DRM files) are distributed with this project.
You must own a legitimate copy of Sid Meier’s Civilization® IV and the expansions Warlords and Beyond the Sword. On first run, the engine will prompt you to locate your Civ IV installation and will use (read-only) files from there.

## Clean-room implementation
The source code in this repository (excluding third-party code) was written from scratch, based on publicly available information and observed behavior for interoperability. No decompiled, disassembled, or leaked source code from Civilization IV or its expansions was used.

## Getting Started (Windows-only for now)

These instructions will get you a copy of the project up and running on your local machine for development and testing purposes. See deployment for notes on how to deploy the project on a live system.

### Prerequisites

• Download and install Git: https://git-scm.com/downloads/win

• Download Godot (for Windows): https://github.com/godotengine/godot/releases/download/4.4.1-stable/Godot_v4.4.1-stable_win64.exe.zip

• Download and install Visual Studio 2022 (Community): https://visualstudio.microsoft.com/de/vs/

	○ Choose "Desktop development with C++"

• Download and install Python: https://www.python.org/downloads/

	○ Add Python to PATH
	○ Via Python console: py -m pip install -U pip scons

### Installing

A step by step series of examples that tell you how to get a development env running

• Start "Git CMD":

	○ cd [YourDesiredFolder]
	○ git clone https://github.com/Chrischn/Project-Tessera.git
	○ git submodule update --init --recursive


• Go to folder Project-Tessera/external/niflib and start "build_static.bat". 


• Start Developer Command Prompt for VS 2022

	○ cd [Project-Tessera Location]
	○ scons platform=windows custom_api_file="extension_api.json" target=template_debug debug_symbols=yes debug_crt=yes


• Open project.godot with the Godot_v4.4.1-stable_win64_console.exe

• Press Play Button top-right

• Select your Civilization4.exe (classic Civ4 for now, no BTS or Warlords)


## Contributing

Please read [CONTRIBUTING.md](CONTRIBUTING.md) for details on our code of conduct, and the process for submitting pull requests to us.

## Versioning

TBD [SemVer](http://semver.org/) for versioning. For the versions available, see the [tags on this repository](https://github.com/your/project/tags). 

## Authors

* **Chrischn89** - *Initial work* - [Chrischn](https://github.com/Chrischn)

See also the list of [contributors](https://github.com/your/project/contributors) who participated in this project.

## License

• Project code: 

This program is licensed under GPL version 3.0. See [LICENSE.md](LICENSE.md) for the full text.

• Game engine: 

Godot Engine is licensed under the MIT License. See the included Godot license for details.

• Third-party components: 

See [NOTICE.md](NOTICE.md) for a list of bundled dependencies and their licenses.

## Acknowledgments

* Blake00 [CivFanatics](https://forums.civfanatics.com/threads/rebuilding-parts-of-civ4-multithreading-64bit-memory-access-to-increase-civ4s-speed-in-large-games.688441/)
* snowern [CivFanatics](https://forums.civfanatics.com/threads/mini-engine-progress.691873/)
* Your name could be here!
