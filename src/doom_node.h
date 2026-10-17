#pragma once

#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/classes/image.hpp>
#include <godot_cpp/classes/image_texture.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/packed_byte_array.hpp>

using namespace godot;

// Объявления функций doomgeneric, которые мы реализуем
extern "C" {
    void doomgeneric_Create(int argc, char **argv);
    void doomgeneric_Tick();
    extern unsigned char *DG_ScreenBuffer;
    //extern int DOOMGENERIC_RESX;
    //extern int DOOMGENERIC_RESY;
}

class DoomNode : public Node {
    GDCLASS(DoomNode, Node);

private:
    Ref<Image> framebuffer_image;
    Ref<ImageTexture> framebuffer_texture;
    bool running = false;
    String wad_path;
    PackedByteArray key_queue; // простая очередь нажатий (pressed, key)

protected:
    static void _bind_methods();

public:
    DoomNode();
    ~DoomNode();

    void _ready() override;
    void _process(double delta) override;
    void _exit_tree() override;

    // Публичный API
    void load_wad(const String &path);
    void start();
    void stop();
    bool is_running() const;

    Ref<ImageTexture> get_framebuffer_texture() const;
    Ref<Image> get_framebuffer_image() const;

    // Инпут
    void key_down(int key);
    void key_up(int key);

    // Для внутренней реализации DG_GetKey
    bool pop_key(int *pressed, unsigned char *key);
};
