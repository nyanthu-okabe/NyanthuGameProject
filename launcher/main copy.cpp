#include "raylib.h"
#include <string>

// シーン定義
enum class Scene { Home, Play, Setting };

// シーン管理クラス
class WindowManager {
public:
    Scene status = Scene::Home;

    void goto_play()    { status = Scene::Play; }
    void goto_setting() { status = Scene::Setting; }
    void goto_home()    { status = Scene::Home; }

    void update() {
        // EキーでHomeに戻る
        if (IsKeyDown(KEY_E)) {
            status = Scene::Home;
        }
    }

    void draw() {
        if (status == Scene::Setting) {
            DrawText("Settings Screen (press E to return)", 20, 20, 20, BLACK);
        } else if (status == Scene::Play) {
            DrawText("The game is start up (press E to return)", 20, 20, 20, BLACK);
        }
    }
};

// ボタンクラス
class Button {
public:
    Rectangle bounds;
    std::string text;
    Color colorNormal;
    Color colorHover;
    Color borderColor;
    Color textColorNormal;
    Color textColorHover;
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

    Button btn1({200, 250, 400, 60}, "Play");
    Button btn2({200, 330, 400, 60}, "Settings");
    Button btn3({200, 410, 400, 60}, "Exit");

    WindowManager window;

    SetTargetFPS(60);

    while (!WindowShouldClose()) {
        Vector2 mousePoint = GetMousePosition();

        // 描画開始
        BeginDrawing();
        ClearBackground(RAYWHITE);

        if (window.status == Scene::Home) {
            // タイトル中央
            const char* msg = "NyanthuGame";
            int fontSize = 40;
            int textWidth = MeasureText(msg, fontSize);
            DrawText(msg,
                     GetScreenWidth()/2 - textWidth/2,
                     100,
                     fontSize, BLACK);

            // ボタン処理
            if (btn1.Draw(mousePoint)) {
                window.goto_play();
            }
            if (btn2.Draw(mousePoint)) {
                window.goto_setting();
            }
            if (btn3.Draw(mousePoint)) {
                CloseWindow();
                return 0;
            }

        } else {
            window.update();
            window.draw();
        }

        EndDrawing();
    }

    CloseWindow();
    return 0;
}
