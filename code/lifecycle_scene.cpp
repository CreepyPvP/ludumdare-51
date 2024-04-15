struct LifecycleScene: Entity {

    void OnCreate() override;

    void OpenMainMenu();
    void OpenGame();
    void OpenStats();
};

#include "game_scene.cpp"

struct StartScene: Entity
{

    EntityRef<LifecycleScene> lifecycle_ref;

    void RenderGUI() override {
        const char *start = "START GAME";

        int text_width = MeasureText(start, 50);

        Rectangle text_rect = Rectangle {
            (float) state->screen_width / 2.0f - (float) text_width / 2.0f,
            (float) state->screen_height / 2.0f,
            (float) text_width, 50
        };
        Color text_color = WHITE;
        Vector2 mouse_pos = GetMousePosition();

        if (CheckCollisionPointRec(mouse_pos,text_rect)) {
            text_color = SKYBLUE;

            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                TraceLog(LOG_INFO, "Start game...");
                lifecycle_ref->OpenGame();
            }
        }

        DrawText(start, text_rect.x, text_rect.y, text_rect.height, text_color);

        i32 volume_size = 20;
        const char *volume = "VOLUME";
        int volume_width = MeasureText(volume, volume_size);

        GuiSetStyle(DEFAULT, TEXT_SIZE, volume_size);
        static float volume_value = 0.3;
        GuiSlider((Rectangle){ text_rect.x + volume_width, text_rect.y + 80, (float) (text_width - volume_width),
            (float) volume_size }, volume, TextFormat("", volume_value), &volume_value, 0, 1);

        SetMasterVolume(volume_value);

        GuiSetStyle(DEFAULT, TEXT_SIZE, 10);
    }

};

struct StatsScene: Entity {
    EntityRef<LifecycleScene> lifecycle_ref;


    void RenderGUI() override {

        const char *start = "Back to Main Menu";

        int text_width = MeasureText(start, 30);

        Rectangle text_rect = Rectangle {
                (float) state->screen_width / 2.0f - (float) text_width / 2.0f,
                (float) state->screen_height - 60,
                (float) text_width, 30
        };
        Color text_color = RED;
        Vector2 mouse_pos = GetMousePosition();

        if (CheckCollisionPointRec(mouse_pos,text_rect)) {
            text_color = SKYBLUE;

            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                lifecycle_ref->OpenMainMenu();
            }
        }

        DrawText(start, text_rect.x, text_rect.y, text_rect.height, text_color);
    }

};

void LifecycleScene::OnCreate() {
    StartScene* new_scene = AllocateEntity<StartScene>();
    new_scene->lifecycle_ref = MakeRef<LifecycleScene>(this);
    PushChild(new_scene);
}

void LifecycleScene::OpenMainMenu()
{
    DeleteEntity(*child);
    StartScene* new_scene = AllocateEntity<StartScene>();
    new_scene->lifecycle_ref = MakeRef<LifecycleScene>(this);
    PushChild(new_scene);
}

void LifecycleScene::OpenGame()
{
    DeleteEntity(*child);
    GameScene* new_scene = AllocateEntity<GameScene>();
    new_scene->lifecycle_ref = MakeRef<LifecycleScene>(this);
    PushChild(new_scene);
}

void LifecycleScene::OpenStats()
{
    DeleteEntity(*child);
    StatsScene* new_scene = AllocateEntity<StatsScene>();
    new_scene->lifecycle_ref = MakeRef<LifecycleScene>(this);
    PushChild(new_scene);
}

