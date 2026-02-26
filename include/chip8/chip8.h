#pragma once

#include <stdbool.h>

#define CHIP8_MEMORY_SIZE 4096
#define CHIP8_SCREEN_WIDTH 64
#define CHIP8_SCREEN_HEIGHT 32

#define CHIP8_REGISTERS 16
#define CHIP8_INPUTS 16
#define CHIP8_STACK_SIZE 16

// C can't return arrays from functions, and I don't feel like heap allocating something that has
// fixed size ._.
typedef struct CHIP_8GFX {
    bool data[CHIP8_SCREEN_WIDTH * CHIP8_SCREEN_HEIGHT];
} CHIP_8GFX;

int Convert2DTo1D(int x, int y, int x_max);
int LoadGameIntoMemory(const char *fileName);
CHIP_8GFX GetChipGFX();
void SimulateCycle();
