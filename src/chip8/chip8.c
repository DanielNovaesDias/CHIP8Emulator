#include "chip8.h"
#include <raylib.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

typedef struct CHIP8 {

    uint8_t memory[CHIP8_MEMORY_SIZE];
    uint8_t v_register[CHIP8_REGISTERS];
    bool keys[CHIP8_INPUTS];

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

void CHIP8_SetKey(size_t key, bool active) { EmulatorState.keys[key] = active; }

int CHIP8_GetKeyPressed() {
    for (int i = 0; i < CHIP8_INPUTS; i++) {

        if (EmulatorState.keys[i]) {
            return i;
        }
    }

    return -1;
}

void CHIP8_DecreaseTimers() {
    if (EmulatorState.delay_timer > 0) {
        EmulatorState.delay_timer -= 1;
    }

    if (EmulatorState.sound_timer > 0) {
        EmulatorState.sound_timer -= 1;
    }
}

uint8_t CHIP8_GetSoundTimer() { return EmulatorState.sound_timer; }

void SkipInstruction() { EmulatorState.pc_counter += 2; }

CHIP8_INSTRUCTION FetchNextInstruction() {
    if (EmulatorState.pc_counter + 1 >= CHIP8_MEMORY_SIZE) {
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
    EmulatorState.stack_pointer -= 1;
    uint16_t stackAddress = EmulatorState.stack[EmulatorState.stack_pointer];
    JumpToNNN(stackAddress);
    EmulatorState.stack[EmulatorState.stack_pointer] = 0;
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
            uint16_t screenIndex = CHIP8_Convert2DTo1D(x, y, CHIP8_SCREEN_WIDTH);

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

void Handle8Code(uint8_t x, uint8_t y, uint8_t n_nibble) {
    switch (n_nibble) {
        case 0: {
            uint8_t Vy = GetRegister(y);
            SetRegister(x, Vy);
            break;
        }
        case 1: {
            uint8_t Vx = GetRegister(x);
            uint8_t Vy = GetRegister(y);
            SetRegister(x, Vx | Vy);
            break;
        }
        case 2: {
            uint8_t Vx = GetRegister(x);
            uint8_t Vy = GetRegister(y);
            SetRegister(x, Vx & Vy);
            break;
        }
        case 3: {
            uint8_t Vx = GetRegister(x);
            uint8_t Vy = GetRegister(y);
            SetRegister(x, Vx ^ Vy);
            break;
        }
        case 4: {
            uint8_t Vx = GetRegister(x);
            uint8_t Vy = GetRegister(y);
            uint16_t sum = Vx + Vy;
            SetRegister(x, (uint8_t)sum);
            SetRegister(15, sum > 255 ? 1 : 0);
            break;
        }
        case 5: {
            uint8_t Vx = GetRegister(x);
            uint8_t Vy = GetRegister(y);
            SetRegister(x, Vx - Vy);
            SetRegister(15, Vx >= Vy ? 1 : 0);

            break;
        }
        case 6: {
            uint8_t Vx = GetRegister(x);
            uint8_t leastSignificant = Vx & 0x01;
            SetRegister(x, Vx >> 1);
            SetRegister(15, leastSignificant);
            break;
        }
        case 7: {
            uint8_t Vx = GetRegister(x);
            uint8_t Vy = GetRegister(y);
            SetRegister(x, Vy - Vx);
            SetRegister(15, Vy >= Vx ? 1 : 0);
            break;
        }
        case 0xE: {
            uint8_t Vx = GetRegister(x);
            uint8_t mostSignificant = Vx & 0x80;
            SetRegister(x, Vx << 1);
            SetRegister(15, mostSignificant != 0);
            break;
        }
    }
}

void HandleFCode(uint8_t x, uint8_t nn_nibble) {
    switch (nn_nibble) {
        case 0x07:
            SetRegister(x, EmulatorState.delay_timer);
            break;
        case 0x0A: {
            int keyPressed = CHIP8_GetKeyPressed();
            if (keyPressed != -1) {
                SetRegister(x, keyPressed);
            } else {
                EmulatorState.pc_counter -= 2;
            }
            break;
        }
        case 0x15:
            EmulatorState.delay_timer = GetRegister(x);
            break;
        case 0x18:
            EmulatorState.sound_timer = GetRegister(x);
            break;
        case 0x1E:
            EmulatorState.idx_register += GetRegister(x);
            break;
        case 0x29:
            EmulatorState.idx_register =
                GetRegister(x) * 5; // Each font is 5 bytes so 0 x 5 = 0 < start at memory index 0
                                    // 1 * 5 = memory index 5;
            break;
        case 0x33: {
            uint8_t Vx = GetRegister(x);
            uint8_t firstDecimal = Vx / 100;
            uint8_t secondDecimal = (Vx / 10) % 10;
            uint8_t thirdDecimal = Vx % 10;
            uint16_t idx = EmulatorState.idx_register;
            EmulatorState.memory[idx] = firstDecimal;
            EmulatorState.memory[idx + 1] = secondDecimal;
            EmulatorState.memory[idx + 2] = thirdDecimal;
            break;
        }

        case 0x55: {
            // x Inclusive;
            for (size_t i = 0; i <= x; i++) {
                EmulatorState.memory[EmulatorState.idx_register + i] = GetRegister(i);
            }
            break;
        }

        case 0x65: {
            // x Inclusive;
            for (size_t i = 0; i <= x; i++) {
                SetRegister(i, EmulatorState.memory[EmulatorState.idx_register + i]);
            }
            break;
        }
    }
}

void HandleECode(uint8_t x, uint8_t nn_nibble) {
    switch (nn_nibble) {
        case 0x9E: {
            uint8_t key = GetRegister(x);

            if (EmulatorState.keys[key]) {
                SkipInstruction();
            }

            break;
        }
        case 0xA1: {
            uint8_t key = GetRegister(x);

            if (!EmulatorState.keys[key]) {
                SkipInstruction();
            }

            break;
        }
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

        case 8: {
            Handle8Code(x_nibble, y_nibble, n_nibble);
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
            break;
        case 0xC: {
            int randomValue = GetRandomValue(0, 255);
            SetRegister(x_nibble, randomValue & nn_nibble);
            break;
        }
        case 0xD:
            Draw(x_nibble, y_nibble, n_nibble);
            break;
        case 0xE:
            HandleECode(x_nibble, nn_nibble);
            break;
        case 0xF:
            HandleFCode(x_nibble, nn_nibble);
            break;
    }
}

int CHIP8_Convert2DTo1D(int x, int y, int x_max) { return y * x_max + x; }

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

CHIP_8GFX CHIP8_GetGFX() { return EmulatorState.gfx; }

int CHIP8_LoadGameIntoMemory(const char* fileName) {
    unsigned char* fileData = LoadFileData(fileName, &RomSize);

    if (fileData == NULL) {
        return -1;
    }

    // maybe check if romsize + 512 is bigger than memory.

    LoadFontDataChip8();

    memcpy(EmulatorState.memory + 512, fileData, RomSize);

    EmulatorState.pc_counter = 512;

    UnloadFileData(fileData);

    return 0;
}

void CHIP8_SimulateCycle() {
    CHIP8_INSTRUCTION NextInstruction = FetchNextInstruction();

    if (!NextInstruction.isValid) {
        return;
    }

    DecodeInstruction(NextInstruction);
}