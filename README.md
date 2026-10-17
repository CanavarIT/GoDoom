# GoDoom

Classic **Doom** on **Godot 4** for **Android** (and Desktop), using a native **GDExtension** (doomgeneric + C/C++).

Touch controls included. Shareware IWAD is optional in the repo.

**Repository:** https://github.com/CanavarIT/GoDoom

---

## Features

- Native Doom logic via doomgeneric
- Godot 4 GDExtension (DoomNode)
- Android arm64 (debug / release .so) + Desktop (Linux/Windows)
- On-screen touch controls: joystick (left), FIRE / USE / RUN / STRAFE (right), weapons 1-7
- Keyboard support on desktop
- Optional WAD embedding into the shared library at build time
- Works as an exported Android APK

---

## Requirements

- Godot 4.3+ (tested on 4.8.0dev7)
- Android NDK 27 or 29
- Python 3 + SCons
- C/C++ compiler (MSVC / GCC / Clang)
- Git
- Your own IWAD if you do not use the included shareware file:
  - doom1.wad (shareware) - free to redistribute
  - Full doom.wad / doom2.wad - NOT included (buy the game)

---

## Quick start

### 1. Clone dependencies

    cd doom_gdextension

    # godot-cpp (master - for Godot 4.8)
    git clone --depth 1 --branch master https://github.com/godotengine/godot-cpp.git
    cd godot-cpp
    git submodule update --init
    cd ..

    # doomgeneric
    git clone --depth 1 https://github.com/ozkl/doomgeneric.git thirdparty/doomgeneric

### 2. (Recommended) Generate API for your Godot 4.8dev7

    godot --dump-extension-api extension_api.json

Then pass it at build time: scons custom_api_file=../extension_api.json

### 3. Build for Desktop (Linux example)

    scons platform=linux target=template_debug -j$(nproc)

The library will appear in demo/addons/doom/bin/.

### 4. Build for Android (arm64)

    scons platform=android target=template_debug arch=arm64 -j$(nproc)
    scons platform=android target=template_release arch=arm64 -j$(nproc)

### 5. Run the demo

1. Open the demo folder in Godot 4.8.
2. Put doom1.wad (shareware) into demo/.
3. Run main.tscn.

---

## Project layout

    GoDoom/
    |-- SConstruct              # Build GDExtension
    |-- src/                    # DoomNode, platform glue, stubs
    |-- thirdparty/doomgeneric/ # Engine sources
    |-- godot-cpp/              # Godot C++ bindings (submodule or local)
    |-- demo/                   # Godot project
        |-- main.gd / main.tscn
        |-- doom1.wad           # Shareware (if present)
        |-- addons/doom/
            |-- doom.gdextension
            |-- bin/            # Built .so files

---

## Architecture

    DoomNode (C++)
        -> doomgeneric_Create / doomgeneric_Tick
        -> doomgeneric (C)  reads WAD, renders into DG_ScreenBuffer
        -> DG_* functions (doomgeneric_godot.c)
        -> Godot pulls framebuffer -> ImageTexture

---

## Roadmap / Next steps

1. Fix pixel format (RGB/BGR) after the first launch.
2. Bind real time (DG_GetTicksMs) to Time.get_ticks_msec().
3. Polish touch controls for Android.
4. Sound (FEATURE_SOUND + SDL or Godot AudioServer).
5. Multi-instance support (currently single-instance).

---

## License

- Wrapper code - MIT / Public Domain.
- doomgeneric and original Doom - GPLv2+.
- Commercial use requires a license for the WAD.
