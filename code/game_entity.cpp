

struct TestEntity : Entity
{
    void Update() override
    {
        Entity::Update();
        // Vector3 pos = GetWorldPosition();
        // TraceLog(LOG_DEBUG, "Test Updated %f::%f::%f", pos.x, pos.y, pos.z);
    }

    void Render() override
    {
        Entity::Render();
    }
};

struct UnitManagementTestScene : Entity
{

    void OnCreate() override
    {
        Entity *unit_management = AllocateEntity<UnitManagementEntity>();
        Entity *tesseract = AllocateEntity<Entity>();
        Entity *projectile_container = AllocateEntity<Entity>();

        tesseract->local_position = {(float) state->screen_width / 2.0f, (float) state->screen_height / 2.0f};


        u32 amount = 10;
        for (int i = 0; i < amount; ++i) {
            UnitEntity *unit = AllocateEntity<UnitEntity>();

            if (amount / 2 < i) {
                unit->team = UnitTeam::FRIENDLY;
                unit->local_position = {800, 0};
                unit->enemy_detection_range = 999999;
                unit->move_factor = unit->move_factor * 3;
                unit->color = BLUE;
            } else {
                unit->team = UnitTeam::HOSTILE;
                unit->color = RED;
                unit->overall_target = MakeRef<Entity>(tesseract);
            }

            if(i % 2 == 0) {
                unit->attack_type = UnitAttackType::RANGED;
                unit->attack_range = unit->attack_range * 4;

                if(unit->team == UnitTeam::FRIENDLY) {
                    unit->attack_speed = unit->attack_speed * 0.25f;
                }
            }

            unit->projectile_container = MakeRef<Entity>(projectile_container);

            TraceLog(LOG_INFO, "Spawn unit %d(%d)", unit->team, unit->id);


            unit_management->PushChild(unit);
        }

        PushChild(projectile_container);
        PushChild(unit_management);
        PushChild(tesseract);
    }
};

struct DevelopmentScene : Entity
{
    void Update() override
    {
        Entity::Update();
        if (IsKeyPressed(KEY_F1)) {
            TraceLog(LOG_INFO, "Swap to Unit Scene");
            Entity *old_child = *child;
            if (old_child) {
                DeleteEntity(old_child);
            }
            Entity *new_child = AllocateEntity<UnitManagementTestScene>();

            new_child->SetParent(this);
        }
        if (IsKeyPressed(KEY_F2)) {
            Entity *old_child = *child;
            if (old_child) {
                DeleteEntity(old_child);
            }
            Entity *new_child = AllocateEntity<UnitTestRenderScene>();

            new_child->SetParent(this);
        }
    }
};




