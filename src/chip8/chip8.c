
#include "chip8.h"
#include <raylib.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

typedef struct CHIP8 {

    uint8_t memory[CHIP8_MEMORY_SIZE];
    uint8_t v_register[CHIP8_REGISTERS];

    uint8_t key[CHIP8_INPUTS];

    CHIP_8GFX gfx;
    uint8_t delay_timer;
    uint8_t sound_timer;

    // uint16_t opcode;
    uint16_t idx_register;
    uint16_t pc_counter;

    uint16_t stack[CHIP8_STACK_SIZE];
    uint16_t stack_pointer;

} CHIP8;

typedef struct CHIP8_INSTRUCTION {
    uint8_t byte1;
    uint8_t byte2;
    bool isValid;
} CHIP8_INSTRUCTION;

CHIP8 EmulatorState = {0};
int RomSize = 0;

void SkipInstruction() { EmulatorState.pc_counter += 2; }

CHIP8_INSTRUCTION FetchNextInstruction() {
    if (EmulatorState.pc_counter + 1 > CHIP8_MEMORY_SIZE) {
        return (CHIP8_INSTRUCTION){0, 0, false};
    }

    uint8_t byte1 = EmulatorState.memory[EmulatorState.pc_counter];
    uint8_t byte2 = EmulatorState.memory[EmulatorState.pc_counter + 1];
    // uint16_t nextInstruction = (byte1 << 8) | byte2;

    SkipInstruction();

    return (CHIP8_INSTRUCTION){byte1, byte2, true};
}

void JumpToNNN(uint16_t NNN) { EmulatorState.pc_counter = NNN; }

void PushToStack(uint16_t NNN) {
    EmulatorState.stack[EmulatorState.stack_pointer] = NNN;
    EmulatorState.stack_pointer += 1;
}
void PopStack() {
    uint16_t stackAddress = EmulatorState.stack[EmulatorState.stack_pointer];
    JumpToNNN(stackAddress);
    EmulatorState.stack[EmulatorState.stack_pointer] = 0;
    EmulatorState.stack_pointer -= 1;
}

void SetRegister(uint8_t x, uint8_t NN) { EmulatorState.v_register[x] = NN; };

uint8_t GetRegister(uint8_t x) { return EmulatorState.v_register[x]; }

void Draw(uint8_t X, uint8_t Y, uint8_t N) {
    uint8_t Vx = GetRegister(X);
    uint8_t Vy = GetRegister(Y);

    SetRegister(15, 0);

    for (uint8_t h = 0; h < N; h++) {
        uint8_t spriteByte = EmulatorState.memory[EmulatorState.idx_register + h];

        for (uint8_t w = 0; w < 8; w++) {
            // parsing from LSB to MSB :_:
            uint8_t spritePixel = (spriteByte >> (7 - w)) & 1;

            if (spritePixel == 0)
                continue;

            uint16_t x = (Vx + w) % CHIP8_SCREEN_WIDTH;
            uint16_t y = (Vy + h) % CHIP8_SCREEN_HEIGHT;

            // since gfx is one dimensional;
            uint16_t screenIndex = Convert2DTo1D(x, y, CHIP8_SCREEN_WIDTH);

            if (EmulatorState.gfx.data[screenIndex] == 1) {
                SetRegister(15, 1);
            }

            EmulatorState.gfx.data[screenIndex] ^= 1;
        }
    }
}

void Handle0Code(uint8_t byte) {
    switch (byte) {
        case 0xE0:
            // Clear screen;
            memset(EmulatorState.gfx.data, 0, sizeof(EmulatorState.gfx.data));
            break;
        case 0xEE:
            PopStack();
            break;
    }
}

void DecodeInstruction(CHIP8_INSTRUCTION instruction) {

    uint8_t first4Bit = instruction.byte1 >> 4;
    uint16_t second12bit = ((instruction.byte1 & 0x0F) << 8) | instruction.byte2;

    uint8_t x_nibble = instruction.byte1 & 0x0F;
    uint8_t y_nibble = instruction.byte2 >> 4;
    uint8_t n_nibble = instruction.byte2 & 0x0F;
    uint8_t nn_nibble = instruction.byte2;

    switch (first4Bit) {
        case 0:
            Handle0Code(second12bit);
            break;
        case 1:
            // Jump to subroutine at NNN;
            JumpToNNN(second12bit);
            break;
        case 2:
            // Push current pc to stack and calls new subroutine at NNN;
            PushToStack(EmulatorState.pc_counter);
            JumpToNNN(second12bit);
            break;
        case 3: {
            uint8_t VxValue = GetRegister(x_nibble);
            if (VxValue == nn_nibble) {
                SkipInstruction();
            }
            break;
        }
        case 4: {
            uint8_t VxValue = GetRegister(x_nibble);
            if (VxValue != nn_nibble) {
                SkipInstruction();
            }
            break;
        }
        case 5: {
            uint8_t VxValue = GetRegister(x_nibble);
            uint8_t VyValue = GetRegister(y_nibble);

            if (VxValue == VyValue) {
                SkipInstruction();
            }

            break;
        }
        case 6:
            SetRegister(x_nibble, nn_nibble);
            break;
        case 7: {
            uint8_t VxValue = GetRegister(x_nibble);
            SetRegister(x_nibble, VxValue + nn_nibble);
            break;
        }

        case 9: {
            uint8_t VxValue = GetRegister(x_nibble);
            uint8_t VyValue = GetRegister(y_nibble);

            if (VxValue != VyValue) {
                SkipInstruction();
            }
            break;
        }
        case 0xA:
            EmulatorState.idx_register = second12bit;
            break;
        case 0xB:
            EmulatorState.pc_counter = GetRegister(0) + second12bit;
        case 0xD:
            Draw(x_nibble, y_nibble, n_nibble);
            break;
    }
}

int Convert2DTo1D(int x, int y, int x_max) { return y * x_max + x; }

void LoadFontDataChip8() {
    uint8_t fontData[80] = {
        0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
        0x20, 0x60, 0x20, 0x20, 0x70, // 1
        0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
        0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
        0x90, 0x90, 0xF0, 0x10, 0x10, // 4
        0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
        0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
        0xF0, 0x10, 0x20, 0x40, 0x40, // 7
        0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
        0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
        0xF0, 0x90, 0xF0, 0x90, 0x90, // A
        0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
        0xF0, 0x80, 0x80, 0x80, 0xF0, // C
        0xE0, 0x90, 0x90, 0x90, 0xE0, // D
        0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
        0xF0, 0x80, 0xF0, 0x80, 0x80  // F}
    };

    memcpy(EmulatorState.memory, fontData, sizeof(fontData));
}

CHIP_8GFX GetChipGFX() { return EmulatorState.gfx; }

int LoadGameIntoMemory(const char* fileName) {
    unsigned char* fileData = LoadFileData(fileName, &RomSize);

    if (fileData == NULL) {
        return 1;
    }

    // maybe check if romsize + 512 is bigger than memory.

    LoadFontDataChip8();

    memcpy(EmulatorState.memory + 512, fileData, RomSize);

    EmulatorState.pc_counter = 512;

    UnloadFileData(fileData);

    return 0;
}

void SimulateCycle() {
    CHIP8_INSTRUCTION NextInstruction = FetchNextInstruction();

    if (!NextInstruction.isValid) {
        return;
    }

    printf("first byte: %x, second byte: %x\n ", NextInstruction.byte1, NextInstruction.byte2);

    DecodeInstruction(NextInstruction);
}