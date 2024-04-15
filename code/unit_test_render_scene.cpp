struct UnitTestRenderScene : Entity
{

    bool checked = false;
    bool dropdown_edit;

    UnitEntity *entity;

    void OnCreate() override
    {
        TraceLog(LOG_INFO, "Started Unit Test Render Scene");

        entity = AllocateEntity<UnitEntity>();
        entity->is_fake = true;
        PushChild(entity);
        entity->local_position = {400, 225};
    }

    void Update() override
    {
        Entity::Update();

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
        if (!entity) {
            return;
        }

        if (dropdown_edit) {
            GuiLock();
        }

        GuiCheckBox((Rectangle){ 10, 20, 20, 20 }, "Unit friendly", &checked);
        GuiUnlock();

        if (GuiDropdownBox((Rectangle){ 10, 50, 125, 30 },
                           "LIGHT;ARCHER;TANK;MEDIC;PLACEHOLDER;PROJECTILE;TESSERACT;PENTAGRAM",
                           (i32*) &entity->appearance, dropdown_edit)) {
            dropdown_edit = !dropdown_edit;
        }
    }
};
