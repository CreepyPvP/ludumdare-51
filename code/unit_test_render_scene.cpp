struct UnitTestRenderScene : Entity
{

    bool checked = false;

    void OnCreate() override
    {
        TraceLog(LOG_INFO, "Started Unit Test Render Scene");

        UnitEntity *unit = AllocateEntity<UnitEntity>();
        PushChild(unit);
        unit->local_position = {400, 225};
    }

    void Update() override
    {
        UnitEntity *unit = (UnitEntity *) *child;
        if (checked && unit->type == UnityType::HOSTILE)
        {
            unit->type = UnityType::FRIENDLY;
        }
        else if (!checked && unit->type == UnityType::FRIENDLY)
        {
            unit->type = UnityType::HOSTILE;
        }
    }

    void RenderGUI() override
    {
        GuiCheckBox((Rectangle){ 0, 10, 15, 15 }, "Unit friendly", &checked);
    }
};
