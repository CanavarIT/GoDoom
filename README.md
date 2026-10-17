<<<<<<< HEAD
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
=======
# Doom GDExtension для Godot 4.8 + Android

Чистый нативный порт Doom через **doomgeneric** + **GDExtension**.

**В комплекте:**
- Виртуальный джойстик (лево)
- Кнопки FIRE / USE / RUN / STRAFE (право)
- Поддержка клавиатуры (для десктопа)

## Что нужно установить

- Godot **4.8.0dev7** (или любой 4.8+)
- Python 3 + SCons
- C/C++ компилятор (MSVC / GCC / Clang)
- Android NDK (для сборки под Android)
- Git

## Быстрый старт

### 1. Клонируем зависимости

```bash
cd doom_gdextension

# godot-cpp (master — под 4.8)
git clone --depth 1 --branch master https://github.com/godotengine/godot-cpp.git
cd godot-cpp
git submodule update --init
cd ..

# doomgeneric
git clone --depth 1 https://github.com/ozkl/doomgeneric.git thirdparty/doomgeneric
```

### 2. (Рекомендуется) Сгенерировать API под твой Godot 4.8dev7

```bash
# Запусти свой Godot
godot --dump-extension-api extension_api.json

# Потом при сборке можно указать
# scons custom_api_file=../extension_api.json
```

### 3. Сборка под Desktop (Linux пример)

```bash
scons platform=linux target=template_debug -j$(nproc)
```

Библиотека появится в `demo/addons/doom/bin/`.

### 4. Сборка под Android (arm64)

```bash
# Нужен NDK. Godot обычно использует свой, но можно указать путь.
scons platform=android target=template_debug arch=arm64 -j$(nproc)
scons platform=android target=template_release arch=arm64 -j$(nproc)
```

### 5. Запуск демо

1. Открой папку `demo` в Godot 4.8.
2. Положи `doom1.wad` (shareware) в корень `demo/`.
3. Запусти сцену `main.tscn`.

## Архитектура

```
DoomNode (C++)
    ↓ вызывает
doomgeneric_Create / doomgeneric_Tick
    ↓
doomgeneric (C)  ←  читает WAD, рендерит в DG_ScreenBuffer
    ↓
DG_* функции (doomgeneric_godot.c)
    ↓
Godot забирает framebuffer → ImageTexture
```

## Следующие шаги

1. Исправить формат пикселей (RGB/BGR) после первого запуска.
2. Привязать реальное время (`DG_GetTicksMs`) к `Time.get_ticks_msec()`.
3. Добавить touch-контролы для Android.
4. Звук (через `FEATURE_SOUND` + SDL или Godot AudioServer).
5. Поддержка нескольких инстансов (сейчас single-instance).

## Лицензия

- Код обёртки — MIT / Public Domain (как хочешь).
- doomgeneric и оригинальный Doom — GPLv2+.
- Для коммерческого использования нужна лицензия на WAD.
>>>>>>> 39f1161 (Add project README)
