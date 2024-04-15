#define CARD_WIDTH 180
#define CARD_HEIGHT 300
#define CARD_HEIGHT_H (CARD_HEIGHT / 2)
#define CARD_WIDTH_H (CARD_WIDTH / 2)

#define CARD_FOLLOW_SPEED 1000.0f

struct CardTargetEntity : Entity
{

    void OnRender() override
    {
        // DrawLine(0, 0, -local_position.x, -local_position.y - CARD_HEIGHT_H, WHITE);
        DrawLineEx({0, 0}, {-local_position.x + CARD_WIDTH_H - 15, -local_position.y - CARD_HEIGHT_H - 2}, 4, WHITE);
        DrawLineEx({0, 0}, {-local_position.x - CARD_WIDTH_H + 15, -local_position.y - CARD_HEIGHT_H - 2}, 4, WHITE);
        DrawSprite(0, 0, 40, 40, GREEN, AppearanceType::PENTAGRAM, 0);
    }
};

struct CardEntity : Entity
{

    bool is_dragged = false;
    bool is_hovering = false;
    bool is_aiming = false;
    Sound start_hover_sfx;
    EntityRef<UnitEntity> preview_unit_ref{};
    EntityRef<CardTargetEntity> target_entity_ref{};

    Vector3 start_position{};

    Vector3 relative_offset{};
    Vector3 hover_size{1.2, 1.2, 1};

    void OnCreate() override
    {
        start_hover_sfx = LoadSound("assets/cards_place_down_1.wav");

        UnitEntity *preview = AllocateEntity<UnitEntity>();
        preview->is_fake = true;
        preview->local_position = {0, 15.0f - CARD_WIDTH_H};
        preview->local_scale = {3, 3, 1};
        PushChild(preview);
        preview_unit_ref = MakeRef<UnitEntity>(preview);

        CardTargetEntity *target = AllocateEntity<CardTargetEntity>();
        target->hidden = true;
        PushChild(target);
        target_entity_ref = MakeRef<CardTargetEntity>(target);
    }

    void OnDestroy() override
    {
        UnloadSound(start_hover_sfx);
    }

    void OnEnable() override
    {
        local_position = {state->screen_width + (float) CARD_WIDTH_H + 5, -CARD_HEIGHT_H - 5};
    }

    void HandleSizing()
    {
        local_scale = Vector3MoveTowards(local_scale, is_hovering ? hover_size : Vector3{1, 1, 1}, GetFrameTime());
    }

    void Update() override
    {
        Entity::Update();
        Rectangle card_shape = {local_position.x - CARD_WIDTH_H, local_position.y - CARD_HEIGHT_H, CARD_WIDTH,
                                CARD_HEIGHT};

        HandleSizing();

        if (!is_dragged) {
            if (CheckCollisionPointRec(GetMousePosition(), card_shape)) {
                if (!is_hovering) {
                    OnStartHover();
                }
            } else {
                if (is_hovering) {
                    OnStopHover();
                }
            }
            if (IsMouseButtonPressed(0) && !state->click_handled && is_hovering) {
                Select();
                return;
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
        if(is_aiming) {
            // TODO spawn pentagram
        }

        target_entity_ref->hidden = true;
        is_dragged = false;
    }

    void NoDrag()
    {
        local_position = Vector3MoveTowards(local_position, start_position, GetFrameTime() * CARD_FOLLOW_SPEED);
    }

    void Drag()
    {
        Vector2 mouse_screen_pos = GetMousePosition();
        Vector3 new_pos = {mouse_screen_pos.x, mouse_screen_pos.y, 0};

        new_pos = Vector3Add(new_pos, relative_offset);


        float max_height = state->screen_height - CARD_HEIGHT_H + 25;

        if ((mouse_screen_pos.y - (state->screen_height - CARD_HEIGHT)) < -40) {
            is_aiming = true;
            DoWorldDrag();
            return;
        }
        is_aiming = false;
        target_entity_ref->hidden = true;
        new_pos.y = fmax(new_pos.y, max_height);

        local_position = new_pos;
    }


    void DoWorldDrag()
    {
        local_position = Vector3MoveTowards(local_position, start_position, GetFrameTime() * CARD_FOLLOW_SPEED);

        target_entity_ref->hidden = false;

        Vector3 delta = Vector3Subtract({GetMousePosition().x, GetMousePosition().y},
                                        local_position);
        target_entity_ref->local_position = Vector3Scale(delta, 1 / local_scale.x);
    }

    void OnStartHover()
    {
        is_hovering = true;
        PlaySound(start_hover_sfx);
    }

    void OnStopHover()
    {
        is_hovering = false;
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


        int text_width = MeasureText(text, 30);
        DrawText(text, -text_width / 2, -CARD_WIDTH_H - 55, 30, WHITE);
    }
};


struct CardScene : Entity
{
    void OnCreate() override
    {

        CardEntity *card = AllocateEntity<CardEntity>();
        card->start_position = {((float) state->screen_width / 2) + CARD_WIDTH - 15, (float) state->screen_height + 40};
        PushChild(card);

        card = AllocateEntity<CardEntity>();
        card->start_position = {((float) state->screen_width / 2), (float) state->screen_height + 40};
        PushChild(card);
        card = AllocateEntity<CardEntity>();
        card->start_position = {((float) state->screen_width / 2) - CARD_WIDTH + 15, (float) state->screen_height + 40};
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
