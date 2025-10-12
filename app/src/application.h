#pragma once

#include <nyanchu/engine.h>
#include <nyanchu/mesh.h>
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
    std::unique_ptr<nyanchu::Mesh> m_mesh;
    float m_angle = 0.0f;
};