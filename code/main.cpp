#include "raylib.h"

#include <stdio.h>
#include <stdint.h>

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;
typedef float f32;
typedef double f64;

int main(void)
{
    i32 screenWidth = 800;
    i32 screenHeight = 450;

    InitWindow(screenWidth, screenHeight, "Game title goes here");

    DisableCursor();
    SetTargetFPS(60);

    i32 size;
    u8* file = LoadFileData("assets/test.txt", &size);

    printf("Got string: %s\n", file);

    UnloadFileData(file);

    while (!WindowShouldClose())
    {
        BeginDrawing();
        ClearBackground(RAYWHITE);
        EndDrawing();
    }

    CloseWindow();

    return 0;
}
