// Реализация платформенных функций doomgeneric для Godot

#include "doomgeneric.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

// Реализована в C++ части (doom_node.cpp)
extern int doom_godot_get_key(int *pressed, unsigned char *key);

void DG_Init(void) {
    printf("DG_Init: Godot platform ready\n");
}

void DG_DrawFrame(void) {
    // Godot сам забирает DG_ScreenBuffer в _process()
}

void DG_SleepMs(uint32_t ms) {
    // Кадрами управляет Godot
    (void)ms;
}

uint32_t DG_GetTicksMs(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint32_t)(ts.tv_sec * 1000u + ts.tv_nsec / 1000000u);
}

// Оставлена для совместимости с C++ кодом, время теперь берётся из системных часов
void doom_godot_advance_time(uint32_t ms) {
    (void)ms;
}

int DG_GetKey(int *pressed, unsigned char *key) {
    return doom_godot_get_key(pressed, key);
}

void DG_SetWindowTitle(const char *title) {
    (void)title;
}