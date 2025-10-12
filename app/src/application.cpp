#include "application.h"
#include <iostream>
#include <chrono>
#include "platform_utils.h"

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

Application::Application()
{
    m_engine = std::make_unique<nyanchu::Engine>();
}

Application::~Application()
{
}

bool Application::initialize()
{
    m_engine->init();

    std::string executableDir = getExecutableDir();
    std::string modelPath = executableDir + "/materials/con.obj";

    m_engine->playBgm("materials/bgm.wav");
    m_mesh = std::make_unique<nyanchu::Mesh>(modelPath.c_str());
    return true;
}

void Application::run()
{
    using clock = std::chrono::high_resolution_clock;
    auto lastTime = clock::now();
    int frameCount = 0;

    while (m_engine->isRunning())
    {
        auto startTime = clock::now();
        m_engine->pollEvents();
        m_engine->beginFrame();

        // App decides what to draw and where
        m_angle += 0.01f;
        glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f));
        model = glm::rotate(model, m_angle, glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::rotate(model, m_angle * 0.5f, glm::vec3(1.0f, 0.0f, 0.0f));

        m_engine->getRenderer().drawMesh(*m_mesh);

        m_engine->endFrame();

        // FPS計算
        frameCount++;
        auto currentTime = clock::now();
        float elapsed = std::chrono::duration<float>(currentTime - lastTime).count();
        if (elapsed >= 1.0f) // 1秒ごとに表示
        {
            std::cout << "FPS: " << frameCount / elapsed << std::endl;
            frameCount = 0;
            lastTime = currentTime;
        }
    }
}


void Application::shutdown()
{
    m_engine->shutdown();
}
