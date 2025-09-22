#include "application.h"
#include <iostream>

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
    return true;
}

void Application::run()
{
    while (m_engine->isRunning())
    {
        m_engine->pollEvents();
        m_engine->beginFrame();

        // App decides what to draw
        m_engine->getRenderer().drawTriangle();

        m_engine->endFrame();
    }
}

void Application::shutdown()
{
    m_engine->shutdown();
}
