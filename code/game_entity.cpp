

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

        TraceLog(LOG_INFO, "CREATED");

        u32 amount = 10;
        for (int i = 0; i < amount; ++i) {
            UnitEntity *unit = AllocateEntity<UnitEntity>();

            if (amount / 2 < i) {
                unit->type = UnityType::FRIENDLY;
            } else {
                unit->overall_target = MakeRef<Entity>(tesseract);
            }

            unit_management->PushChild(unit);
        }

        PushChild(unit_management);
        PushChild(tesseract);
    }

    void Update() override
    {

    }
};

struct DevelopmentScene : Entity
{
    void Update() override
    {
        if (IsKeyPressed(KEY_F1)) {
            TraceLog(LOG_INFO, "Swap to Unit Scene");
            Entity *old_child = *child;
            if (old_child) {
                old_child->SetParent(nullptr);
                DeleteEntity(old_child);
            }
            Entity *new_child = AllocateEntity<UnitManagementTestScene>();

            new_child->SetParent(this);
        }
    }
};




