# GoDoom

Classic **Doom** on **Godot 4** for **Android**, using a native **GDExtension** (doomgeneric + C/C++).

Touch controls included. Shareware IWAD is optional in the repo.

**Repository:** https://github.com/CanavarIT/GoDoom

---

## Features

- Native Doom logic via doomgeneric
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

---

## Architecture

DoomNode (C++)
    -> doomgeneric_Create / doomgeneric_Tick
    -> doomgeneric (C)  reads WAD, renders into DG_ScreenBuffer
    -> DG_* functions (doomgeneric_godot.c)
    -> Godot pulls framebuffer -> ImageTexture

---

## Next steps

1. Fix pixel format (RGB/BGR) after the first launch.
2. Bind real time (`DG_GetTicksMs`) to `Time.get_ticks_msec()`.
3. Add touch controls for Android.
4. Sound (via `FEATURE_SOUND` + SDL or Godot AudioServer).
5. Multi-instance support (currently single-instance).

---

## License

- Wrapper code — MIT / Public Domain.
- doomgeneric and original Doom — GPLv2+.
- Commercial use requires a license for the WAD.
