#include "nyanchu/engine.h"
#include <GLFW/glfw3.h>
#include <iostream>

#ifdef __APPLE__
#include "nyanchu/renderer_metal.h"
#else
#include "nyanchu/renderer_opengl.h"
#endif

namespace nyanchu {

// GLFW framebuffer resize callback
static void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    auto engine = static_cast<Engine*>(glfwGetWindowUserPointer(window));
    if (engine) {
        engine->resize(width, height);
    }
}

Engine::Engine() : m_window(nullptr) {}

Engine::~Engine() {
    // shutdown should be called explicitly
}

void Engine::init() {
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return;
    }

    // For now, let's create a window here. This might be configurable later.
    m_window = glfwCreateWindow(800, 600, "Nyanthu Engine", NULL, NULL);
    if (!m_window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return;
    }
    glfwMakeContextCurrent(m_window);

    // Set up resize callback
    glfwSetWindowUserPointer(m_window, this);
    glfwSetFramebufferSizeCallback(m_window, framebuffer_size_callback);

    #ifdef __APPLE__
    m_renderer = std::make_unique<RendererMetal>();
#else
    m_renderer = std::make_unique<RendererBGFX>();
#endif
    if (!m_renderer->initialize(m_window, 800, 600)) {
        std::cerr << "Failed to initialize Renderer" << std::endl;
        return;
    }

    m_audio = std::make_unique<Audio>();

    m_isRunning = true;
    std::cout << "Engine initialized" << std::endl;
}

void Engine::shutdown() {
    m_renderer->shutdown();
    glfwDestroyWindow(m_window);
    glfwTerminate();
    std::cout << "Engine shutdown" << std::endl;
}

bool Engine::isRunning() {
    return m_isRunning && !glfwWindowShouldClose(m_window);
}

void Engine::pollEvents() {
    glfwPollEvents();
}

void Engine::beginFrame() {
    m_renderer->beginFrame();
}

void Engine::endFrame() {
    m_renderer->endFrame();
    glfwSwapBuffers(m_window);
}

void Engine::resize(int width, int height) {
    m_renderer->resize(width, height);
}

IRenderer& Engine::getRenderer() {
    return *m_renderer;
}

Audio& Engine::getAudio() {
    return *m_audio;
}

} // namespace nyanchu