#include "chip8.h"
#include "resource_dir.h"
#include <raylib.h>
#include <stddef.h>

#define WIDTH 1280
#define HEIGHT 800
#define PROJNAME "CHIP-8 Emulator"
#define RESOURCES_DIR "resources"

#define SCALE 10
#define FPS 60
#define CYCLE_MULTIPLIER (FPS / 6) // 30 * 60 =

typedef enum {
    RUN_MODE_NORMAL,
    RUN_MODE_STEP,
} RUN_MODE;

RUN_MODE CurrentRunMode = RUN_MODE_NORMAL;

void StepCycle() {
    switch (CurrentRunMode) {
        case RUN_MODE_NORMAL:
            CHIP8_SimulateCycle();
            break;
        case RUN_MODE_STEP:
            bool HasPressedStepKey = IsKeyPressed(KEY_K);
            if (HasPressedStepKey)
                CHIP8_SimulateCycle();
    }
}

void HandleInput() {
    bool pressedKeys[CHIP8_INPUTS] = {IsKeyDown(KEY_X),     IsKeyDown(KEY_ONE), IsKeyDown(KEY_TWO),
                                      IsKeyDown(KEY_THREE), IsKeyDown(KEY_Q),   IsKeyDown(KEY_W),
                                      IsKeyDown(KEY_E),     IsKeyDown(KEY_A),   IsKeyDown(KEY_S),
                                      IsKeyDown(KEY_D),     IsKeyDown(KEY_Z),   IsKeyDown(KEY_C),
                                      IsKeyDown(KEY_FOUR),  IsKeyDown(KEY_R),   IsKeyDown(KEY_F),
                                      IsKeyDown(KEY_V)

    };

    for (size_t i = 0; i < CHIP8_INPUTS; i++) {
        CHIP8_SetKey(i, pressedKeys[i]);
    }
}

void DrawScaled() {
    CHIP_8GFX gfx = CHIP8_GetGFX();

    int scaledWidth = CHIP8_SCREEN_WIDTH * SCALE;
    int scaledHeight = CHIP8_SCREEN_HEIGHT * SCALE;

    int offsetX = (WIDTH - scaledWidth) / 2;
    int offsetY = (HEIGHT - scaledHeight) / 2;

    for (int x = 0; x < CHIP8_SCREEN_WIDTH; x++) {

        for (int y = 0; y < CHIP8_SCREEN_HEIGHT; y++) {

            int screenIndex = CHIP8_Convert2DTo1D(x, y, CHIP8_SCREEN_WIDTH);

            if (gfx.data[screenIndex]) {
                DrawRectangle(offsetX + (x * SCALE), offsetY + (y * SCALE), SCALE, SCALE, WHITE);
            }
        }
    }
}

int main() {

    SetConfigFlags(FLAG_VSYNC_HINT | FLAG_WINDOW_HIGHDPI);
    InitWindow(WIDTH, HEIGHT, PROJNAME);

    InitAudioDevice();

    SetTargetFPS(FPS);
    SearchAndSetResourceDir(RESOURCES_DIR);

    Sound beep = LoadSound("beep.wav");

    int success = CHIP8_LoadGameIntoMemory("roms/tests/7-beep.ch8");

    // Failed to load rom file.
    if (success == -1) {
        return 1;
    }

    while (!WindowShouldClose()) {
        BeginDrawing();

        HandleInput();

        CHIP8_DecreaseTimers();

        uint8_t soundTimer = CHIP8_GetSoundTimer();

        if (soundTimer != 0 && !IsSoundPlaying(beep)) {
            PlaySound(beep);
        }

        for (int i = 0; i < CYCLE_MULTIPLIER; i++) {
            StepCycle();
        }

        ClearBackground(BLACK);

        DrawScaled();

        EndDrawing();
    }

    CloseWindow();
    return 0;
}
