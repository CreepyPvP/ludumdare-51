

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

struct UnitManagementTestScene: Entity {
    void Update() override {
        if(IsKeyDown(KEY_F1)) {

        }
    }
};

struct DevelopmentScene: Entity {
    void Update() override {
        if(IsKeyDown(KEY_F1)) {
            Entity* old_child = *child;
            if(old_child) {
                old_child->SetParent(nullptr);
                // TODO remove entity
            }
            Entity* new_child = AllocateEntity<UnitManagementTestScene>();

            new_child->SetParent(this);
        }
    }
};




