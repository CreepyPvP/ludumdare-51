struct GameWorld: Entity {

    EntityRef<Entity> unit_container_ref;
    EntityRef<Entity> projectile_container_ref;
    EntityRef<TesseractEntity> tesseract_ref;

    void OnCreate() override {

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



struct GameScene: Entity {

    void OnCreate() override {

        GameWorld* game_world = AllocateEntity<GameWorld>();

        PentagramEntitySpawner* penta_spawner = AllocateEntity<PentagramEntitySpawner>();

        penta_spawner->projectile_container_ref = game_world->projectile_container_ref;
        penta_spawner->unit_container_ref = game_world->unit_container_ref;
        penta_spawner->tesseract_ref = game_world->tesseract_ref;


        CardScene* card_scene = AllocateEntity<CardScene>();
        card_scene->spawner_ref = MakeRef<PentagramEntitySpawner>(penta_spawner);

        PushChild(penta_spawner);
        PushChild(game_world);
        PushChild(card_scene);
    }
};
