#include "raylib.h"

#define RAYGUI_IMPLEMENTATION
#include "raygui.h"

#include "WindowManager.h"
#include <cmath>

int main() {
    InitWindow(800, 600, "NyanthuGame Settings");
    SetTargetFPS(60);

    WindowManager window;

    float angle = 0.0f;

    while (!WindowShouldClose()) {
        angle += 0.01f; // キューブ回転用

        BeginDrawing();
        ClearBackground(RAYWHITE);

        if (window.status == Scene::Home) {
            // カメラ回転（キューブ表示用）
            Camera3D camera = {0};
            camera.position = {6.0f * sinf(angle), 3.0f, 6.0f * cosf(angle)};
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
            DrawText(msg,
                     GetScreenWidth()/2 - MeasureText(msg, fontSize)/2,
                     100,
                     fontSize, BLACK);

            if (GuiButton((Rectangle){200, 250, 400, 60}, "#162#Play")) {
                window.goto_play();
            }
            if (GuiButton((Rectangle){200, 330, 400, 60}, "#141#Settings")) {
                window.goto_setting();
            }
            if (GuiButton((Rectangle){200, 410, 400, 60}, "#159#Exit")) {
                window.showMessageBox = true; // まず「終了確認出すぞ」フラグを立てる
            }

            if (window.showMessageBox) {
                int result = GuiMessageBox(
                    (Rectangle){ 200, 200, 400, 100 },
                    "#191#Confirm Exit",
                    "Really exit the game?",
                    "Yes;No"
                );

                if (result == 0) {  // Yes
                    CloseWindow();
                    return 0;
                }
                else if (result == 1) {  // No
                    window.showMessageBox = false; // ダイアログ閉じる
                }
            }
            if (GuiButton((Rectangle){600, 410, 60, 60}, "#150#")) {
                window.goto_mysite();
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