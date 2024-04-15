#include "raylib.h"
#include "rlgl.h"
#include "raymath.h"
#include "utils.h"

#define RAYGUI_IMPLEMENTATION
#include "raygui.h"

#include <math.h>
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

enum AppearanceType
{
    LIGHT,
    ARCHER,
    TANK,
    MEDIC,

    UNIT_TYPE_COUNT,

    PROJECTILE,
    TESSERACT,
    PENTAGRAM
};

struct EntitySlot
{
    char buffer[300];
};

struct GameStats {
    float match_duration;
    u32 enemies_killed;
};

struct GameState
{
    Arena arena;

    Shader neon_shader;
    i32 entity_type_loc;
    i32 distortion_amp_loc;

    u32 entity_cap;
    EntitySlot *entity_slots;
    u32 *entity_generations;
    u32 free_entity_count;
    u32 *free_entities;

    i32 screen_width;
    i32 screen_height;
    bool click_handled;

    GameStats stats;

    u32 max_units[UNIT_TYPE_COUNT];
    u32 alive_units[UNIT_TYPE_COUNT];
    Music unit_music[UNIT_TYPE_COUNT];
    float unit_music_volume[UNIT_TYPE_COUNT];
    Music base_layer;
    Sound unit_summoned_sound[UNIT_TYPE_COUNT];

    Sound tesseract_damaged_sound[3];
};

GameState *state;

void DrawSprite(f32 x, f32 y, f32 width, f32 height, Color color, AppearanceType type, float distortion_amp)
{

    BeginShaderMode(state->neon_shader);

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
    SetShaderValue(state->neon_shader, state->distortion_amp_loc, &distortion_amp, SHADER_UNIFORM_FLOAT);
    rlDrawRenderBatchActive();

    EndShaderMode();
}

#include "unit_data.cpp"
#include "entity.cpp"
#include "unit_entity.cpp"
#include "unit_test_render_scene.cpp"
#include "start_scene.cpp"
#include "card_scene.cpp"
#include "game_scene.cpp"
#include "game_entity.cpp"
#include "arena.cpp"

static GameState* create_game_state(void *memory, u64 memory_size)
{
    GameState pre = {};
    InitArena(&pre.arena, memory, memory_size);

    GameState *state = PushStruct(&pre.arena, GameState);
    *state = pre;

    state->click_handled = false;
    state->screen_width = 1600;
    state->screen_height = 900;
    state->entity_cap = 100;
    state->entity_slots = PushArray(&state->arena, EntitySlot, state->entity_cap);
    state->entity_generations = PushArray(&state->arena, u32, state->entity_cap);
    state->free_entity_count = state->entity_cap;
    state->free_entities = PushArray(&state->arena, u32, state->entity_cap);
    for (u32 i = 0; i < state->entity_cap; ++i) {
        state->free_entities[i] = i;
        state->entity_generations[i] = 1;
    }

    for (u32 i = 0; i < UNIT_TYPE_COUNT; ++i) {
        state->max_units[i] = 5;
    }

    return state;
}

void load_game_audio(GameState *state)
{
    SetMasterVolume(0.15f);
    state->base_layer = LoadMusicStream("assets/Gameplay_base_layer.wav");


    state->unit_music[LIGHT] = LoadMusicStream("assets/Melee_summoned_music.wav");
    state->unit_music[ARCHER] = LoadMusicStream("assets/Range_summoned_music.wav");
    state->unit_music[TANK] = LoadMusicStream("assets/Tank_summoned_music.wav");
    state->unit_music[MEDIC] = LoadMusicStream("assets/Medic_summoned_music.wav");

    state->unit_summoned_sound[LIGHT] = LoadSound("assets/melee_summon.wav");
    state->unit_summoned_sound[ARCHER] = LoadSound("assets/Range_summon.wav");
    state->unit_summoned_sound[TANK] = LoadSound("assets/tank_summon.wav");
    state->unit_summoned_sound[MEDIC] = LoadSound("assets/medic_summon.wav");

    PlayMusicStream(state->base_layer);
    SetMusicVolume(state->base_layer, 0.3);

    for (u32 i = 0; i < UNIT_TYPE_COUNT; ++i) {
        if (!IsMusicReady(state->unit_music[i])) {
            continue;
        }
        PlayMusicStream(state->unit_music[i]);
    }

    state->tesseract_damaged_sound[0] = LoadSound("assets/tesseract_damage_1.wav");
    state->tesseract_damaged_sound[1] = LoadSound("assets/tesseract_damage_2.wav");
    state->tesseract_damaged_sound[2] = LoadSound("assets/tesseract_damage_3.wav");
}

i32 main(void)
{
    SetTraceLogLevel(LOG_DEBUG);

    u64 memory_size = Megabytes(1);
    void *memory = malloc(memory_size);

    state = create_game_state(memory, memory_size);

    Entity *root = AllocateEntity<Entity>();
    root->OnEnable();
    DevelopmentScene *development_scene = AllocateEntity<DevelopmentScene>();
    root->PushChild(development_scene);

    InitWindow(state->screen_width, state->screen_height, "Title...");
    InitAudioDevice();

    load_game_audio(state);

    // DisableCursor();
    SetTargetFPS(60);

    // i32 size;
    // u8 *file = LoadFileData("assets/test.txt", &size);
    // printf("Got string: %s\n", file);
    // UnloadFileData(file);

    state->neon_shader = LoadShader("assets/neon.vert", "assets/neon.frag");

    i32 seconds_loc = GetShaderLocation(state->neon_shader, "seconds");
    state->entity_type_loc = GetShaderLocation(state->neon_shader, "entity_type");
    state->distortion_amp_loc = GetShaderLocation(state->neon_shader, "distortion_amp");

    f32 seconds = 0;

    while (!WindowShouldClose()) {
        seconds += GetFrameTime();
        state->click_handled = false;

        SetShaderValue(state->neon_shader, seconds_loc, &seconds, SHADER_UNIFORM_FLOAT);

        if(IsMusicReady(state->base_layer)) {
            UpdateMusicStream(state->base_layer);
        }

        for (u32 i = 0; i < UNIT_TYPE_COUNT; ++i) {
            if (!IsMusicReady(state->unit_music[i])) {
                continue;
            }

            float target_volume = fmin((float) state->alive_units[i] / (float) state->max_units[i], 1) * 0.5;
            float current_volume = state->unit_music_volume[i];
            float faded_volume = (target_volume - current_volume) * 0.01 + current_volume;
            state->unit_music_volume[i] = faded_volume;
            UpdateMusicStream(state->unit_music[i]);
            SetMusicVolume(state->unit_music[i], faded_volume);
        }

        root->Update();
        ClearBackground(BLACK);
        BeginDrawing();

        // static bool checked = true;
        // GuiCheckBox((Rectangle){ 25, 108, 15, 15 }, "hello world", &checked);

        root->Render();
        root->RenderGUI();

        EndDrawing();
    }

    root->OnDestroy();
    CloseAudioDevice();
    CloseWindow();

    return 0;
}
