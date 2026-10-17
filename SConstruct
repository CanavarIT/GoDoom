#!/usr/bin/env python
import os
from SCons.Script import *

def normalize_path(val, env):
    return val if os.path.isabs(val) else os.path.join(env.Dir("#").abspath, val)

localEnv = Environment(tools=["default"], ENV=os.environ)

customs = ["custom.py"]
customs = [os.path.abspath(path) for path in customs]

opts = Variables(customs, ARGUMENTS)
opts.Update(localEnv)

Help(opts.GenerateHelpText(localEnv))

env = localEnv.Clone()

# godot-cpp
env = SConscript("godot-cpp/SConstruct", {"env": env, "customs": customs})

# Отдельные объектные файлы для каждой платформы/цели/архитектуры
# (например doom_node.android.template_debug.arm64.os), чтобы при смене arch
# они не перезаписывали друг друга и каждый раз не пересобиралось всё.
# Действует только на файлы, объявленные ниже; сам godot-cpp не затрагивается.
env["SHOBJSUFFIX"] = env["suffix"] + ".os"

# Хэш файла считается только если изменилось его время
Decider("MD5-timestamp")

# Sources of our extension
sources = [
    "src/register_types.cpp",
    "src/doom_node.cpp",
    "src/doomgeneric_godot.c",
    "src/sound_stubs.c",
]

