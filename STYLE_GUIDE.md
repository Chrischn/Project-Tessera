# Project Tessera — Style Guide

Coding conventions used across the project. Follow these when contributing.

## Versioning

This project uses [Semantic Versioning](http://semver.org/). Current version: **0.1.0** (prototype).

- **MAJOR:** Breaking changes to mod compatibility or save format
- **MINOR:** New features (e.g., new NIF block support, new TCP commands)
- **PATCH:** Bug fixes

## GDScript

Follow the [official Godot GDScript style guide](https://docs.godotengine.org/en/stable/tutorials/scripting/gdscript/gdscript_styleguide.html).

- **Indentation:** Tabs
- **Naming:** `snake_case` for variables/functions, `PascalCase` for classes, `UPPER_SNAKE_CASE` for constants
- **Type hints:** Use where practical (`var name: String`, `func foo() -> int:`)

### File Header (GDScript)

```gdscript
# =============================================================================
# Script Name:        filename.gd
# Author(s):          Chrischn89
# Godot Version:      4.5
# Description:
#     Brief description of what this script does.
#
# License:
#	Released under the terms of the GNU General Public License version 3.0
# =============================================================================
```

## C++ (GDExtension + TesseraHost)

- **Standard:** C++17 (see ADR-001)
- **Indentation:** Tabs
- **Braces:** K&R style (opening brace on same line for functions and control flow)
- **Naming:** `snake_case` for variables/functions, `PascalCase` for classes/types, `UPPER_SNAKE_CASE` for macros/constants
- **Includes:** Group by: standard library, third-party, project headers (separated by blank lines)

### File Header (C++)

```cpp
// =============================================================================
// File:              filename.cpp
// Author(s):         Chrischn89
// Description:
//   Brief description of what this file does.
//
// License:
//   Released under the terms of the GNU General Public License version 3.0
// =============================================================================
```

## Commit Messages

Follow [Conventional Commits](https://www.conventionalcommits.org/):

```
type(scope): short description

Longer explanation if needed.

Co-Authored-By: Name <email>
```

Types: `feat`, `fix`, `docs`, `refactor`, `chore`, `wip`
Scopes: `host`, `godot`, `relay`, `nif` (or omit for general changes)

## Architecture Decisions

Significant technical decisions are recorded in [`docs/ADRs.md`](docs/ADRs.md) using the format:
- **Decision** — what was decided
- **Rationale** — why
- **Alternatives considered** — what else was evaluated

## General Principles

- **YAGNI** — don't build what isn't needed yet
- **DRY** — don't repeat yourself
- **Clean-room** — no decompiled/disassembled code from Civ4 or Gamebryo
- **GPL 3.0** — all project code must be compatible
- **No PII** — no personal information except the author nickname "Chrischn89"
