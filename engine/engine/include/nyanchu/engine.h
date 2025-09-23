#pragma once

#include "renderer.h"
#include "audio.h"

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
    void shutdown();
    bool isRunning();

    void pollEvents();
    void beginFrame();
    void endFrame();

    IRenderer& getRenderer();

    void playBgm(const std::string& soundName);

    void resize(int width, int height);

private:
    const std::string& getResourceDir() const;

    GLFWwindow* m_window;
    std::unique_ptr<IRenderer> m_renderer;
    std::unique_ptr<Audio> m_audio;
    std::string m_resourceDir;
    bool m_isRunning = true;
};

} // namespace nyanchu
