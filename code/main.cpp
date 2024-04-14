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

    Shader neon_shader;
    i32 entity_type_loc;

    u32 entity_cap;
    EntitySlot *entity_slots;
    u32 *entity_generations;
    u32 free_entity_count;
    u32 *free_entities;
};

GameState *state;

void DrawSprite(f32 x, f32 y, f32 width, f32 height, Color color, i32 type)
{

    rlBegin(RL_QUADS);

    rlNormal3f(0.0f, 0.0f, 1.0f);
    rlColor4ub(color.r, color.g, color.b, color.a);

    rlTexCoord2f(0, 1);
    rlVertex2f(x - width / 2, y - height / 2);

    rlTexCoord2f(0, 0);
    rlVertex2f(x - width / 2, y + height / 2);

    rlTexCoord2f(1, 0);
    rlVertex2f(x + width / 2, y + height / 2);

    rlTexCoord2f(1, 1);
    rlVertex2f(x + width / 2, y - height / 2);

    rlEnd();
    rlSetTexture(0);

    SetShaderValue(state->neon_shader, state->entity_type_loc, &type, SHADER_UNIFORM_INT);
    rlDrawRenderBatchActive();
}

#include "entity.cpp"
#include "unit_entity.cpp"
#include "unit_test_render_scene.cpp"
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
    DevelopmentScene *development_scene = AllocateEntity<DevelopmentScene>();

    root->PushChild(development_scene);


    InitWindow(screen_width, screen_height, "Title...");
    InitAudioDevice();

    SetShapesTexture({}, {});

    // DisableCursor();
    SetTargetFPS(60);

    // i32 size;
    // u8 *file = LoadFileData("assets/test.txt", &size);
    // printf("Got string: %s\n", file);
    // UnloadFileData(file);

    state->neon_shader = LoadShader("assets/neon.vert", "assets/neon.frag");

    i32 seconds_loc = GetShaderLocation(state->neon_shader, "seconds");
    state->entity_type_loc = GetShaderLocation(state->neon_shader, "entity_type");

    f32 seconds = 0;

    while (!WindowShouldClose()) {
        seconds += GetFrameTime();
        SetShaderValue(state->neon_shader, seconds_loc, &seconds, SHADER_UNIFORM_FLOAT);

        root->Update();
        ClearBackground(BLACK);
        BeginDrawing();

        // static bool checked = true;
        // GuiCheckBox((Rectangle){ 25, 108, 15, 15 }, "hello world", &checked);

        BeginShaderMode(state->neon_shader);

        root->Render();

        EndShaderMode();
        root->RenderGUI();
        EndDrawing();
    }

    root->OnDestroy();
    CloseAudioDevice();
    CloseWindow();

    return 0;
}
