struct UnitTestRenderScene : Entity
{

    bool checked = false;

    void OnCreate() override
    {
        TraceLog(LOG_INFO, "Started Unit Test Render Scene");

        UnitEntity *unit = AllocateEntity<UnitEntity>();
        PushChild(unit);
        unit->local_position = {(float) state->screen_width/2.0f, (float) state->screen_height/2.0f};
    }

    void Update() override
    {
        UnitEntity *unit = (UnitEntity *) *child;
        if (checked && unit->team == UnitTeam::HOSTILE)
        {
            unit->team = UnitTeam::FRIENDLY;
        }
        else if (!checked && unit->team == UnitTeam::FRIENDLY)
        {
            unit->team = UnitTeam::HOSTILE;
        }
    }

    void RenderGUI() override
    {
        GuiCheckBox((Rectangle){ 0, 10, 15, 15 }, "Unit friendly", &checked);
    }
};
