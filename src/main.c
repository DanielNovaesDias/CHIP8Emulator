#include "chip8.h"
#include "resource_dir.h"
#include <raylib.h>
#include <stddef.h>

#define WIDTH 1280
#define HEIGHT 800
#define PROJNAME "CHIP-8 Emulator"
#define RESOURCES_DIR "resources"

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

int main() {

    SetConfigFlags(FLAG_VSYNC_HINT | FLAG_WINDOW_HIGHDPI);
    InitWindow(WIDTH, HEIGHT, PROJNAME);
    SetTargetFPS(60);
    SearchAndSetResourceDir(RESOURCES_DIR);

    int success = LoadGameIntoMemory("roms/IBMLogo.ch8");

    // Failed to load rom file.
    if (success == 1) {
        return 1;
    }

    while (!WindowShouldClose()) {
        BeginDrawing();

        StepCycle();

        ClearBackground(BLACK);

        EndDrawing();
    }

    CloseWindow();
    return 0;
}
