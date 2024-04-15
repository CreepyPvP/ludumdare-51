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

        PushChild(projectile_container);
        PushChild(unit_management);
        PushChild(tesseract);

        unit_container_ref = MakeRef<Entity>(unit_management);
        projectile_container_ref = MakeRef<Entity>(projectile_container);
        tesseract_ref = MakeRef<TesseractEntity>(tesseract);
    }
};


struct GameScene : Entity
{


    void OnCreate() override
    {

        state->stats = {};

        GameWorld *game_world = AllocateEntity<GameWorld>();

        PentagramEntitySpawner *penta_spawner = AllocateEntity<PentagramEntitySpawner>();

        penta_spawner->projectile_container_ref = game_world->projectile_container_ref;
        penta_spawner->unit_container_ref = game_world->unit_container_ref;
        penta_spawner->tesseract_ref = game_world->tesseract_ref;

        for (u32 i = 0; i < 10; ++i) {
            float x = halton(i, 2) * state->screen_width;
            float y = halton(i, 3) * state->screen_height;

            penta_spawner->Summon(Vector3 { x, y, 0 }, { UnitType_LIGHT, 1, true });
        }

        CardScene *card_scene = AllocateEntity<CardScene>();
        card_scene->spawner_ref = MakeRef<PentagramEntitySpawner>(penta_spawner);

        PushChild(card_scene);
        PushChild(game_world);
        PushChild(penta_spawner);
    }

    void Update() override
    {
        Entity::Update();
        state->stats.match_duration += GetFrameTime();
    }


    void RenderGUI() override
    {
        Entity::RenderGUI();
        DrawText("Duration: ", 6, 2, 40, WHITE);
        const char *duration_timer = TextFormat("%.2f", state->stats.match_duration);
        DrawText(duration_timer, MeasureText("Duration: ", 40) + 6, 2, 40, ORANGE);

        int enemy_killed_text_w = MeasureText(" enemies killed", 40);
        DrawText(" enemies killed", state->screen_width - enemy_killed_text_w - 6, 2, 40, WHITE);
        const char *kill_counter = TextFormat("%d", state->stats.enemies_killed);
        DrawText(kill_counter, state->screen_width - enemy_killed_text_w - 6 - MeasureText(kill_counter, 40), 2, 40,
                 RED);
    }
};
