#include "raylib.h"
#include "resource_dir.h"

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

	while (!WindowShouldClose())
	{

		BeginDrawing();

		ClearBackground(BLACK);

		EndDrawing();
	}

	CloseWindow();
	return 0;
}
