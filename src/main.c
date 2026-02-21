#include <raylib.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include "resource_dir.h"
#include "chip8.h"

#define WIDTH 1280
#define HEIGHT 800
#define PROJNAME "CHIP-8 Emulator"
#define RESOURCES_DIR "resources"

int main()
{

	SetConfigFlags(FLAG_VSYNC_HINT | FLAG_WINDOW_HIGHDPI);
	InitWindow(WIDTH, HEIGHT, PROJNAME);
	SetTargetFPS(75);
	SearchAndSetResourceDir(RESOURCES_DIR);

	int success = LoadGameIntoMemory("test_opcode.ch8");

	//Failed to load rom file.
	if (success == 1)
	{
		return 1;
	}

	while (!WindowShouldClose())
	{
		BeginDrawing();

		ClearBackground(BLACK);

		EndDrawing();
	}

	CloseWindow();
	return 0;
}
