<img width="2400" height="1080" alt="1000835468" src="https://github.com/user-attachments/assets/068e8728-3288-4101-af7c-dc5185cb4eb6" />
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

## Build (Android arm64)

git clone --recursive https://github.com/CanavarIT/GoDoom.git
cd GoDoom

# Set NDK (example paths — adjust to your machine)
export ANDROID_HOME=...
export ANDROID_NDK_ROOT=$ANDROID_HOME/ndk/29.0.14033849

# Debug
scons platform=android target=template_debug arch=arm64 api_version=4.3 -j$(nproc)

# Release
scons platform=android target=template_release arch=arm64 api_version=4.3 -j$(nproc)

Output:

demo/addons/doom/bin/libdoom.android.template_debug.arm64.so
demo/addons/doom/bin/libdoom.android.template_release.arm64.so

---

## Run in Godot

1. Open the `demo/` folder in Godot 4.
2. Ensure `addons/doom/doom.gdextension` points at the built libraries.
3. Put `doom1.wad` in `demo/` if it is not already there (shareware or your own IWAD).
4. Run the main scene, or **Project → Export → Android**.

### `doom.gdextension` (example)

[configuration]

entry_symbol = "doom_library_init"
compatibility_minimum = "4.3"
reloadable = false

[libraries]

android.debug.arm64 = "res://addons/doom/bin/libdoom.android.template_debug.arm64.so"
android.release.arm64 = "res://addons/doom/bin/libdoom.android.template_release.arm64.so"

---

## Controls

| Action | Control |
|--------|---------|
| Move | Virtual stick |
| Fire | FIRE |
| Use / open | USE |
| Run | RUN (toggle) |
| Strafe | STRAFE |
| Weapons | 1–7 |
| Menu | MENU / OK / MAP |

Keyboard (desktop): WASD / arrows, Ctrl = fire, Space = use, Shift = run.

---

## Notes

- Sound is stubbed (`FEATURE_SOUND=0`) for a simpler Android build.
- If the WAD is embedded at compile time, the library size is ~5 MB and no external WAD is required at runtime.
- On Android, export **Debug** or ship both debug and release `.so` files so the loader always finds a library.

---

## License

- **This project** (wrapper, demo scripts): GPLv2
- **doomgeneric**: GPLv2 (see `thirdparty/doomgeneric/`)
- **godot-cpp**: MIT
- **Doom shareware IWAD** (`doom1.wad`): © id Software — free shareware data only

Full commercial Doom / Doom II data are **not** distributed. Buy the games and use your own IWAD for the full content.

Doom® is a registered trademark of id Software LLC.
