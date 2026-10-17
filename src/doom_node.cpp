#include "doom_node.h"

#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/file_access.hpp>
#include <godot_cpp/classes/project_settings.hpp>
#include <godot_cpp/variant/utility_functions.hpp>
#include <godot_cpp/core/class_db.hpp>

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cstdint>
#include <unistd.h>
#include <signal.h>
#include <unwind.h>
#include <dlfcn.h>
#include <sys/ucontext.h>

using namespace godot;

// 0 = обычная работа (для APK).
// 1 = писать stderr/stdout Doom в doom_log.txt и ловить краши (для отладки).
#define DOOM_DEBUG_LOG 0

#ifndef DOOMGENERIC_RESX
#define DOOMGENERIC_RESX 640
#endif
#ifndef DOOMGENERIC_RESY
#define DOOMGENERIC_RESY 400
#endif

// WAD, вшитый в библиотеку (src/wad_embed.c)
extern "C" {
const unsigned char *doom_wad_data(void);
size_t doom_wad_size(void);
}

// Глобальный указатель на активный DoomNode (doomgeneric single-instance)
static DoomNode *g_doom_instance = nullptr;

// Doom хранит указатель на argv (myargv) всё время работы,
// поэтому массив и строки должны жить вечно (не локальные переменные!)
static char s_arg0[] = "doom";
static char s_arg1[] = "-iwad";
static char s_wad[1024];
static char *s_argv[4];

// Записывает вшитый WAD в файл (если там ещё нет файла такого же размера)
static bool extract_embedded_wad(const char *out_path) {
    const unsigned char *data = doom_wad_data();
    size_t size = doom_wad_size();
    
    // если size == 0 — вшивка не сработала
    if (!data || size < 1000000) {
        return false;
    }
    // ... остальной код без изменений

    FILE *f = fopen(out_path, "rb");
    if (f) {
        fseek(f, 0, SEEK_END);
        long cur = ftell(f);
        fclose(f);
        if (cur == (long)size) {
            return true;
        }
    }

    f = fopen(out_path, "wb");
    if (!f) {
        return false;
    }
    size_t written = fwrite(data, 1, size, f);
    fclose(f);
    return written == size;
}

#if DOOM_DEBUG_LOG

// ---------- Перехват крашей и exit() ----------

static void log_addr(const char *tag, uintptr_t pc) {
    Dl_info info;
    if (pc && dladdr((void *)pc, &info) && info.dli_fname) {
        fprintf(stderr, "  %s pc=%p off=0x%lx %s (%s)\n", tag, (void *)pc,
                (unsigned long)(pc - (uintptr_t)info.dli_fbase), info.dli_fname,
                info.dli_sname ? info.dli_sname : "?");
    } else {
        fprintf(stderr, "  %s pc=%p\n", tag, (void *)pc);
    }
}

static _Unwind_Reason_Code unwind_cb(struct _Unwind_Context *ctx, void *arg) {
    int *n = (int *)arg;
    uintptr_t pc = _Unwind_GetIP(ctx);
    if (pc) {
        char tag[16];
        snprintf(tag, sizeof(tag), "#%02d", *n);
        log_addr(tag, pc);
        (*n)++;
    }
    return (*n >= 40) ? _URC_END_OF_STACK : _URC_NO_REASON;
}

static void log_backtrace() {
    int n = 0;
    _Unwind_Backtrace(unwind_cb, &n);
}

static struct sigaction g_old_act[32];

static void crash_handler(int sig, siginfo_t *si, void *uctx) {
    fprintf(stderr, "\n*** CRASH: signal %d (code %d) fault_addr=%p ***\n", sig, si->si_code, si->si_addr);
#if defined(__aarch64__)
    ucontext_t *uc = (ucontext_t *)uctx;
    log_addr("PC", (uintptr_t)uc->uc_mcontext.pc);
    log_addr("LR", (uintptr_t)uc->uc_mcontext.regs[30]);
#else
    (void)uctx;
#endif
    log_backtrace();
    fflush(stderr);
    // Возвращаем прежний обработчик: при повторе сигнала краш пойдёт дальше как обычно
    sigaction(sig, &g_old_act[sig], nullptr);
}

