#define CARD_WIDTH 180
#define CARD_HEIGHT 300
#define CARD_HEIGHT_H (CARD_HEIGHT / 2)
#define CARD_WIDTH_H (CARD_WIDTH / 2)

#define CARD_FOLLOW_SPEED 1000.0f

struct CardScene : Entity {
    EntityRef <PentagramEntitySpawner> spawner_ref{};
    EntityRef <Entity> shop_ref{};
    bool shop_active;

    void OnEnable() override;

    void AutoLayout();

    void OpenShop();

    void CloseShop();


    void Update() override {
        Entity::Update();
    }


    void Render() override {
        // DON'T RENDER CHILDREN
    }

    void AddCard(UnitData unit_data);

    void RenderGUI() override {
        Entity::Render();
        Entity::RenderGUI();
    }

    bool Summon(Vector3 position, UnitData &data);

};


struct CardTargetEntity : Entity {

    void OnRender() override {
        DrawSprite(0, 0, 40 * 3, 40 * 3, BLUE, AppearanceType::PENTAGRAM, 0);
        DrawLineEx({0, 0}, {-local_position.x + CARD_WIDTH_H - 15, -local_position.y - CARD_HEIGHT_H - 2}, 4, WHITE);
        DrawLineEx({0, 0}, {-local_position.x - CARD_WIDTH_H + 15, -local_position.y - CARD_HEIGHT_H - 2}, 4, WHITE);
    }
};

enum ShopType {
    ShopType_Summon,
    ShopType_Modification,
};

struct SummonCardEntity : Entity {

    bool is_dragged = false;
    bool is_hovering = false;
    bool is_aiming = false;
    bool is_locked = false;
    bool is_on_cooldown = false;

    float max_cooldown = 15;
    float cooldown;

    UnitData unitData;

    Sound start_hover_sfx;
    Sound place_sfx;
    EntityRef <UnitEntity> preview_unit_ref{};
    EntityRef <CardTargetEntity> target_entity_ref{};
    EntityRef <CardScene> card_scene_ref{};

    Vector3 start_position{};

    Vector3 relative_offset{};
    Vector3 hover_size{1.2, 1.2, 1};

    void OnCreate() override {
        start_hover_sfx = LoadSound("assets/cards_place_down_1.wav");
        place_sfx = LoadSound("assets/cards_place_down_2.wav");
    }

    void OnEnable() override {
        Entity::OnEnable();

        local_position = {state->screen_width + (float) CARD_WIDTH_H + 5, -CARD_HEIGHT_H - 5};

        UnitEntity *preview = AllocateEntity<UnitEntity>();
        preview->is_fake = true;
        preview->local_position = {0, 15.0f - CARD_WIDTH_H};
        preview->local_scale = {3, 3, 1};
        ConfigureFromData(preview, unitData, nullptr);
        PushChild(preview);
        preview_unit_ref = MakeRef<UnitEntity>(preview);

        CardTargetEntity *target = AllocateEntity<CardTargetEntity>();
        target->hidden = true;
        PushChild(target);
        target_entity_ref = MakeRef<CardTargetEntity>(target);
    }

    void OnDestroy() override {
        Entity::OnDestroy();
        UnloadSound(start_hover_sfx);
        UnloadSound(place_sfx);
    }

    void HandleSizing() {
        local_scale = Vector3MoveTowards(local_scale, is_hovering ? hover_size : Vector3{1, 1, 1}, GetFrameTime());
    }

    Rectangle GetShape() {
        return {local_position.x - CARD_WIDTH_H, local_position.y - CARD_HEIGHT_H, CARD_WIDTH,
                CARD_HEIGHT};
    }

    void HoverUpdate() {
        if (CheckCollisionPointRec(GetMousePosition(), GetShape())) {
            if (!is_hovering) {
                OnStartHover();
            }
        } else {
            if (is_hovering) {
                OnStopHover();
            }
        }
    }

