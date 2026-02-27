#include "chip8.h"
#include "resource_dir.h"
#include <raylib.h>
#include <stddef.h>
#include <stdlib.h>

#define RAYGUI_IMPLEMENTATION
#include "raygui.h"

#undef RAYGUI_IMPLEMENTATION

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

typedef struct ButtonStates {
    bool loadFilePressed;
    bool romPickerOpen;
    char* selectedFilePath;
} ButtonStates;

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

void buildRomPicker(ButtonStates* state) {
    float PanelWidth = 600;
    float PanelHeight = 300;

    float PanelAnchorX = (WIDTH - PanelWidth) / 2;
    float PanelAnchorY = (HEIGHT - PanelHeight) / 2;

    GuiPanel((Rectangle){PanelAnchorX, PanelAnchorY, PanelWidth, PanelHeight}, "Pick a ROM");

    FilePathList ROMS =
        LoadDirectoryFilesEx(TextFormat("%s/roms", GetWorkingDirectory()), "FILES*", false);

    int* buttons = (int*)malloc(sizeof(int) * ROMS.count);

    for (int i = 0; i < ROMS.count; i++) {
        buttons[i] =
            GuiButton((Rectangle){PanelAnchorX + 10, PanelAnchorY + (30 * (i + 1)), 100, 20},
                      GetFileName(ROMS.paths[i]));
    }

    for (int i = 0; i < ROMS.count; i++) {
        if (buttons[i]) {
            state->selectedFilePath = ROMS.paths[i];
        }
    }

    free(buttons);
}

void buildUI(ButtonStates* state) {
    state->loadFilePressed = GuiButton((Rectangle){12, 8, 24, 24}, "#8#");

    if (state->romPickerOpen) {
        buildRomPicker(state);
    }
}

void handleUI(ButtonStates* state) {
    if (state->loadFilePressed) {
        state->romPickerOpen = true;
    }

    if (state->selectedFilePath != NULL) {
        state->romPickerOpen = false;

        CHIP8_LoadGameIntoMemory(state->selectedFilePath);

        state->selectedFilePath = NULL;
    }
}

int main() {

    SetConfigFlags(FLAG_VSYNC_HINT | FLAG_WINDOW_HIGHDPI);
    InitWindow(WIDTH, HEIGHT, PROJNAME);

    InitAudioDevice();

    SetTargetFPS(FPS);
    SearchAndSetResourceDir(RESOURCES_DIR);

    Sound beep = LoadSound("beep.wav");

    ButtonStates state = {0};

    bool isGameLoaded = false;

    int success = CHIP8_LoadGameIntoMemory("roms/tests/1-chip8-logo.ch8");

    // Failed to load rom file.
    if (success == -1) {
        return 1;
    } else {
        isGameLoaded = true;
    }

    while (!WindowShouldClose()) {
        BeginDrawing();

        handleUI(&state);

        if (isGameLoaded) {
            HandleInput();

            CHIP8_DecreaseTimers();

            uint8_t soundTimer = CHIP8_GetSoundTimer();

            if (soundTimer != 0 && !IsSoundPlaying(beep)) {
                PlaySound(beep);
            }

            for (int i = 0; i < CYCLE_MULTIPLIER; i++) {
                StepCycle();
            }
        }

        ClearBackground(BLACK);

        DrawScaled();

        buildUI(&state);

        EndDrawing();
    }

    CloseWindow();
    return 0;
}
