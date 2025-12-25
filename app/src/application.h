#pragma once

#include <nyanchu/engine.h>
#include <nyanchu/mesh.h>
#include <memory>

/*
 * Nyanthu Okabe 2025-12-25
 *
 * Copyright (c) 2025 nyanthu.com
 * All rights reserved.
 *
 * Do not modify or copy without permission.
 */


class Application
{
public:
    Application();
    ~Application();

    bool initialize();
    void run();

private:
    std::unique_ptr<nyanchu::Engine> m_engine;
    std::unique_ptr<nyanchu::Mesh> m_mesh;
    float m_angle = 0.0f;
};
