#include "raylib.h"

#include <stdio.h>
#include <assert.h>
#include <stdint.h>
#include <stdlib.h>

#define Kilobytes(size) (1024 * size)
#define Megabytes(size) (1024 * Kilobytes(size))
#define Gigabytes(size) (1024 * Megabytes(size))

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

#include "arena.cpp"

i32 main(void)
{
    SetTraceLogLevel(LOG_DEBUG);   

    i32 screen_width = 800;
    i32 screen_height = 450;

    u64 memory_size = Kilobytes(10);
    void *memory = malloc(memory_size);
    Arena arena;
    init_arena(&arena, memory, memory_size);

    InitWindow(screen_width, screen_height, "Title...");
    InitAudioDevice();

    SetShapesTexture({}, {});

    DisableCursor();
    SetTargetFPS(60);

    // i32 size;
    // u8* file = LoadFileData("assets/test.txt", &size);
    // printf("Got string: %s\n", file);
    // UnloadFileData(file);
    
    Shader neon_shader = LoadShader("assets/neon.vert", "assets/neon.frag");

    while (!WindowShouldClose())
    {
        ClearBackground(BLACK);


        BeginDrawing();
        BeginShaderMode(neon_shader);

        DrawRectangle(100, 100, 50, 50, BLUE);

        EndShaderMode();
        EndDrawing();
    }

    CloseAudioDevice();
    CloseWindow();

    return 0;
}
