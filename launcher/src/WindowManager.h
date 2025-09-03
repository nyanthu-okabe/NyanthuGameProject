#ifndef WINDOW_MANAGER_H
#define WINDOW_MANAGER_H

#include "raylib.h"

// シーン定義
enum class Scene { Home, Play, Setting, Running, Readme };

// シーン管理クラス
class WindowManager {
public:
    Scene status = Scene::Home;
    bool showMessageBox = false;

    void goto_play();
    void goto_setting();
    void goto_home();
    void now_running();
    void goto_mysite();

    void update();
    void draw();
};

#endif // WINDOW_MANAGER_H