# All doomgeneric sources (each file only once!)
doom_sources = [
    "thirdparty/doomgeneric/doomgeneric/am_map.c",
    "thirdparty/doomgeneric/doomgeneric/d_event.c",
    "thirdparty/doomgeneric/doomgeneric/d_items.c",
    "thirdparty/doomgeneric/doomgeneric/d_iwad.c",
    "thirdparty/doomgeneric/doomgeneric/d_loop.c",
    "thirdparty/doomgeneric/doomgeneric/d_main.c",
    "thirdparty/doomgeneric/doomgeneric/d_mode.c",
    "thirdparty/doomgeneric/doomgeneric/d_net.c",
    "thirdparty/doomgeneric/doomgeneric/doomdef.c",
    "thirdparty/doomgeneric/doomgeneric/doomgeneric.c",
    "thirdparty/doomgeneric/doomgeneric/doomstat.c",
    "thirdparty/doomgeneric/doomgeneric/dstrings.c",
    "thirdparty/doomgeneric/doomgeneric/f_finale.c",
    "thirdparty/doomgeneric/doomgeneric/f_wipe.c",
    "thirdparty/doomgeneric/doomgeneric/g_game.c",
    "thirdparty/doomgeneric/doomgeneric/hu_lib.c",
    "thirdparty/doomgeneric/doomgeneric/hu_stuff.c",
    "thirdparty/doomgeneric/doomgeneric/i_endoom.c",
    "thirdparty/doomgeneric/doomgeneric/i_joystick.c",
    "thirdparty/doomgeneric/doomgeneric/i_scale.c",
    "thirdparty/doomgeneric/doomgeneric/i_system.c",
    "thirdparty/doomgeneric/doomgeneric/i_timer.c",
    "thirdparty/doomgeneric/doomgeneric/i_video.c",
    "thirdparty/doomgeneric/doomgeneric/i_input.c",
    "thirdparty/doomgeneric/doomgeneric/icon.c",
    "thirdparty/doomgeneric/doomgeneric/info.c",
    "thirdparty/doomgeneric/doomgeneric/m_argv.c",
    "thirdparty/doomgeneric/doomgeneric/m_bbox.c",
    "thirdparty/doomgeneric/doomgeneric/m_cheat.c",
    "thirdparty/doomgeneric/doomgeneric/m_config.c",
    "thirdparty/doomgeneric/doomgeneric/m_controls.c",
    "thirdparty/doomgeneric/doomgeneric/m_fixed.c",
    "thirdparty/doomgeneric/doomgeneric/m_menu.c",
    "thirdparty/doomgeneric/doomgeneric/m_misc.c",
    "thirdparty/doomgeneric/doomgeneric/m_random.c",
    "thirdparty/doomgeneric/doomgeneric/p_ceilng.c",
    "thirdparty/doomgeneric/doomgeneric/p_doors.c",
    "thirdparty/doomgeneric/doomgeneric/p_enemy.c",
    "thirdparty/doomgeneric/doomgeneric/p_floor.c",
    "thirdparty/doomgeneric/doomgeneric/p_inter.c",
    "thirdparty/doomgeneric/doomgeneric/p_lights.c",
    "thirdparty/doomgeneric/doomgeneric/p_map.c",
    "thirdparty/doomgeneric/doomgeneric/p_maputl.c",
    "thirdparty/doomgeneric/doomgeneric/p_mobj.c",
    "thirdparty/doomgeneric/doomgeneric/p_plats.c",
    "thirdparty/doomgeneric/doomgeneric/p_pspr.c",
    "thirdparty/doomgeneric/doomgeneric/p_saveg.c",
    "thirdparty/doomgeneric/doomgeneric/p_setup.c",
    "thirdparty/doomgeneric/doomgeneric/p_sight.c",
    "thirdparty/doomgeneric/doomgeneric/p_spec.c",
    "thirdparty/doomgeneric/doomgeneric/p_switch.c",
    "thirdparty/doomgeneric/doomgeneric/p_telept.c",
    "thirdparty/doomgeneric/doomgeneric/p_tick.c",
    "thirdparty/doomgeneric/doomgeneric/p_user.c",
    "thirdparty/doomgeneric/doomgeneric/r_bsp.c",
    "thirdparty/doomgeneric/doomgeneric/r_data.c",
    "thirdparty/doomgeneric/doomgeneric/r_draw.c",
    "thirdparty/doomgeneric/doomgeneric/r_main.c",
    "thirdparty/doomgeneric/doomgeneric/r_plane.c",
    "thirdparty/doomgeneric/doomgeneric/r_segs.c",
    "thirdparty/doomgeneric/doomgeneric/r_sky.c",
    "thirdparty/doomgeneric/doomgeneric/r_things.c",
    "thirdparty/doomgeneric/doomgeneric/s_sound.c",
    "thirdparty/doomgeneric/doomgeneric/sounds.c",
    "thirdparty/doomgeneric/doomgeneric/st_lib.c",
    "thirdparty/doomgeneric/doomgeneric/st_stuff.c",
    "thirdparty/doomgeneric/doomgeneric/statdump.c",
    "thirdparty/doomgeneric/doomgeneric/tables.c",
    "thirdparty/doomgeneric/doomgeneric/v_video.c",
    "thirdparty/doomgeneric/doomgeneric/w_checksum.c",
    "thirdparty/doomgeneric/doomgeneric/w_file.c",
    "thirdparty/doomgeneric/doomgeneric/w_main.c",
    "thirdparty/doomgeneric/doomgeneric/w_wad.c",
    "thirdparty/doomgeneric/doomgeneric/wi_stuff.c",
    "thirdparty/doomgeneric/doomgeneric/z_zone.c",
    "thirdparty/doomgeneric/doomgeneric/sha1.c",
    "thirdparty/doomgeneric/doomgeneric/dummy.c",
    "thirdparty/doomgeneric/doomgeneric/w_file_stdc.c",
]

sources += doom_sources

# Include paths
env.Append(CPPPATH=[
    "src/",
    "thirdparty/doomgeneric/doomgeneric/",
])

# Important defines for doomgeneric
env.Append(CPPDEFINES=[
    "FEATURE_SOUND=0",          # disable sound for now
    "NORMALUNIX",
    "HAVE_STDINT_H",
])

# WAD, вшитый прямо в библиотеку (не зависит от экспорта Godot)
wad_file = os.path.abspath("assets/doom1.wad")
_hdr = "src/wad_path.h"
_txt = '#define DOOM_WAD_PATH "%s"\n' % wad_file
if not os.path.exists(_hdr) or open(_hdr).read() != _txt:
    open(_hdr, "w").write(_txt)
wad_obj = env.SharedObject("src/wad_embed.c")
env.Depends(wad_obj, wad_file)
sources += wad_obj

# Output library
library = env.SharedLibrary(
    "demo/addons/doom/bin/libdoom{}{}".format(env["suffix"], env["SHLIBSUFFIX"]),
    source=sources,
)

Default(library)