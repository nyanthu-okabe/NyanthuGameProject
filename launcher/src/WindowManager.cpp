#include "WindowManager.h"
#include "raygui.h"
#include <string>
#include <cstdlib>
#include <iostream>

void WindowManager::goto_play()    { status = Scene::Play; }
void WindowManager::goto_setting() { status = Scene::Setting; }
void WindowManager::goto_home()    { status = Scene::Home; }
void WindowManager::now_running()  { status = Scene::Running; }
void WindowManager::goto_mysite() { status = Scene::Readme; }

void WindowManager::update() {
    if (IsKeyDown(KEY_E)) {
        status = Scene::Home;
    }
    if (GuiButton((Rectangle){600, 410, 60, 60}, "Back")) {
        goto_home();
    }
}

void WindowManager::draw() {
    if (status == Scene::Setting) {
        DrawText("Settings Screen (press E to return)", 20, 20, 20, BLACK);

    } else if (status == Scene::Play) {
        std::string command = std::string("open ../../build/app/game");
        system(command.c_str());
        now_running(); //呼び出し重複防止

    } else if (status == Scene::Running) {
        DrawText("The Game is running (press E to return)", 20, 20, 20, BLACK);
    } else if (status == Scene::Readme) {
        DrawText("Info (press E to return)\n\nThis Game is made Nyanthu okabe\nCopyright (c) 2025 Nyanchu", 20, 20, 20, BLACK);
        if (GuiButton((Rectangle){30, 250, 120, 60}, "#171#Github")) {
            std::system("open https://github.com/nyanthu-okabe/");
        }
    }
}
