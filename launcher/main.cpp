#include "raylib.h"
#include <cstdlib>
#include <string>
#include <math.h>

// シーン定義
enum class Scene { Home, Play, Setting, Runing, Setting__ed, Runing__ed };

// シーン管理クラス
class WindowManager {
public:
    Scene status = Scene::Home;

    void goto_play()    { status = Scene::Play; }
    void goto_setting() { status = Scene::Setting__ed; }
    void goto_home()    { status = Scene::Home; }
    void now_runing()    { status = Scene::Runing__ed; }

    void update() {
        if (IsKeyDown(KEY_E)) status = Scene::Home;
    }

    void draw() {
        if (status == Scene::Setting) {
            DrawText("Settings Screen (press E to return)", 20, 20, 20, BLACK);
            std::string command = std::string("open ../../data/setting/setting.json");
            int result = system(command.c_str());
            goto_setting();//呼び出し重複防止
        } else if (status == Scene::Play) {
            std::string command = std::string("open ../../build/app/game");
            int result = system(command.c_str());
            now_runing();//呼び出し重複防止

        } else if (status == Scene::Runing) {
            DrawText("The Game is running (press E to return)", 20, 20, 20, BLACK);
        }
    }
};

// ボタンクラス
class Button {
public:
    Rectangle bounds;
    std::string text;
    Color colorNormal, colorHover, borderColor, textColorNormal, textColorHover;
    int fontSize;

    Button(Rectangle rect, const std::string& label,
           Color normal = GRAY, Color hover = LIGHTGRAY,
           Color border = DARKGRAY,
           Color textNormal = BLACK, Color textHover = BLACK,
           int fontSize = 20)
        : bounds(rect), text(label),
          colorNormal(normal), colorHover(hover),
          borderColor(border),
          textColorNormal(textNormal), textColorHover(textHover),
          fontSize(fontSize) {}

    bool Draw(Vector2 mousePoint) {
        bool hovered = CheckCollisionPointRec(mousePoint, bounds);

        DrawRectangleRec(bounds, hovered ? colorHover : colorNormal);
        DrawRectangleLinesEx(bounds, 2, borderColor);

        int textWidth = MeasureText(text.c_str(), fontSize);
        DrawText(text.c_str(),
                 bounds.x + (bounds.width - textWidth) / 2,
                 bounds.y + (bounds.height - fontSize) / 2,
                 fontSize, hovered ? textColorHover : textColorNormal);

        return hovered && IsMouseButtonPressed(MOUSE_LEFT_BUTTON);
    }
};

int main() {
    InitWindow(800, 600, "NyanthuGame Settings");
    SetTargetFPS(60);

    Button btn1({200, 250, 400, 60}, "Play");
    Button btn2({200, 330, 400, 60}, "Settings");
    Button btn3({200, 410, 400, 60}, "Exit");

    WindowManager window;

    float angle = 0.0f;

    while (!WindowShouldClose()) {
        Vector2 mousePoint = GetMousePosition();
        angle += 0.01f; // キューブ回転用

        BeginDrawing();
        ClearBackground(RAYWHITE);

        if (window.status == Scene::Home) {
            // カメラ回転（キューブ表示用）
            Camera3D camera = {0};
            camera.position = {6.0f * sinf(angle), 6.0f * 0.5f, 6.0f * cosf(angle)};
            camera.target   = {0.0f, 0.0f, 0.0f};
            camera.up       = {0.0f, 1.0f, 0.0f};
            camera.fovy     = 45.0f;

            BeginMode3D(camera);
            DrawCube((Vector3){0,0,0}, 3.0f, 3.0f, 3.0f, RED);
            DrawCubeWires((Vector3){0,0,0}, 4.0f, 4.0f, 4.0f, BLACK);
            EndMode3D();

            // UI描画
            const char* msg = "NyanthuGame\nLauncher v0.01";
            int fontSize = 40;
            int textWidth = MeasureText(msg, fontSize);
            DrawText(msg,
                     GetScreenWidth()/2 - textWidth/2,
                     100,
                     fontSize, BLACK);

            if (btn1.Draw(mousePoint)) window.goto_play();
            if (btn2.Draw(mousePoint)) window.goto_setting();
            if (btn3.Draw(mousePoint)) { CloseWindow(); return 0; }

        } else {
            window.update();
            window.draw();
        }

        EndDrawing();
    }

    CloseWindow();
    return 0;
}
