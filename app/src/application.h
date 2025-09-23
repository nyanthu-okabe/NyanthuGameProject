#pragma once

#include <nyanchu/engine.h>
#include <nyanchu/audio.h>
#include <memory>

class Application
{
public:
    Application();
    ~Application();

    bool initialize();
    void run();
    void shutdown();

private:
    std::unique_ptr<nyanchu::Engine> m_engine;
    nyanchu::Audio* m_audio = nullptr;
    float m_angle = 0.0f;
};