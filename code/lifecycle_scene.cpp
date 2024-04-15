struct LifecycleScene : Entity
{

    void OnCreate() override;

    void OpenMainMenu();

    void OpenGame();

    void OpenStats();
};

#include "game_scene.cpp"

struct StartScene : Entity
{

    EntityRef<LifecycleScene> lifecycle_ref;

    void RenderGUI() override
    {
        const char *start = "START GAME";

        int text_width = MeasureText(start, 50);

        Rectangle text_rect = Rectangle{
                (float) state->screen_width / 2.0f - (float) text_width / 2.0f,
                (float) state->screen_height / 2.0f,
                (float) text_width, 50
        };
        Color text_color = WHITE;
        Vector2 mouse_pos = GetMousePosition();

        if (CheckCollisionPointRec(mouse_pos, text_rect)) {
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
        GuiSlider((Rectangle) {text_rect.x + volume_width, text_rect.y + 80, (float) (text_width - volume_width),
                               (float) volume_size}, volume, TextFormat("", volume_value), &volume_value, 0, 1);

        SetMasterVolume(volume_value);

        GuiSetStyle(DEFAULT, TEXT_SIZE, 10);
    }

};

struct StatsScene : Entity
{
    EntityRef<LifecycleScene> lifecycle_ref;


    void RenderGUI() override
    {

        const char *close_menu = "Back to Main Menu";

        int text_width = MeasureText(close_menu, 30);

        Rectangle text_rect = Rectangle{
                (float) state->screen_width / 2.0f - (float) text_width / 2.0f,
                (float) state->screen_height - 60,
                (float) text_width, 30
        };
        Color text_color = RED;
        Vector2 mouse_pos = GetMousePosition();

        if (CheckCollisionPointRec(mouse_pos, text_rect)) {
            text_color = SKYBLUE;

            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                lifecycle_ref->OpenMainMenu();
            }
        }
        DrawText(close_menu, text_rect.x, text_rect.y, text_rect.height, text_color);

        int x_offset = state->screen_width / 2 - 280;
        int spacing = 40 + 10;
        int y_offset = state->screen_height / 2 - spacing * 1.5f;

        DrawText("Your core was destroyed", state->screen_width / 2-MeasureText("Your core was destroyed", 50) / 2, y_offset - 2 * spacing, 50, RED);

        DrawText("Duration: ", x_offset, y_offset + spacing * 0, 40, WHITE);
        const char *duration_timer = GetDurationString();
        DrawText(duration_timer, MeasureText("Duration: ", 40) + x_offset, y_offset + spacing * 0, 40, ORANGE);

        const char *kill_counter = TextFormat("%d", state->stats.enemies_killed);
        DrawText("Killed enemies: ",  x_offset, y_offset + spacing * 1, 40, WHITE);
        DrawText(kill_counter, MeasureText("Killed enemies: ", 40) + x_offset, y_offset + spacing * 1, 40, RED);

        const char *wave_counter = TextFormat("%d", state->stats.wave_id);
        DrawText("Survived waves: ", x_offset, y_offset + spacing * 2, 40, WHITE);
        DrawText(wave_counter, MeasureText("Survived waves: ", 40) + x_offset, y_offset + spacing * 2, 40, GREEN);


        DrawText("Share your highscores in the comments :D", state->screen_width / 2-MeasureText("Share your highscores in the comments :D", 40) / 2, y_offset + spacing * 4, 40, GOLD);
    }

};

void LifecycleScene::OnCreate()
{
    StartScene *new_scene = AllocateEntity<StartScene>();
    new_scene->lifecycle_ref = MakeRef<LifecycleScene>(this);
    PushChild(new_scene);
}

void LifecycleScene::OpenMainMenu()
{
    DeleteEntity(*child);
    StartScene *new_scene = AllocateEntity<StartScene>();
    new_scene->lifecycle_ref = MakeRef<LifecycleScene>(this);
    PushChild(new_scene);
}

void LifecycleScene::OpenGame()
{
    DeleteEntity(*child);
    GameScene *new_scene = AllocateEntity<GameScene>();
    new_scene->lifecycle_ref = MakeRef<LifecycleScene>(this);
    PushChild(new_scene);
}

void LifecycleScene::OpenStats()
{
    DeleteEntity(*child);
    StatsScene *new_scene = AllocateEntity<StatsScene>();
    new_scene->lifecycle_ref = MakeRef<LifecycleScene>(this);
    PushChild(new_scene);
}

