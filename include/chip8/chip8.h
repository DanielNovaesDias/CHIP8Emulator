#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>


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

int CHIP8_Convert2DTo1D(int x, int y, int x_max);
int CHIP8_LoadGameIntoMemory(const char *fileName);
CHIP_8GFX CHIP8_GetGFX();
void CHIP8_SimulateCycle();
void CHIP8_SetKey(size_t key, bool active);
void CHIP8_DecreaseTimers();
uint8_t CHIP8_GetSoundTimer();