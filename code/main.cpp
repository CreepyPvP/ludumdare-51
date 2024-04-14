#include "raylib.h"
#include "rlgl.h"
#include "raymath.h"
#include "utils.h"

#define RAYGUI_IMPLEMENTATION
#include "raygui.h"

#include <stdio.h>
#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <new>

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

#include "arena.h"

struct EntitySlot
{
    char buffer[128];
};

struct GameState
{
    Arena arena;

    u32 entity_cap;
    EntitySlot *entity_slots;
    u32 *entity_generations;
    u32 free_entity_count;
    u32 *free_entities;
};

GameState *state;

#include "entity.cpp"
#include "unit_entity.cpp"
#include "game_entity.cpp"
#include "arena.cpp"

static GameState *create_game_state(void *memory, u64 memory_size)
{
    GameState pre = {};
    InitArena(&pre.arena, memory, memory_size);

    GameState *state = PushStruct(&pre.arena, GameState);
    *state = pre;

    state->entity_cap = 100;
    state->entity_slots = PushArray(&state->arena, EntitySlot, state->entity_cap);
    state->entity_generations = PushArray(&state->arena, u32, state->entity_cap);
    state->free_entity_count = state->entity_cap;
    state->free_entities = PushArray(&state->arena, u32, state->entity_cap);
    for (u32 i = 0; i < state->entity_cap; ++i) {
        state->free_entities[i] = i;
        state->entity_generations[i] = 1;
    }

    return state;
}

i32 main(void)
{
    SetTraceLogLevel(LOG_DEBUG);

    i32 screen_width = 800;
    i32 screen_height = 450;

    u64 memory_size = Megabytes(1);
    void *memory = malloc(memory_size);

    state = create_game_state(memory, memory_size);

    Entity *root = AllocateEntity<Entity>();
    TestEntity *child_a = AllocateEntity<TestEntity>();
    Entity *child_b = AllocateEntity<Entity>();

    root->PushChild(child_b);
    child_b->local_position = {0, 0};
    child_b->PushChild(child_a);


    InitWindow(screen_width, screen_height, "Title...");
    InitAudioDevice();

    SetShapesTexture({}, {});

    DisableCursor();
    SetTargetFPS(60);

    // i32 size;
    // u8 *file = LoadFileData("assets/test.txt", &size);
    // printf("Got string: %s\n", file);
    // UnloadFileData(file);

    Shader neon_shader = LoadShader("assets/neon.vert", "assets/neon.frag");

    i32 seconds_loc = GetShaderLocation(neon_shader, "seconds");

    f32 seconds = 0;

    while (!WindowShouldClose()) {
        seconds += GetFrameTime();
        SetShaderValue(neon_shader, seconds_loc, &seconds, SHADER_UNIFORM_FLOAT);

        root->Update();
        ClearBackground(BLACK);
        BeginDrawing();

        static bool checked = true;
        GuiCheckBox((Rectangle){ 25, 108, 15, 15 }, "hello world", &checked);

        BeginShaderMode(neon_shader);

        root->Render();
        DrawRectangle(150, 170, 100, 100, BLUE);
        DrawRectangle(350, 170, 100, 100, GREEN);
        DrawRectangle(550, 170, 100, 100, RED);

        // DrawRectangle(100, 200, 200, 200, {255, 0, 0, 255});
        // DrawRectangle(300, 200, 200, 200, {0, 255, 0, 255});
        // DrawRectangle(500, 200, 200, 200, {0, 0, 255, 255});

        EndShaderMode();
        EndDrawing();
    }

    root->OnDestroy();
    CloseAudioDevice();
    CloseWindow();

    return 0;
}
