#include "chip8.h"
#include "resource_dir.h"
#include <raylib.h>
#include <stddef.h>

#define WIDTH 1280
#define HEIGHT 800
#define PROJNAME "CHIP-8 Emulator"
#define RESOURCES_DIR "resources"

#define SCALE 10

typedef enum {
    RUN_MODE_NORMAL,
    RUN_MODE_STEP,
} RUN_MODE;

RUN_MODE CurrentRunMode = RUN_MODE_STEP;

void StepCycle() {
    switch (CurrentRunMode) {
        case RUN_MODE_NORMAL:
            SimulateCycle();
            break;
        case RUN_MODE_STEP:
            bool HasPressedStepKey = IsKeyPressed(KEY_K);
            if (HasPressedStepKey)
                SimulateCycle();
    }
}

void DrawScaled() {
    CHIP_8GFX gfx = GetChipGFX();

    int scaledWidth = CHIP8_SCREEN_WIDTH * SCALE;
    int scaledHeight = CHIP8_SCREEN_HEIGHT * SCALE;

    int offsetX = (WIDTH - scaledWidth) / 2;
    int offsetY = (HEIGHT - scaledHeight) / 2;

    for (int x = 0; x < CHIP8_SCREEN_WIDTH; x++) {

        for (int y = 0; y < CHIP8_SCREEN_HEIGHT; y++) {

            int screenIndex = Convert2DTo1D(x, y, CHIP8_SCREEN_WIDTH);

            if (gfx.data[screenIndex]) {
                DrawRectangle(offsetX + (x * SCALE), offsetY + (y * SCALE), SCALE, SCALE, WHITE);
            }
        }
    }
}

int main() {

    SetConfigFlags(FLAG_VSYNC_HINT | FLAG_WINDOW_HIGHDPI);
    InitWindow(WIDTH, HEIGHT, PROJNAME);
    SetTargetFPS(60);
    SearchAndSetResourceDir(RESOURCES_DIR);

    int success = LoadGameIntoMemory("roms/pong.rom");

    // Failed to load rom file.
    if (success == 1) {
        return 1;
    }

    while (!WindowShouldClose()) {
        BeginDrawing();

        StepCycle();

        ClearBackground(BLACK);

        DrawScaled();

        EndDrawing();
    }

    CloseWindow();
    return 0;
}