static void at_exit_log() {
    fprintf(stderr, "\n*** exit() called; call stack: ***\n");
    log_backtrace();
    fflush(stderr);
}

static void install_crash_handlers() {
    static bool installed = false;
    if (installed) {
        return;
    }
    installed = true;

    static char altstack[64 * 1024];
    stack_t ss;
    ss.ss_sp = altstack;
    ss.ss_size = sizeof(altstack);
    ss.ss_flags = 0;
    sigaltstack(&ss, nullptr);

    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_sigaction = crash_handler;
    sa.sa_flags = SA_SIGINFO | SA_ONSTACK;
    sigemptyset(&sa.sa_mask);

    const int sigs[] = { SIGSEGV, SIGBUS, SIGABRT, SIGFPE, SIGILL };
    for (int s : sigs) {
        sigaction(s, &sa, &g_old_act[s]);
    }
    atexit(at_exit_log);
}

// I_Error() в Doom пишет в stderr и вызывает exit(), на Android это не видно в Godot.
// Перенаправляем в файл: сначала в общую папку (читается из Termux),
// если нельзя (APK без прав), то в папку приложения.
static void setup_doom_log(const char *user_dir) {
    char p_user[1100];
    snprintf(p_user, sizeof(p_user), "%s/doom_log.txt", user_dir);

    const char *paths[] = {
        "/storage/emulated/0/Documents/doom_log.txt",
        "/storage/emulated/0/Download/doom_log.txt",
        p_user,
    };
    for (const char *path : paths) {
        FILE *test = fopen(path, "w");
        if (!test) {
            continue;
        }
        fclose(test);

        if (freopen(path, "w", stderr)) {
            dup2(fileno(stderr), fileno(stdout));
            setvbuf(stderr, nullptr, _IONBF, 0);
            setvbuf(stdout, nullptr, _IONBF, 0);
            fprintf(stderr, "=== doom log started: %s ===\n", path);
            install_crash_handlers();
            return;
        }
    }
}
#endif

DoomNode::DoomNode() {
    framebuffer_image.instantiate();
    framebuffer_texture.instantiate();
}

DoomNode::~DoomNode() {
    if (g_doom_instance == this) {
        g_doom_instance = nullptr;
    }
}

void DoomNode::_bind_methods() {
    ClassDB::bind_method(D_METHOD("load_wad", "path"), &DoomNode::load_wad);
    ClassDB::bind_method(D_METHOD("start"), &DoomNode::start);
    ClassDB::bind_method(D_METHOD("stop"), &DoomNode::stop);
    ClassDB::bind_method(D_METHOD("is_running"), &DoomNode::is_running);
    ClassDB::bind_method(D_METHOD("get_framebuffer_texture"), &DoomNode::get_framebuffer_texture);
    ClassDB::bind_method(D_METHOD("get_framebuffer_image"), &DoomNode::get_framebuffer_image);
    ClassDB::bind_method(D_METHOD("key_down", "key"), &DoomNode::key_down);
    ClassDB::bind_method(D_METHOD("key_up", "key"), &DoomNode::key_up);
}

void DoomNode::_ready() {
    // ничего
}

void DoomNode::_process(double delta) {
    if (!running) {
        return;
    }

    // Один тик Doom
    doomgeneric_Tick();

    if (!DG_ScreenBuffer) {
        return;
    }

    const int w = DOOMGENERIC_RESX;
    const int h = DOOMGENERIC_RESY;

    PackedByteArray data;
    data.resize(w * h * 4); // RGBA

    // Формат DG_ScreenBuffer: 32-bit XRGB (в памяти B,G,R,X) -> конвертируем в RGBA.
    // Если цвета перепутаны, поменяйте местами индексы 0 и 2.
    const uint8_t *src = reinterpret_cast<const uint8_t *>(DG_ScreenBuffer);
    uint8_t *dst = data.ptrw();

    for (int i = 0; i < w * h; i++) {
        dst[i * 4 + 0] = src[i * 4 + 2]; // R
        dst[i * 4 + 1] = src[i * 4 + 1]; // G
        dst[i * 4 + 2] = src[i * 4 + 0]; // B
        dst[i * 4 + 3] = 255;            // A
    }

    framebuffer_image->set_data(w, h, false, Image::FORMAT_RGBA8, data);

    if (framebuffer_texture->get_width() != w || framebuffer_texture->get_height() != h) {
        framebuffer_texture->set_image(framebuffer_image);
    } else {
        framebuffer_texture->update(framebuffer_image);
    }
}

