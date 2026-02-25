#include <raylib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define MEMORY_SIZE 4096

typedef struct CHIP8 {

    uint8_t memory[MEMORY_SIZE];
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

void FetchNextInstruction(uint8_t InstructionArr[2]) {
    if (EmulatorState.pc_counter + 1 > MEMORY_SIZE) {
        return;
    }

    uint8_t byte1 = EmulatorState.memory[EmulatorState.pc_counter];
    uint8_t byte2 = EmulatorState.memory[EmulatorState.pc_counter + 1];
    // uint16_t nextInstruction = (byte1 << 8) | byte2;

    InstructionArr[0] = byte1;
    InstructionArr[1] = byte2;

    EmulatorState.pc_counter += 2;

    return;
}

int LoadGameIntoMemory(const char* fileName) {
    unsigned char* fileData = LoadFileData(fileName, &RomSize);

    if (fileData == NULL) {
        return 1;
    }

    // maybe check if romsize + 512 is bigger than memory.

    memcpy(EmulatorState.memory + 512, fileData, RomSize);

    EmulatorState.pc_counter = 512;

    UnloadFileData(fileData);

    return 0;
}

void SimulateCycle() {
    uint8_t NextInstruction[2] = {0};
    FetchNextInstruction(NextInstruction);

    if (NextInstruction == 0) {
        return;
    }

    printf("first byte: %x, second byte: %x\n ", NextInstruction[0], NextInstruction[1]);

    // DecodeInstruction();

    // ExecuteInstruction();
}
