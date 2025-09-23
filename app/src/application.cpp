#include "application.h"
#include <iostream>

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
    m_audio = &m_engine->getAudio();
    std::string bgm_path = m_engine->getResourceDir() + "/materials/bgm.wav";
    m_audio->play_bgm(bgm_path.c_str());
    return true;
}

void Application::run()
{
    while (m_engine->isRunning())
    {
        m_engine->pollEvents();
        m_engine->beginFrame();

        // App decides what to draw and where
        m_angle += 0.01f;
        glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f));
        model = glm::rotate(model, m_angle, glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::rotate(model, m_angle * 0.5f, glm::vec3(1.0f, 0.0f, 0.0f));

        m_engine->getRenderer().drawCube(model);

        // App decides what to draw and where

        m_engine->endFrame();
    }
}

void Application::shutdown()
{
    m_engine->shutdown();
}
