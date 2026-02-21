#include <stdint.h>
#include <stdio.h>
#include <raylib.h>
#include <string.h>
typedef struct CHIP8
{

    uint8_t memory[4096];
    uint8_t v_register[16];

    uint8_t key[16];

    uint8_t gfx[64 * 32];
    uint8_t delay_timer;
    uint8_t sound_timer;

    uint16_t opcode;
    uint16_t idx_register;
    uint16_t pc_counter;

    uint16_t stack[16];
    uint16_t stack_pointer;

} CHIP8;

CHIP8 EmulatorState = {0};
int RomSize = 0;

int LoadGameIntoMemory(const char *fileName)
{
    unsigned char *fileData = LoadFileData(fileName, &RomSize);

    if (fileData == NULL)
    {
        return 1;
    }

    //maybe check if romsize + 512 is bigger than memory.

    memcpy(EmulatorState.memory + 512, fileData, RomSize);

    UnloadFileData(fileData);

    return 0;
}