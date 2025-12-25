#pragma once

#include "renderer.h"
#include "audio.h"
#include "camera.h"
#include "input.h"

#include <memory>
#include <string>

// Forward declare GLFWwindow
struct GLFWwindow;

namespace nyanchu {

class Engine {
public:
    Engine();
    ~Engine();

    void init();
    bool isRunning();

    void pollEvents();
    void beginFrame();
    void endFrame();

    IRenderer& getRenderer();
    Camera& getCamera();
    Input& getInput();

    void playBgm(const std::string& soundName);

    void resize(int width, int height);

    void cursor_disable();
    void cursor_able();

private:
    const std::string& getResourceDir() const;

    GLFWwindow* m_window;
    std::unique_ptr<IRenderer> m_renderer;
    std::unique_ptr<Audio> m_audio;
    std::unique_ptr<Camera> m_camera;
    std::unique_ptr<Input> m_input;
    std::string m_resourceDir;
    bool m_isRunning = true;
};

} // namespace nyanchu
