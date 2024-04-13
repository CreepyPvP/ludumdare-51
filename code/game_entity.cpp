

struct TestEntity: Entity {
    void Update() override {
        Entity::Update();
        Vector3 pos = GetWorldPosition();
        TraceLog(LOG_DEBUG, "Test Updated %f::%f::%f", pos.x, pos.y, pos.z);
    }
};