void DoomNode::_exit_tree() {
    stop();
}

void DoomNode::load_wad(const String &path) {
    wad_path = path;
}

void DoomNode::start() {
    if (running) {
        return;
    }

    String real_path;
    bool ok = false;

    // 1) Пробуем user://
    String embedded = ProjectSettings::get_singleton()->globalize_path("user://doom1_embedded.wad");
    CharString emb_utf8 = embedded.utf8();
    if (extract_embedded_wad(emb_utf8.get_data())) {
        real_path = embedded;
        ok = true;
        UtilityFunctions::print("DoomNode: extracted WAD to user:// (", (int64_t)doom_wad_size(), " bytes)");
    }

    // 2) Запасной путь — почти всегда можно писать
    if (!ok) {
        const char *fallback = "/data/local/tmp/doom1_embedded.wad";
        if (extract_embedded_wad(fallback)) {
            real_path = String(fallback);
            ok = true;
            UtilityFunctions::print("DoomNode: extracted WAD to /data/local/tmp");
        }
    }

    // 3) Путь из load_wad() (если GDScript передал)
    if (!ok && !wad_path.is_empty()) {
        real_path = wad_path;
        if (real_path.begins_with("res://") || real_path.begins_with("user://")) {
            real_path = ProjectSettings::get_singleton()->globalize_path(real_path);
        }
        CharString p = real_path.utf8();
        FILE *probe = fopen(p.get_data(), "rb");
        if (probe) {
            fclose(probe);
            ok = true;
        }
    }

    if (!ok) {
        UtilityFunctions::printerr("DoomNode: cannot extract or find WAD. embedded size=", (int64_t)doom_wad_size());
        return;
    }

    CharString path_utf8 = real_path.utf8();

    // дальше как у тебя было: chdir, s_argv, doomgeneric_Create...
    String user_dir = ProjectSettings::get_singleton()->globalize_path("user://");
    CharString user_dir_utf8 = user_dir.utf8();
    chdir(user_dir_utf8.get_data());

    g_doom_instance = this;

    strncpy(s_wad, path_utf8.get_data(), sizeof(s_wad) - 1);
    s_wad[sizeof(s_wad) - 1] = '\0';

    s_argv[0] = s_arg0;
    s_argv[1] = s_arg1;
    s_argv[2] = s_wad;
    s_argv[3] = nullptr;

    doomgeneric_Create(3, s_argv);
    running = true;
}

void DoomNode::stop() {
    running = false;
    if (g_doom_instance == this) {
        g_doom_instance = nullptr;
    }
}

bool DoomNode::is_running() const {
    return running;
}

Ref<ImageTexture> DoomNode::get_framebuffer_texture() const {
    return framebuffer_texture;
}

Ref<Image> DoomNode::get_framebuffer_image() const {
    return framebuffer_image;
}

void DoomNode::key_down(int key) {
    // Простая очередь: 1 = pressed, потом код клавиши
    key_queue.push_back(1);
    key_queue.push_back(key);
}

void DoomNode::key_up(int key) {
    key_queue.push_back(0);
    key_queue.push_back(key);
}

bool DoomNode::pop_key(int *pressed, unsigned char *key) {
    if (key_queue.size() < 2) {
        return false;
    }
    *pressed = key_queue[0];
    *key = (unsigned char)key_queue[1];
    key_queue.remove_at(0);
    key_queue.remove_at(0);
    return true;
}

// ========== Глобальные функции для doomgeneric_godot.c ==========

extern "C" {

// Эти функции вызываются из doomgeneric_godot.c
int doom_godot_get_key(int *pressed, unsigned char *key) {
    if (g_doom_instance) {
        return g_doom_instance->pop_key(pressed, key) ? 1 : 0;
    }
    return 0;
}

} // extern "C"