    void Update() override {
        Entity::Update();

        if (is_on_cooldown) {
            cooldown -= GetFrameTime();
            if (cooldown <= 0) {
                cooldown = 0;
                is_on_cooldown = false;
            }
        }

        HandleSizing();

        if (!is_dragged) {
            HoverUpdate();
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

    void Select() {
        state->click_handled = true;
        relative_offset = Vector3Subtract(local_position, {GetMousePosition().x, GetMousePosition().y, 0});
        is_dragged = true;
    }

    void Release() {
        if (is_aiming) {
            bool success = card_scene_ref->Summon({GetMousePosition().x, GetMousePosition().y}, unitData);
            if (success) {
                is_on_cooldown = true;
                cooldown = max_cooldown;
            }
            PlaySound(place_sfx);
        }

        target_entity_ref->hidden = true;
        is_dragged = false;
    }

    void NoDrag() {
        local_position = Vector3MoveTowards(local_position, start_position, GetFrameTime() * CARD_FOLLOW_SPEED);
    }

    void Drag() {
        Vector2 mouse_screen_pos = GetMousePosition();
        Vector3 new_pos = {mouse_screen_pos.x, mouse_screen_pos.y, 0};

        new_pos = Vector3Add(new_pos, relative_offset);


        float max_height = state->screen_height - CARD_HEIGHT_H + 25;

        if (!is_locked && !is_on_cooldown && (mouse_screen_pos.y - (state->screen_height - CARD_HEIGHT)) < -40) {
            is_aiming = true;
            DoWorldDrag();
            return;
        }
        is_aiming = false;
        target_entity_ref->hidden = true;
        new_pos.y = fmax(new_pos.y, max_height);

        local_position = new_pos;
    }


    void DoWorldDrag() {
        local_position = Vector3MoveTowards(local_position, start_position, GetFrameTime() * CARD_FOLLOW_SPEED);

        target_entity_ref->hidden = false;

        Vector3 delta = Vector3Subtract({GetMousePosition().x, GetMousePosition().y},
                                        local_position);
        target_entity_ref->local_position = Vector3Scale(delta, 1 / local_scale.x);
    }

    void OnStartHover() {
        is_hovering = true;
        PlaySound(start_hover_sfx);
    }

    void OnStopHover() {
        is_hovering = false;
    }

    void OnRender() override {
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

    void OnLateRender() {
        Rectangle card_shape = {-CARD_WIDTH_H, -CARD_HEIGHT_H, CARD_WIDTH, CARD_HEIGHT};

        if (is_locked || is_on_cooldown) {
            DrawRectangleRounded(card_shape, 0.2f, 1, {0, 0, 0, 120});
            DrawRectangleRoundedLines(card_shape, 0.2f, 1, 4, {0, 0, 0, 120});

            const char *text = TextFormat("%.2f", cooldown);

            int text_width = MeasureText(text, 30);
            DrawText(text, -text_width / 2, -CARD_WIDTH_H - 15, 30, WHITE);
        }
    }
};

struct ShopCardEntity : Entity {
    ShopType type;

    UnitData new_unit_data;
    EntityRef <CardScene> card_scene;
    EntityRef <UnitEntity> preview_unit_ref;

    Vector3 start_position;

    void OnCreate() override {
        local_position = {(float) state->screen_width / 2, -CARD_HEIGHT_H - 20};
    }

    void OnEnable() override {
        Entity::OnEnable();

        if(type == ShopType_Summon) {
            UnitEntity *preview = AllocateEntity<UnitEntity>();
            preview->is_fake = true;
            preview->local_position = {0, 15.0f - CARD_WIDTH_H};
            preview->local_scale = {3, 3, 1};
            ConfigureFromData(preview, new_unit_data, nullptr);
            PushChild(preview);
            preview_unit_ref = MakeRef<UnitEntity>(preview);
        }
    }


    void ApplyToCard(UnitData &unit_data) {
        if (type == ShopType_Summon) {
            card_scene->AddCard(new_unit_data);
        }
        DeleteEntity(*parent);
        card_scene->AutoLayout();
        card_scene->CloseShop();
    }

    void Update() override {
        Entity::Update();
        local_position = Vector3MoveTowards(local_position, start_position, GetFrameTime() * CARD_FOLLOW_SPEED);
    }

    void OnRender() override {
        Rectangle card_shape = {-CARD_WIDTH_H, -CARD_HEIGHT_H, CARD_WIDTH, CARD_HEIGHT};
        DrawRectangleRounded(card_shape, 0.2f, 1, BLACK);
        DrawRectangleRoundedLines(card_shape, 0.2f, 1, 4, WHITE);

        if(type == ShopType_Summon) {
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

    }


    Rectangle GetShape() {
        return {local_position.x - CARD_WIDTH_H, local_position.y - CARD_HEIGHT_H, CARD_WIDTH,
                CARD_HEIGHT};
    }
};

struct ShopEntity : Entity {

    bool close_timer_active;
    float close_timer;
    EntityRef <CardScene> card_scene;

    void OnEnable() override {
        Entity::OnEnable();
        int free_card_spaces = 9 - card_scene->child_count; // Includes SHOP ENTITY so cards are -1
        if (free_card_spaces < 0) free_card_spaces = 0;


        if (free_card_spaces > 0) {
            ShopCardEntity *card = AllocateEntity<ShopCardEntity>();
            card->type = ShopType_Summon;
            card->card_scene = card_scene;
            card->new_unit_data = {
                    UnitType_TANK,
                    5,
            };
            PushChild(card);

            card = AllocateEntity<ShopCardEntity>();
            card->type = ShopType_Summon;
            card->card_scene = card_scene;
            card->new_unit_data = {
                    UnitType_LIGHT,
                    5,
            };

            PushChild(card);
            card = AllocateEntity<ShopCardEntity>();
            card->type = ShopType_Summon;
            card->card_scene = card_scene;
            card->new_unit_data = {
                    UnitType_ARCHER,
                    5,
            };

            PushChild(card);

            card = AllocateEntity<ShopCardEntity>();
            card->type = ShopType_Summon;
            card->card_scene = card_scene;
            card->new_unit_data = {
                    UnitType_MEDIC,
                    3,
            };

            PushChild(card);

            AutoLayout();
        }


        if (child_count == 0) {
            close_timer_active = true;
            close_timer = 2.5;
        }
    }


    void Update() override {
        Entity::Update();

        if (close_timer_active) {
            close_timer -= GetFrameTime();
            if (close_timer <= 0) {
                card_scene->CloseShop();
                DeleteEntity(this);
            }
        }
    }

    void OnRender() override {

        DrawRectangle(0, 0, state->screen_width, state->screen_height, {0, 0, 0, 180});

        if (close_timer_active) {
            const char *text = "No purchase available...";

            int text_width = MeasureText(text, 50);

            Rectangle text_rect = Rectangle{
                    (float) state->screen_width / 2.0f - (float) text_width / 2.0f,
                    (float) state->screen_height / 2.0f,
                    (float) text_width, 50
            };
            Color text_color = RED;
            DrawText(text, text_rect.x, text_rect.y, text_rect.height, text_color);
        }
    }


    void AutoLayout() {

        ShopCardEntity *next_target = (ShopCardEntity *) *child;
        i32 index = 0;
        float x_base = ((float) state->screen_width / 2) - ((float) child_count / 2.0f) * (CARD_WIDTH + 15);
        while (next_target) {

            next_target->start_position = {x_base + (CARD_WIDTH + 15) * index, (float) state->screen_height / 2 - 60};

            index++;
            next_target = (ShopCardEntity *) *next_target->next;
        }
    }

    bool InteractAt(Vector3 position, UnitData &data) {
        ShopCardEntity *next_target = (ShopCardEntity *) *child;
        while (next_target) {

            if (CheckCollisionPointRec({position.x, position.y}, next_target->GetShape())) {
                next_target->ApplyToCard(data);
                return true;
            }
            next_target = (ShopCardEntity *) *next_target->next;
        }

        return false;
    }
};

void CardScene::OnEnable() {
    Entity::OnEnable();

    SummonCardEntity *card = AllocateEntity<SummonCardEntity>();
    card->card_scene_ref = MakeRef<CardScene>(this);
    card->unitData = {
            UnitType_ARCHER,
            5,
    };
    PushChild(card);

    card = AllocateEntity<SummonCardEntity>();
    card->card_scene_ref = MakeRef<CardScene>(this);
    card->unitData = {
            UnitType_TANK,
            5,
    };
    PushChild(card);
    card = AllocateEntity<SummonCardEntity>();
    card->card_scene_ref = MakeRef<CardScene>(this);
    card->unitData = {
            UnitType_MEDIC,
            5,
    };
    PushChild(card);

    AutoLayout();
}

void CardScene::AddCard(UnitData unit_data) {
    SummonCardEntity *card = AllocateEntity<SummonCardEntity>();
    card->card_scene_ref = MakeRef<CardScene>(this);
    card->unitData = unit_data;
    PushChild(card);
    AutoLayout();
}

void CardScene::AutoLayout() {

    SummonCardEntity *next_target = (SummonCardEntity *) *child;
    i32 index = 0;
    float x_base = ((float) state->screen_width / 2) - ((float) child_count / 2.0f) * CARD_WIDTH;
    while (next_target) {

        next_target->start_position = {x_base + CARD_WIDTH * index, (float) state->screen_height + 40};

        index++;
        next_target = (SummonCardEntity *) *next_target->next;
    }
}

void CardScene::OpenShop() {
    shop_active = true;
    ShopEntity *shop = AllocateEntity<ShopEntity>();
    shop->card_scene = MakeRef<CardScene>(this);

    shop_ref = MakeRef<Entity>(shop);
    PushChild(shop);
}

void CardScene::CloseShop() {
    shop_active = false;
}


bool CardScene::Summon(Vector3 position, UnitData &data) {
    if (shop_active) {
        ShopEntity *shop = (ShopEntity *) *shop_ref;

        shop->InteractAt(position, data);

        return false;
    }
    spawner_ref->Summon(position, data);
    return true;
}
