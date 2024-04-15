struct GameWorld : Entity
{

    EntityRef<Entity> unit_container_ref;
    EntityRef<Entity> projectile_container_ref;
    EntityRef<TesseractEntity> tesseract_ref;

    void OnCreate() override
    {

        Entity *unit_management = AllocateEntity<UnitManagementEntity>();
        TesseractEntity *tesseract = AllocateEntity<TesseractEntity>();
        Entity *projectile_container = AllocateEntity<Entity>();

        tesseract->local_position = {(float) state->screen_width / 2.0f, (float) state->screen_height / 2.0f};
        tesseract->health = 20;

        PushChild(projectile_container);
        PushChild(unit_management);
        PushChild(tesseract);

        unit_container_ref = MakeRef<Entity>(unit_management);
        projectile_container_ref = MakeRef<Entity>(projectile_container);
        tesseract_ref = MakeRef<TesseractEntity>(tesseract);
    }
};

void ClearEntity(Entity *entity)
{
    Entity *child = *entity->child;
    while (child) {
        Entity *tmp = child;
        child = *child->next;
        DeleteEntity(tmp);
    }
}

struct GameScene : Entity
{

    u32 spawn_seed;
    u32 units_left_in_wave;
    float time_until_next_spawn;
    float time_until_next_wave;

    bool stall_for_shop_after_wave;

    u32 wave_id = 0;

    PentagramEntitySpawner *penta_spawner;

    EntityRef<LifecycleScene> lifecycle_ref;
    EntityRef<GameWorld> game_world_ref;
    EntityRef<CardScene> card_scene_ref;

    void OnCreate() override
    {

        state->stats = {};

        GameWorld *game_world = AllocateEntity<GameWorld>();
        game_world_ref = MakeRef<GameWorld>(game_world);

        penta_spawner = AllocateEntity<PentagramEntitySpawner>();
        penta_spawner->projectile_container_ref = game_world->projectile_container_ref;
        penta_spawner->unit_container_ref = game_world->unit_container_ref;
        penta_spawner->tesseract_ref = game_world->tesseract_ref;

        CardScene *card_scene = AllocateEntity<CardScene>();
        card_scene_ref = MakeRef<CardScene>(card_scene);
        card_scene->spawner_ref = MakeRef<PentagramEntitySpawner>(penta_spawner);

        units_left_in_wave = 4;
        time_until_next_spawn = 0;
        time_until_next_wave = 3;

        u32 spawn_seed = GetRandomValue(0, 10000);

        PushChild(card_scene);
        PushChild(game_world);
        PushChild(penta_spawner);
    }

    void Update() override
    {
        Entity::Update();

        if (card_scene_ref->shop_active) {
            return;
        }

        float delta = GetFrameTime();
        state->stats.match_duration += delta;

        if (time_until_next_wave < 4 && stall_for_shop_after_wave) {
            card_scene_ref->OpenShop();
            ClearEntity(*game_world_ref->unit_container_ref);
            ClearEntity(penta_spawner);
            stall_for_shop_after_wave = false;
        }

        if (time_until_next_wave <= 0) {
            if (units_left_in_wave > 0) {
                time_until_next_spawn -= delta;
                if (time_until_next_spawn <= 0) {
                    float x = 0, y = 0, dist;

                    do {
                        x = halton(units_left_in_wave + 20 * wave_id + spawn_seed + x * 100 + y * 200, 2);
                        y = halton(units_left_in_wave + 20 * wave_id + spawn_seed + x * 200 + y * 100, 3);
                        dist = (x - 0.5) * (x - 0.5) + (y - 0.5) * (y - 0.5);
                    } while (dist < 0.2 * 0.2);

                    x *= state->screen_width;
                    y *= state->screen_height;

                    UnitType type = UnitType_LIGHT;
                    if (units_left_in_wave % 4 == 0) {
                        type = UnitType_ARCHER;
                    }
                    if (wave_id > 3 && units_left_in_wave % 5 == 0) {
                        type = UnitType_TANK;
                    }
                    if (wave_id > 5 && units_left_in_wave % 7 == 0) {
                        type = UnitType_MEDIC;
                    }

                    u32 batch_size = 1;
                    float batch_random = halton(units_left_in_wave + wave_id + spawn_seed, 7);
                    if (batch_random > 0.6) {
                        batch_size = 2;
                    }
                    if (wave_id > 3 && batch_random > 0.8) {
                        batch_size = 3;
                    }
                    if (wave_id > 8 && batch_random > 0.7) {
                        batch_size = 4;
                    }

                    penta_spawner->Summon(Vector3 { x, y, 0 }, { type, batch_size, true });

                    time_until_next_spawn = 0.25 * halton(units_left_in_wave + spawn_seed, 5) + 0.2;
                    units_left_in_wave--;

                    if (units_left_in_wave == 0) {
                        wave_id++;
                        state->stats.wave_id = wave_id;

                        time_until_next_wave = 20;
                        time_until_next_spawn = 0;
                        units_left_in_wave = 10 + wave_id * 2;
                        stall_for_shop_after_wave = wave_id % 3 == 0;

                        if (stall_for_shop_after_wave) {
                            time_until_next_wave = 30;
                        }
                    }
                }
            }
        } else {
            time_until_next_wave -= delta;
        }

        if (game_world_ref->tesseract_ref->health == 0) {
            lifecycle_ref->OpenStats();
        }
    }


    void RenderGUI() override
    {
        Entity::RenderGUI();
        DrawText("Duration: ", 6, 2, 40, WHITE);
        const char *duration_timer;
        if (state->stats.match_duration <= 60.0) {
            duration_timer = TextFormat("%.2f", state->stats.match_duration);
        } else {
            duration_timer = TextFormat("%d:%02d", (i32) (state->stats.match_duration / 60), (i32) state->stats.match_duration % 60);
        }
        DrawText(duration_timer, MeasureText("Duration: ", 40) + 6, 2, 40, ORANGE);

        int enemy_killed_text_w = MeasureText(" Enemies killed", 40);
        DrawText(" Enemies killed", state->screen_width - enemy_killed_text_w - 6, 2, 40, WHITE);
        const char *kill_counter = TextFormat("%d", state->stats.enemies_killed);
        DrawText(kill_counter, state->screen_width - enemy_killed_text_w - 6 - MeasureText(kill_counter, 40), 2, 40,
                 RED);

        int wave_w = MeasureText("Wave ", 40);
        DrawText("Wave ", state->screen_width / 2 - wave_w / 2, 2, 40, WHITE);
        const char *wave_counter = TextFormat("%d", wave_id);
        DrawText(wave_counter, state->screen_width / 2 + wave_w / 2  , 2, 40,
                 RED);


        const char *warning = "WAVE INCOMING";
        i32 warning_size = 40;
        i32 warning_width =  MeasureText(warning, warning_size);

        if (time_until_next_wave <= 1.8 && time_until_next_wave >= 1.5) {
            DrawText(warning, state->screen_width / 2 - warning_width / 2, 150, warning_size, RED);
        }
        if (time_until_next_wave <= 1.2 && time_until_next_wave >= 0.9) {
            DrawText(warning, state->screen_width / 2 - warning_width / 2, 150, warning_size, RED);
        }
        if (time_until_next_wave <= 0.6 && time_until_next_wave >= 0.3) {
            DrawText(warning, state->screen_width / 2 - warning_width / 2, 150, warning_size, RED);
        }
    }
};
