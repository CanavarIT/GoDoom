#include <stddef.h>
#include "wad_path.h"   /* создаётся SConstruct: #define DOOM_WAD_PATH "..." */

/* Вставляем doom1.wad в данные библиотеки */
__asm__(
    ".pushsection .rodata\n"
    ".balign 16\n"
    ".global doom_embedded_wad_start\n"
    ".hidden doom_embedded_wad_start\n"
    "doom_embedded_wad_start:\n"
    ".incbin \"" DOOM_WAD_PATH "\"\n"
    ".global doom_embedded_wad_end\n"
    ".hidden doom_embedded_wad_end\n"
    "doom_embedded_wad_end:\n"
    ".byte 0\n"
    ".popsection\n"
);

extern const unsigned char doom_embedded_wad_start[];
extern const unsigned char doom_embedded_wad_end[];

const unsigned char *doom_wad_data(void) {
    return doom_embedded_wad_start;
}

size_t doom_wad_size(void) {
    return (size_t)(doom_embedded_wad_end - doom_embedded_wad_start);
}