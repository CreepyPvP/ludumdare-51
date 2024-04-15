#define CARD_WIDTH 180
#define CARD_HEIGHT 300
#define CARD_HEIGHT_H (CARD_HEIGHT / 2)
#define CARD_WIDTH_H (CARD_WIDTH / 2)

#define CARD_FOLLOW_SPEED 200.0f

struct CardEntity : Entity
{

    bool is_dragged = false;
    EntityRef<UnitEntity> preview_unit_ref{};

    Vector3 start_position{};

    Vector3 relative_offset{};

    void OnCreate() override
    {
        UnitEntity *preview = AllocateEntity<UnitEntity>();
        preview->local_position = {0, 15.0f - CARD_WIDTH_H};
        preview->local_scale = {3, 3, 1};
        PushChild(preview);

        preview_unit_ref = MakeRef<UnitEntity>(preview);
    }

    void OnEnable() override
    {
        local_position = start_position;
    }

    void Update() override
    {
        Entity::Update();

        if (!is_dragged) {
            if (IsMouseButtonPressed(0) && !state->click_handled) {

                Rectangle card_shape = {local_position.x - CARD_WIDTH_H, local_position.y - CARD_HEIGHT_H, CARD_WIDTH,
                                        CARD_HEIGHT};
                if (CheckCollisionPointRec(GetMousePosition(), card_shape)){
                    Select();
                    return;
                }
            }
            NoDrag();
            return;
        }

        if (!IsMouseButtonDown(0)) {
            Release();
            return;
        }
        Drag();
    }

    void Select()
    {
        state->click_handled = true;
        relative_offset = Vector3Subtract(local_position, {GetMousePosition().x, GetMousePosition().y, 0});
        is_dragged = true;
    }

    void Release()
    {
        is_dragged = false;
    }

    void NoDrag() {
        local_position = Vector3MoveTowards(local_position, start_position, GetFrameTime() * 600);
    }

    void Drag()
    {
        Vector2 mouse_screen_pos = GetMousePosition();
        Vector3 new_pos = {mouse_screen_pos.x, mouse_screen_pos.y, 0};

        new_pos = Vector3Add(new_pos, relative_offset);

        float max_height = state->screen_height - CARD_HEIGHT_H + 25;
        new_pos.y = fmax(new_pos.y, max_height);

        local_position = new_pos;
    }

    void OnRender() override
    {
        Rectangle card_shape = {-CARD_WIDTH_H, -CARD_HEIGHT_H, CARD_WIDTH, CARD_HEIGHT};
        DrawRectangleRounded(card_shape, 0.2f, 1, BLACK);
        DrawRectangleRoundedLines(card_shape, 0.2f, 1, 4, WHITE);

        const char *text;
        switch (preview_unit_ref->appearance) {
            case LIGHT:
                text = "Light";
                break;
            case ARCHER:
                text = "Archer";
                break;
            case TANK:
                text = "Tank";
                break;
            case MEDIC:
                text = "Medic";
                break;
            default:
                text = "None";
                break;
        }


        int text_width = MeasureText(text, 30);                                 // Measure string width for default font
        DrawText(text, -text_width / 2, -CARD_WIDTH_H - 55, 30, WHITE);
    }
};


struct CardScene : Entity
{
    void OnCreate() override
    {

        CardEntity *card = AllocateEntity<CardEntity>();
        card->start_position = {((float) state->screen_width / 2) + CARD_WIDTH - 15, (float) state->screen_height};
        PushChild(card);

        card = AllocateEntity<CardEntity>();
        card->start_position = {((float) state->screen_width / 2), (float) state->screen_height};
        PushChild(card);
        card = AllocateEntity<CardEntity>();
        card->start_position = {((float) state->screen_width / 2) - CARD_WIDTH + 15, (float) state->screen_height};
        PushChild(card);
    }


    void Render() override
    {
        // DON'T RENDER CHILDREN
    }

    void RenderGUI() override
    {
        Entity::Render();
        Entity::RenderGUI();
    }

};
