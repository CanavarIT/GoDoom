# GoDoom

Classic **Doom** on **Godot 4** for **Android**, using a native **GDExtension** (doomgeneric + C/C++).

Touch controls included. Shareware IWAD is optional in the repo.

**Repository:** [https://github.com/CanavarIT/GoDoom](https://github.com/CanavarIT/GoDoom)

---

## Features

- Native Doom logic via [doomgeneric](https://github.com/ozkl/doomgeneric)
- Godot 4 GDExtension (`DoomNode`)
- Android **arm64** (debug / release `.so`)
- On-screen touch controls (stick, fire, use, run, strafe, weapons 1–7)
- Optional WAD embedding into the shared library at build time
- Works as an exported Android APK

---

## Requirements

- Godot **4.3+** (tested on 4.8.dev)
- Android NDK **27** or **29**
- SCons, Python
- Your own IWAD if you do not use the included shareware file:
  - `doom1.wad` (shareware) — free to redistribute
  - Full `doom.wad` / `doom2.wad` — **not** included (buy the game)

---

## Project layout

```text
GoDoom/
├── SConstruct              # Build GDExtension
├── src/                    # DoomNode, platform glue, stubs
├── thirdparty/doomgeneric/ # Engine sources
├── godot-cpp/              # Godot C++ bindings (submodule or local)
└── demo/                   # Godot project
    ├── main.gd / main.tscn
    ├── doom1.wad           # Shareware (if present)
    └── addons/doom/
        ├── doom.gdextension
        └── bin/            # Built .so files
